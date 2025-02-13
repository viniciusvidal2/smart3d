#include "DeleteTool.h"

#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkFloatArray.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkInformation.h>
#include <vtkPolygon.h>
#include <vtkPlanes.h>
#include <vtkIdFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkExtractSelectedIds.h>
#include <vtkPointData.h>
#include <vtkCellData.h>

#include <wx/msgdlg.h>

#include "LineWidget.h"
#include "Draw.h"
#include "Utils.h"
#include "Mesh.h"

vtkStandardNewMacro(DeleteTool);

void DeleteTool::setMesh(Mesh * mesh)
{
	if (mesh)
	{
		this->mesh = mesh;
		for (const auto& actor : mesh->actors)
		{
			polyDatas.push_back(Utils::getActorPolyData(actor));
		}
		if (mesh->getIsPointCloud())
		{
			fieldType = vtkSelectionNode::POINT;
		}
		else
		{
			fieldType = vtkSelectionNode::CELL;
		}
	}
}

//----------------------------------------------------------------------------
DeleteTool::DeleteTool() : vtk3DWidget()
{
	this->EventCallbackCommand->SetCallback(DeleteTool::ProcessEvents);
}

//----------------------------------------------------------------------------
DeleteTool::~DeleteTool()
{
	if (lineWidget)
	{
		lineWidget->EnabledOff();
		lineWidget->RemoveObserver(this->EventCallbackCommand);
	}
	for(auto actor : actorsPolygon)
	{
		CurrentRenderer->RemoveActor(actor);
	}
	actorsPolygon.clear();
	selections.clear();
	polyDatas.clear();
	mesh = nullptr;
}

//----------------------------------------------------------------------------
void DeleteTool::SetEnabled(int enabling)
{
	if (!this->Interactor)
	{
		return;
	}

	if (enabling) //------------------------------------------------------------
	{
		if (this->Enabled || !mesh) //already enabled, just return
		{
			return;
		}
		if (!this->CurrentRenderer)
		{
			this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(
				this->Interactor->GetLastEventPosition()[0],
				this->Interactor->GetLastEventPosition()[1]));
			if (!this->CurrentRenderer)
			{
				return;
			}
		}
		this->Enabled = 1;

		// listen for the following events
		vtkRenderWindowInteractor *i = this->Interactor;
		i->AddObserver(vtkCommand::LeftButtonPressEvent,
			this->EventCallbackCommand, this->Priority);
		i->AddObserver(vtkCommand::KeyPressEvent,
			this->EventCallbackCommand, this->Priority);

		if (!lineWidget)
		{
			lineWidget = vtkSmartPointer<LineWidget>::New();
			lineWidget->SetInteractor(this->Interactor);
			lineWidget->setCloseLoopOnFirstNode(true);
			vtkNew<LineWidgetRepresentation> rep;
			rep->set2DRepresentation(true);
			lineWidget->SetRepresentation(rep);
		}
		lineWidget->EnabledOn();
		lineWidget->AddObserver(vtkCommand::EndInteractionEvent, this->EventCallbackCommand, this->Priority);

		this->InvokeEvent(vtkCommand::EnableEvent, nullptr);
	}

	else //disabling----------------------------------------------------------
	{
		if (!this->Enabled) //already disabled, just return
		{
			return;
		}
		this->Enabled = 0;

		// don't listen for events any more
		this->Interactor->RemoveObserver(this->EventCallbackCommand);

		// turn off the various actors
		if (lineWidget)
		{
			lineWidget->EnabledOff();
			lineWidget->RemoveObserver(this->EventCallbackCommand);
			lineWidget = nullptr;
		}
		for (auto actor : actorsPolygon)
		{
			CurrentRenderer->RemoveActor(actor);
		}
		actorsPolygon.clear();
		selections.clear();
		polyDatas.clear();
		mesh = nullptr;

		this->InvokeEvent(vtkCommand::DisableEvent, nullptr);
		this->SetCurrentRenderer(nullptr);
	}

	this->Interactor->Render();
}

//----------------------------------------------------------------------------
void DeleteTool::ProcessEvents(vtkObject* vtkNotUsed(object),
	unsigned long event,
	void* clientdata,
	void* vtkNotUsed(calldata))
{
	DeleteTool* self =
		reinterpret_cast<DeleteTool *>(clientdata);

	//okay, let's do the right thing
	switch (event)
	{
	case vtkCommand::EndInteractionEvent:
		self->UpdateRepresentation();
		break;
	case vtkCommand::LeftButtonPressEvent://Do not let the user rotate the camera
	{
		if (self->lineWidget)
		{
			if (self->lineWidget->GetRepresentation()->getPoints())
			{
				self->EventCallbackCommand->SetAbortFlag(1);
			}
		}
		break;
	}
	case vtkCommand::KeyPressEvent:
	{
		char key = self->Interactor->GetKeyCode();
		if (key == 'I')
		{
			for (auto selection : self->selections)
			{
				selection->GetNode(0)->GetProperties()->Set(vtkSelectionNode::INVERSE(),
					!selection->GetNode(0)->GetProperties()->Get(vtkSelectionNode::INVERSE()));
			}
			self->createActorPolygon();
			self->Interactor->Render();
		}
		break;
	}
	}
}

//----------------------------------------------------------------------------
void DeleteTool::UpdateRepresentation()
{
	if (lineWidget)
	{
		vtkSmartPointer<LineWidgetRepresentation> rep = lineWidget->GetRepresentation();
		if (rep->isLoopClosed() && actorsPolygon.size() == 0)
		{
			wxBeginBusyCursor();
			//Get points
			vtkSmartPointer<vtkPoints> pointsLineWidget = rep->getPoints();
			size_t numberOfPoints = pointsLineWidget->GetNumberOfPoints() - 1;//The last point is equal to the first
			//Test to see if it is convex
			vtkNew<vtkIdTypeArray> ids;
			ids->SetNumberOfComponents(1);
			for (size_t i = 0; i < numberOfPoints; i++)
			{
				ids->InsertNextTuple1(i);
			}
			if (!vtkPolygon::IsConvex(ids, pointsLineWidget))
			{
				wxMessageBox("The selection must be convex", "Error", wxICON_ERROR);
				rep->reset();
				lineWidget->EnabledOff();
				lineWidget->EnabledOn();
				selections.clear();
				this->Interactor->Render();
				wxEndBusyCursor();
				return;
			}
			vtkNew<vtkPoints> pointsFrustum;
			double frustumCentroid[3] = { 0, 0, 0 };
			for (size_t i = 0; i < 2; i++)
			{
				vtkNew<vtkIdTypeArray> ids;
				ids->SetNumberOfComponents(1);
				for (size_t j = 0; j < numberOfPoints; j++)
				{
					double display[3];
					Utils::getDisplayPosition(CurrentRenderer, pointsLineWidget->GetPoint(j), display);
					display[2] = i;
					double point[4];
					CurrentRenderer->SetDisplayPoint(display);
					CurrentRenderer->DisplayToWorld();
					CurrentRenderer->GetWorldPoint(point);
					pointsFrustum->InsertNextPoint(point[0], point[1], point[2]);
					ids->InsertNextTuple1(i * numberOfPoints + j);
				}
				double centroid[3];
				vtkPolygon::ComputeCentroid(ids, pointsFrustum, centroid);
				frustumCentroid[0] += centroid[0];
				frustumCentroid[1] += centroid[1];
				frustumCentroid[2] += centroid[2];
			}
			frustumCentroid[0] /= 2.0;
			frustumCentroid[1] /= 2.0;
			frustumCentroid[2] /= 2.0;
			//Create the implict function
			vtkNew<vtkPoints> planesPoints;
			vtkNew<vtkFloatArray> normalsPoints;
			normalsPoints->SetNumberOfComponents(3);
			for (int i = 0; i < 2; i++)//The top and bottom planes
			{
				planesPoints->InsertNextPoint(pointsFrustum->GetPoint(i + i * numberOfPoints));
				double n[3], pointA[3], pointB[3], pointC[3];
				pointsFrustum->GetPoint(numberOfPoints - 1 + (i * numberOfPoints), pointA);
				pointsFrustum->GetPoint(i + (i * numberOfPoints), pointB);
				pointsFrustum->GetPoint(i + 1 + (i * numberOfPoints), pointC);
				Utils::getNormal(pointA, pointB, pointC, frustumCentroid, n);
				vtkMath::MultiplyScalar(n, -1);
				normalsPoints->InsertNextTuple3(n[0], n[1], n[2]);
			}
			for (int i = 0; i < numberOfPoints; i++)
			{
				planesPoints->InsertNextPoint(pointsFrustum->GetPoint(i));
				double n[3], pointA[3], pointB[3], pointC[3];
				pointsFrustum->GetPoint(i + numberOfPoints, pointA);
				if ((i + 1) == numberOfPoints)//Last plane
				{
					pointsFrustum->GetPoint(0, pointB);
					pointsFrustum->GetPoint(i, pointC);
				}
				else//Planes in the side
				{
					pointsFrustum->GetPoint(i, pointB);
					pointsFrustum->GetPoint(i + 1, pointC);
				}
				Utils::getNormal(pointA, pointB, pointC, frustumCentroid, n);
				vtkMath::MultiplyScalar(n, -1);
				normalsPoints->InsertNextTuple3(n[0], n[1], n[2]);
			}
			vtkNew<vtkPlanes> planes;
			planes->SetPoints(planesPoints);
			planes->SetNormals(normalsPoints);
			size_t selectedElements = 0;
			for (auto polyData : polyDatas)
			{
				//Get Mesh points
				vtkNew<vtkIdFilter> idFilter;
				idFilter->SetInputData(polyData);
				idFilter->SetIdsArrayName("OriginalIds");
				idFilter->Update();
				// This is needed to convert the ouput of vtkIdFilter (vtkDataSet) back to vtkPolyData
				vtkNew<vtkDataSetSurfaceFilter> surfaceFilter;
				surfaceFilter->SetInputConnection(idFilter->GetOutputPort());
				surfaceFilter->Update();
				vtkNew<vtkExtractPolyDataGeometry> extractGeometry;
				extractGeometry->SetImplicitFunction(planes);
				extractGeometry->SetInputConnection(surfaceFilter->GetOutputPort());
				extractGeometry->Update();
				//List of points inside the polygon
				vtkSmartPointer<vtkIdTypeArray> insideArray;
				if (fieldType == vtkSelectionNode::CELL)
				{
					insideArray = vtkIdTypeArray::SafeDownCast(extractGeometry->GetOutput()->GetCellData()->GetArray("OriginalIds"));
				}
				else
				{
					insideArray = vtkIdTypeArray::SafeDownCast(extractGeometry->GetOutput()->GetPointData()->GetArray("OriginalIds"));
				}
				if (insideArray)
				{
					selectedElements += insideArray->GetNumberOfTuples();
				}
				else
				{
					insideArray = vtkSmartPointer<vtkIdTypeArray>::New();
				}
				vtkNew<vtkSelection> selection;
				vtkNew<vtkSelectionNode> selectionNode;
				selectionNode->SetFieldType(fieldType);
				selectionNode->SetContentType(vtkSelectionNode::INDICES);
				selectionNode->SetSelectionList(insideArray);
				selection->AddNode(selectionNode);
				selections.push_back(selection);
			}
			if (selectedElements == 0)
			{
				wxMessageBox("Nothing was selected", "Warning", wxICON_WARNING);
				rep->reset();
				lineWidget->EnabledOff();
				lineWidget->EnabledOn();
				selections.clear();
			}
			else
			{
				createActorPolygon();
				if (lineWidget)
				{
					lineWidget->EnabledOff();
					lineWidget->RemoveObserver(this->EventCallbackCommand);
					lineWidget = nullptr;
				}
			}
			this->Interactor->Render();
			wxEndBusyCursor();
		}
	}

}

void DeleteTool::createActorPolygon()
{
	if (actorsPolygon.size() != 0)
	{
		for (auto actor : actorsPolygon)
		{
			CurrentRenderer->RemoveActor(actor);
		}
		actorsPolygon.clear();
	}
	for (unsigned int i = 0; i < selections.size(); i++)
	{
		actorsPolygon.push_back(Draw::createPointCloud(CurrentRenderer, polyDatas[i], selections[i], 255, 0, 0));
	}
}

bool DeleteTool::enterKeyPressed()
{
	if (selections.size() != 0)
	{
		wxBeginBusyCursor();
		for (unsigned int i = 0; i < selections.size(); i++)
		{
			vtkSmartPointer<vtkPolyData> poly = polyDatas.at(i);
			if (poly->GetNumberOfCells() != selections.at(i)->GetNode(0)->GetSelectionList()->GetNumberOfValues())
			{
				selections.at(i)->GetNode(0)->GetProperties()->Set(vtkSelectionNode::INVERSE(), !selections.at(i)->GetNode(0)->GetProperties()->Get(vtkSelectionNode::INVERSE()));
				vtkNew<vtkExtractSelectedIds> extractSelectedIds;
				extractSelectedIds->SetInputData(0, poly);
				extractSelectedIds->SetInputData(1, selections.at(i));
				extractSelectedIds->Update();

				vtkNew<vtkDataSetSurfaceFilter> surfaceFilter;
				surfaceFilter->SetInputConnection(extractSelectedIds->GetOutputPort());
				surfaceFilter->Update();

				poly = surfaceFilter->GetOutput();
			}
			else
			{
				//Delete all mesh
				poly = vtkSmartPointer<vtkPolyData>::New();
			}
			mesh->updateCells(poly, i);
		}
		this->SetEnabled(false);
		wxEndBusyCursor();
		return true;
	}
	return false;
}
