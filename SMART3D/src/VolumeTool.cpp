#include "VolumeTool.h"

#include <sstream>

#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkCellPicker.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkDelaunay3D.h>
#include <vtkPolygon.h>
#include <vtkPlaneCollection.h>
#include <vtkPlane.h>
#include <vtkUnstructuredGrid.h>
#include <vtkMapper.h>
#include <vtkClipClosedSurface.h>
#include <vtkTriangleFilter.h>
#include <vtkMassProperties.h>

#include <wx/treelist.h>
#include <wx/msgdlg.h>

#include "ImplicitPlaneWidget.h"
#include "LineWidget.h"
#include "LineWidgetRepresentation.h"
#include "Draw.h"
#include "Utils.h"
#include "Volume.h"
#include "Calibration.h"
#include "Mesh.h"

vtkStandardNewMacro(VolumeTool);

//----------------------------------------------------------------------------
VolumeTool::VolumeTool() : vtk3DWidget()
{
	this->EventCallbackCommand->SetCallback(VolumeTool::ProcessEvents);
}

//----------------------------------------------------------------------------
VolumeTool::~VolumeTool()
{
	mesh = nullptr;
	treeMesh = nullptr;
	pointsPolygon = nullptr;
	cellPicker = nullptr;
	if (lineWidget)
	{
		lineWidget->EnabledOff();
		lineWidget->RemoveObserver(this->EventCallbackCommand);
		lineWidget = nullptr;
	}
	if (planeWidget)
	{
		planeWidget->EnabledOff();
		planeWidget = nullptr;
	}
	if (polygonActor)
	{
		CurrentRenderer->RemoveActor(polygonActor);
		polygonActor = nullptr;
	}
	if (planeActor)
	{
		CurrentRenderer->RemoveActor(planeActor);
		planeActor = nullptr;
	}
}

//----------------------------------------------------------------------------
void VolumeTool::SetEnabled(int enabling)
{
	if (!this->Interactor)
	{
		return;
	}

	if (enabling) //------------------------------------------------------------
	{
		if (this->Enabled || !mesh || !treeMesh) //already enabled, just return
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
		i->AddObserver(vtkCommand::KeyPressEvent,
			this->EventCallbackCommand, this->Priority);
		i->AddObserver(vtkCommand::RightButtonPressEvent,
			this->EventCallbackCommand, this->Priority);

		if (!cellPicker)
		{
			cellPicker = vtkSmartPointer<vtkCellPicker>::New();
			cellPicker->PickFromListOn();
		}
		if (!planeWidget)
		{
			planeWidget = vtkSmartPointer<ImplicitPlaneWidget>::New();
		}
		planeWidget->SetInteractor(this->GetInteractor());
		planeWidget->GetPlaneProperty()->SetColor(1.0, 1.0, 0.0);
		planeWidget->GetSelectedPlaneProperty()->SetColor(0.0, 1.0, 0.0);
		planeWidget->GetSelectedPlaneProperty()->SetOpacity(1.0);
		planeWidget->TubingOff();
		planeWidget->OutlineTranslationOff();
		planeWidget->OriginTranslationOff();
		planeWidget->PickingManagedOff();
		planeWidget->SetPlaceFactor(1.25);
		planeWidget->SetDiagonalRatio(0.05);
		if (mesh->volumeActor)
		{
			planeWidget->PlaceWidget(mesh->volumeActor->GetBounds());
			cellPicker->AddPickList(mesh->volumeActor);
		}
		else
		{
			planeWidget->PlaceWidget(mesh->getPolyData()->GetBounds());
			for (auto actor : mesh->actors)
			{
				cellPicker->AddPickList(actor);
			}
		}
		volumeComputed = false;
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

		mesh = nullptr;
		treeMesh = nullptr;
		pointsPolygon = nullptr;
		cellPicker = nullptr;
		if (lineWidget)
		{
			lineWidget->EnabledOff();
			lineWidget->RemoveObserver(this->EventCallbackCommand);
			lineWidget = nullptr;
		}
		if (planeWidget)
		{
			planeWidget->EnabledOff();
			planeWidget = nullptr;
		}
		if (polygonActor)
		{
			CurrentRenderer->RemoveActor(polygonActor);
			polygonActor = nullptr;
		}
		if (planeActor)
		{
			CurrentRenderer->RemoveActor(planeActor);
			planeActor = nullptr;
		}
		updatePolygon = false;

		this->InvokeEvent(vtkCommand::DisableEvent, nullptr);
		this->SetCurrentRenderer(nullptr);
	}

	this->Interactor->Render();
}

//----------------------------------------------------------------------------
void VolumeTool::ProcessEvents(vtkObject* vtkNotUsed(object),
	unsigned long event,
	void* clientdata,
	void* vtkNotUsed(calldata))
{
	VolumeTool* self =
		reinterpret_cast<VolumeTool*>(clientdata);

	//okay, let's do the right thing
	switch (event)
	{
	case vtkCommand::StartInteractionEvent:
	case vtkCommand::InteractionEvent:
	case vtkCommand::EndInteractionEvent:
		self->updatePolygon = true;
		self->UpdateRepresentation();
		break;
	case vtkCommand::KeyPressEvent:
		self->OnKeyPress();
		break;
	case vtkCommand::RightButtonPressEvent:
		self->OnRightButton();
		break;
	}
}

//----------------------------------------------------------------------------
void VolumeTool::UpdateRepresentation()
{
	if (lineWidget && updatePolygon)
	{
		vtkSmartPointer<LineWidgetRepresentation> rep = lineWidget->GetRepresentation();
		if (rep->isLoopClosed())
		{
			updatePolygon = false;
			vtkSmartPointer<vtkPoints> repPoints = rep->getPoints();
			pointsPolygon = nullptr;
			pointsPolygon = vtkSmartPointer<vtkPoints>::New();
			unsigned int numberOfPoints = repPoints->GetNumberOfPoints() - 1;
			for (unsigned int i = 0; i < numberOfPoints; i++)
			{
				pointsPolygon->InsertNextPoint(repPoints->GetPoint(i));
			}
			//To change the height of the polygon you should multiply the n vector
			double n[3];
			planeWidget->GetNormal(n);
			vtkMath::MultiplyScalar(n, polygonHeight);
			double r[3];
			for (unsigned int i = 0; i < numberOfPoints; i++)
			{
				vtkMath::Add(n, pointsPolygon->GetPoint(i), r);
				pointsPolygon->InsertNextPoint(r);
			}
			vtkNew<vtkPolyData> polyDataPoints;
			polyDataPoints->SetPoints(pointsPolygon);

			vtkNew<vtkDelaunay3D> delaunay3D;
			delaunay3D->SetInputData(polyDataPoints);
			delaunay3D->Update();

			if (polygonActor)
			{
				CurrentRenderer->RemoveActor(polygonActor);
			}
			polygonActor = Draw::createDataSet(CurrentRenderer, delaunay3D->GetOutput(), 1.0, 0.0, 0.0);
			polygonActor->GetProperty()->SetOpacity(0.3);
			polygonActor->SetPickable(false);
			CurrentRenderer->AddActor(polygonActor);
			this->Interactor->Render();
		}
		else
		{
			if (polygonActor)
			{
				CurrentRenderer->RemoveActor(polygonActor);
				this->Interactor->Render();
			}
		}
	}
}

void VolumeTool::enterKeyPressed()
{
	if (planeWidget->GetEnabled())
	{
		//Get plane actor
		if (!planeActor)
		{
			planeActor = vtkSmartPointer<vtkActor>::New();
		}
		planeActor = planeWidget->GetPlaneActor();
		planeWidget->Off();
		CurrentRenderer->AddActor(planeActor);
		CurrentRenderer->GetRenderWindow()->Render();
		//Start the line widget
		lineWidget = vtkSmartPointer<LineWidget>::New();
		lineWidget->SetInteractor(this->Interactor);
		lineWidget->setCloseLoopOnFirstNode(true);
		vtkSmartPointer<LineWidgetRepresentation> rep = vtkSmartPointer<LineWidgetRepresentation>::New();
		rep->addProp(planeActor);
		lineWidget->SetRepresentation(rep);
		lineWidget->EnabledOn();
		lineWidget->AddObserver(vtkCommand::StartInteractionEvent, this->EventCallbackCommand, this->Priority);
		lineWidget->AddObserver(vtkCommand::InteractionEvent, this->EventCallbackCommand, this->Priority);
		lineWidget->AddObserver(vtkCommand::EndInteractionEvent, this->EventCallbackCommand, this->Priority);
		updatePolygon = true;
	}
	else
	{
		if (lineWidget)
		{
			if (lineWidget->GetRepresentation()->isLoopClosed())
			{
				computeVolume();
				volumeComputed = true;
			}
		}
	}
}

void VolumeTool::OnKeyPress()
{
	char key = this->Interactor->GetKeyCode();
	if (key == '=')
	{
		polygonHeight += 0.1;
		updatePolygon = true;
		this->UpdateRepresentation();
	}
	else if (key == '-')
	{
		polygonHeight -= 0.1;
		if (polygonHeight <= 0.11)
		{
			polygonHeight = 0.1;
		}
		updatePolygon = true;
		this->UpdateRepresentation();
	}
}

void VolumeTool::OnRightButton()
{
	if (cellPicker && !planeWidget->GetEnabled() && !planeActor)
	{
		if (cellPicker->Pick(this->GetInteractor()->GetEventPosition()[0], this->GetInteractor()->GetEventPosition()[1], 0, CurrentRenderer))
		{
			double clickPos[3];
			cellPicker->GetPickPosition(clickPos);
			planeWidget->SetOrigin(clickPos);
			planeWidget->SetNormal(0, 0, 1);
			planeWidget->UpdatePlacement();
			planeWidget->On();
		}
	}
}

void VolumeTool::computeVolume()
{
	//Mesh to be tested
	vtkSmartPointer<vtkPolyData> meshPolyData;
	if (mesh->volumeActor)
	{
		meshPolyData = vtkPolyData::SafeDownCast(mesh->volumeActor->GetMapper()->GetInputAsDataSet());
	}
	else
	{
		meshPolyData = mesh->getPolyData();
	}
	//Compute centroid
	size_t numberOfPoints = pointsPolygon->GetNumberOfPoints() / 2.0;
	double centroid[3] = { 0, 0, 0 };
	for (size_t i = 0; i < 2; i++)
	{
		vtkNew<vtkIdTypeArray> ids;
		ids->SetNumberOfComponents(1);
		for (size_t j = 0; j < numberOfPoints; j++)
		{
			ids->InsertNextTuple1(i * numberOfPoints + j);
		}
		double centroidAux[3];
		vtkPolygon::ComputeCentroid(ids, pointsPolygon, centroidAux);
		centroid[0] += centroidAux[0];
		centroid[1] += centroidAux[1];
		centroid[2] += centroidAux[2];
	}
	centroid[0] /= 2.0;
	centroid[1] /= 2.0;
	centroid[2] /= 2.0;

	//Create the planes to use in the vtkClipClosedSurface
	vtkNew<vtkPlaneCollection> planes;
	double n[3];
	planeWidget->GetNormal(n);
	for (int i = 0; i < 2; i++)//The top and bottom planes
	{
		vtkNew<vtkPlane> plane;
		double pointAux[3];
		pointsPolygon->GetPoint(i + i * numberOfPoints, pointAux);
		plane->SetOrigin(pointAux);
		plane->SetNormal(n[0] + i * (-2 * n[0]), n[1] + i * (-2 * n[1]), n[2] + i * (-2 * n[2]));
		planes->AddItem(plane);
	}
	for (int i = 0; i < numberOfPoints; i++)
	{
		vtkNew<vtkPlane> plane;
		double normal[3];
		double pointA[3];
		double pointB[3];
		double pointC[3];
		pointsPolygon->GetPoint(i + numberOfPoints, pointA);
		pointsPolygon->GetPoint(i, pointB);
		if ((i + 1) == numberOfPoints)//Last plane
		{
			pointsPolygon->GetPoint(0, pointC);
		}
		else//Planes in the side
		{
			pointsPolygon->GetPoint(i + 1, pointC);
		}
		Utils::getNormal(pointA,
			pointB,
			pointC,
			centroid,
			normal);
		plane->SetOrigin(pointB);
		plane->SetNormal(normal);
		planes->AddItem(plane);
	}
	//DEBUG
	//for (int i = 0; i < planes->GetNumberOfItems(); i++)
	//{
	//	Draw::createPlane(CurrentRenderer, planes->GetItem(i)->GetOrigin(), planes->GetItem(i)->GetNormal(),255,0,0);
	//}
	

	vtkNew<vtkClipClosedSurface> clipperClosed;
	clipperClosed->SetInputData(meshPolyData);
	clipperClosed->SetClippingPlanes(planes);
	clipperClosed->Update();
	if (clipperClosed->GetOutput()->GetPolys()->GetNumberOfCells() == 0)
	{
		wxMessageBox("Nothing was selected", "Error", wxICON_ERROR);
		return;
	}
	//Pass to triangles
	vtkNew<vtkTriangleFilter> triangleFilter;
	triangleFilter->SetInputConnection(clipperClosed->GetOutputPort());
	triangleFilter->Update();

	//Compute the volume
	vtkNew<vtkMassProperties> massProperties;
	massProperties->SetInputConnection(triangleFilter->GetOutputPort());
	massProperties->Update();

	wxString stringCalcVolume;
	if ((massProperties->GetVolume() - massProperties->GetVolumeProjected()) * 10000 > massProperties->GetVolume())
	{
		wxMessageBox("Something went wrong", "Error", wxICON_ERROR);
		stringCalcVolume << "#" << mesh->qtdVolumes << " Error - " << massProperties->GetVolume() << "\u00B3";
	}
	else
	{
		stringCalcVolume << "#" << mesh->qtdVolumes << " - " << massProperties->GetVolume() << mesh->getCalibration()->getMeasureUnit() << "\u00B3";
	}
	Volume* volume = new Volume(1);
	volume->index = mesh->qtdVolumes;
	volume->volume = massProperties->GetVolume();
	volume->polyData = clipperClosed->GetOutput();
	volume->actor = Draw::createPolyData(CurrentRenderer, volume->polyData, 1.0, 1.0, 0.0);
	const std::string volumeIndicatorText = stringCalcVolume.utf8_str();
	volume->text = Draw::createText(CurrentRenderer, volume->actor->GetCenter(), volumeIndicatorText, 24, 1.0, 1.0, 1.0);
	if (!mesh->getListItemMeshVolumes())
	{
		mesh->setListItemMeshVolumes(treeMesh->AppendItem(mesh->getListItemMesh(), "Volumes"));
	}
	wxString name;
	name << "#" << mesh->qtdVolumes;
	volume->setListItem(treeMesh->AppendItem(mesh->getListItemMeshVolumes(), name));
	mesh->addVolume(volume);
	treeMesh->CheckItem(mesh->getListItemMeshVolumes(), wxCHK_CHECKED);
	treeMesh->CheckItem(volume->getListItem(), wxCHK_CHECKED);
}
