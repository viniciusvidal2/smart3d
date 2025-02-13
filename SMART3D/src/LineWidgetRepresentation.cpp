#include "LineWidgetRepresentation.h"

#include <windows.h>
#include <gl\GL.h>

#include <vtkObjectFactory.h>
#include <vtkCellLocator.h>
#include <vtkPoints.h>
#include <vtkAbstractPicker.h>
#include <vtkPropPicker.h>
#include <vtkCellPicker.h>
#include <vtkPointPicker.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkActor2D.h>
#include <vtkProperty2D.h>
#include <vtkLineSource.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkInteractorObserver.h>
#include <vtkTransform.h>
#include <vtkPolyLine.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkWindow.h>
#include <vtkCamera.h>

#include "Utils.h"

vtkStandardNewMacro(LineWidgetRepresentation);

LineWidgetRepresentation::LineWidgetRepresentation()
{
	this->InteractionState = LineWidgetRepresentation::Outside;

	meshPropPicker = vtkSmartPointer<vtkPropPicker>::New();
	meshPropPicker->PickFromListOn();
	meshCellPicker = vtkSmartPointer<vtkCellPicker>::New();
	meshCellPicker->PickFromListOn();
	meshPointPicker = vtkSmartPointer<vtkPointPicker>::New();
	meshPointPicker->PickFromListOn();
}

LineWidgetRepresentation::~LineWidgetRepresentation()
{
	this->InteractionState = LineWidgetRepresentation::Outside;
	indexActiveNode = -1;
	nodesActors.clear();
	nodesTransformFilter.clear();
	nodePoints = NULL;
	lineActor = NULL;
	lineActor2D = NULL;
	lineSource = NULL;
}

void LineWidgetRepresentation::setAlwaysOnTop(bool alwaysOnTop)
{
	this->alwaysOnTop = alwaysOnTop;
}

bool LineWidgetRepresentation::isOverFirstNode(int x, int y)
{
	if (pickNode(x, y))
	{
		return indexActiveNode == 0;
	}
	return false;
}

void LineWidgetRepresentation::closeLoop()
{
	nodePoints->SetPoint(nodePoints->GetNumberOfPoints() - 1, nodePoints->GetPoint(0));
	createNode(nodePoints->GetPoint(0));
	loopClosed = true;
}

bool LineWidgetRepresentation::isLoopClosed()
{
	return loopClosed;
}

int LineWidgetRepresentation::getNumberOfNodes()
{
	return nodesActors.size();
}

void LineWidgetRepresentation::set2DRepresentation(bool is2D)
{
	representation2D = is2D;
}

void LineWidgetRepresentation::setDrawLine(bool drawLine)
{
	this->drawLine = drawLine;
}

void LineWidgetRepresentation::setSphereColor(double * color)
{
	sphereColor[0] = color[0];
	sphereColor[1] = color[1];
	sphereColor[2] = color[2];
}

void LineWidgetRepresentation::addProp(vtkSmartPointer<vtkProp> prop)
{
	if (meshPropPicker == NULL)
	{
		meshPropPicker = vtkSmartPointer<vtkPropPicker>::New();
		meshPropPicker->PickFromListOn();
	}
	if (meshCellPicker == NULL)
	{
		meshCellPicker = vtkSmartPointer<vtkCellPicker>::New();
		meshCellPicker->PickFromListOn();
	}
	if (meshPointPicker == NULL)
	{
		meshPointPicker = vtkSmartPointer<vtkPointPicker>::New();
		meshPointPicker->PickFromListOn();
	}
	meshPropPicker->AddPickList(prop);
	meshCellPicker->AddPickList(prop);
	meshPointPicker->AddPickList(prop);
}

void LineWidgetRepresentation::addLocator(vtkSmartPointer<vtkCellLocator> locator)
{
	if (meshCellPicker == NULL)
	{
		meshCellPicker = vtkSmartPointer<vtkCellPicker>::New();
		meshCellPicker->PickFromListOn();
	}
	meshCellPicker->AddLocator(locator);
}

void LineWidgetRepresentation::createNode(double* point)
{
	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->SetRadius(0.1);
	vtkSmartPointer<vtkTransform> T = vtkSmartPointer<vtkTransform>::New();
	T->Identity();
	vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	transformFilter->SetTransform(T);
	transformFilter->SetInputConnection(sphereSource->GetOutputPort());
	transformFilter->Update();
	nodesTransformFilter.push_back(transformFilter);
	//Create a mapper and actor
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(transformFilter->GetOutputPort());
	vtkSmartPointer<vtkActor> actorSphere = vtkSmartPointer<vtkActor>::New();
	actorSphere->SetMapper(mapper);
	actorSphere->GetProperty()->SetColor(sphereColor[0], sphereColor[1], sphereColor[2]);
	actorSphere->SetPosition(point);
	nodesActors.push_back(actorSphere);
}

void LineWidgetRepresentation::addNode(int x, int y)
{
	double* pickedPos = new double[3];
	if (pickFinalPosition(x, y, pickedPos))
	{
		if (nodePoints == NULL)
		{
			nodePoints = vtkSmartPointer<vtkPoints>::New();
			nodePoints->InsertNextPoint(pickedPos);
		}
		else
		{
			nodePoints->SetPoint(nodePoints->GetNumberOfPoints() - 1, pickedPos);
		}
		nodePoints->InsertNextPoint(pickedPos);
		createNode(pickedPos);
		return;
	}
	delete pickedPos;
}

bool LineWidgetRepresentation::addFinalNode(int x, int y)
{
	double* pickedPos = new double[3];
	if (pickFinalPosition(x, y, pickedPos))
	{
		nodePoints->SetPoint(nodePoints->GetNumberOfPoints() - 1, pickedPos);
		createNode(pickedPos);
		return true;
	}
	delete pickedPos;
	return false;
}

void LineWidgetRepresentation::deleteNode(int id)
{
	if (nodesActors.size() == 0)
	{
		return;
	}
	this->InteractionState = LineWidgetRepresentation::Outside;
	indexActiveNode = -1;
	if (nodesActors.size() == 1)
	{
		nodesActors.clear();
		nodesTransformFilter.clear();
		nodePoints = NULL;
		lineActor = NULL;
		lineActor2D = NULL;
		lineSource = NULL;
	}
	else if (nodesActors.size() < nodePoints->GetNumberOfPoints())//We are still adding points, remove the last point
	{
		nodesActors.pop_back();
		nodesTransformFilter.pop_back();
		Utils::deletePoint(nodePoints, nodePoints->GetNumberOfPoints() - 1);
	}
	else if (loopClosed)
	{
		if (id == 0 || id == nodesActors.size() - 1)
		{
			nodesActors.pop_back();
			nodesTransformFilter.pop_back();
			Utils::deletePoint(nodePoints, nodePoints->GetNumberOfPoints() - 1);
			nodePoints->InsertNextPoint(nodesActors.back()->GetPosition());
			loopClosed = false;
		}
		else
		{
			nodesActors.erase(nodesActors.begin() + id);
			nodesTransformFilter.erase(nodesTransformFilter.begin() + id);
			Utils::deletePoint(nodePoints, id);
		}
		if (loopClosed && nodesActors.size() == 3)//we just have a line, destroy the last.
		{
			nodesActors.pop_back();
			nodesTransformFilter.pop_back();
			Utils::deletePoint(nodePoints, nodePoints->GetNumberOfPoints() - 1);
			nodePoints->InsertNextPoint(nodesActors.back()->GetPosition());
			loopClosed = false;
		}
	}
	else
	{
		nodesActors.erase(nodesActors.begin() + id);
		nodesTransformFilter.erase(nodesTransformFilter.begin() + id);
		Utils::deletePoint(nodePoints, id);
		//Add a point because the user need to add more points to match the maxNumberOfPoints
		nodePoints->InsertNextPoint(nodesActors.back()->GetPosition());
	}
}

void LineWidgetRepresentation::removeLastNode()
{
	deleteNode(-1);
}

void LineWidgetRepresentation::removeActiveNode()
{
	if (indexActiveNode != -1)
	{
		deleteNode(indexActiveNode);
	}
}

void LineWidgetRepresentation::resetPickers()
{
	if (meshPropPicker == NULL)
	{
		meshPropPicker = vtkSmartPointer<vtkPropPicker>::New();
	}
	meshPropPicker->PickFromListOn();
	meshPropPicker->InitializePickList();
	if (meshCellPicker == NULL)
	{
		meshCellPicker = vtkSmartPointer<vtkCellPicker>::New();
	}
	meshCellPicker->PickFromListOn();
	meshCellPicker->InitializePickList();
	meshCellPicker->RemoveAllLocators();
	if (meshPointPicker == NULL)
	{
		meshPointPicker = vtkSmartPointer<vtkPointPicker>::New();
	}
	meshPointPicker->PickFromListOn();
	meshPointPicker->InitializePickList();
}

void LineWidgetRepresentation::reset()
{
	this->InteractionState = LineWidgetRepresentation::Outside;
	indexActiveNode = -1;
	nodesActors.clear();
	nodesTransformFilter.clear();
	nodePoints = NULL;
	lineActor = NULL;
	lineActor2D = NULL;
	lineSource = NULL;
	loopClosed = false;
}

vtkSmartPointer<vtkPoints> LineWidgetRepresentation::getPoints()
{
	return nodePoints;
}

void LineWidgetRepresentation::updateActiveNode(int x, int y)
{
	if (indexActiveNode == -1)
	{
		return;
	}
	double* pickedPos = new double[3];
	if (pickPosition(meshPropPicker, x, y, pickedPos) || pickPosition(meshPointPicker, x, y, pickedPos))
	{
		if (loopClosed && (indexActiveNode == 0 || indexActiveNode == nodesActors.size() - 1))
		{
			nodesActors.at(0)->SetPosition(pickedPos);
			nodesActors.at(nodesActors.size() - 1)->SetPosition(pickedPos);
			nodePoints->SetPoint(0, pickedPos);
			nodePoints->SetPoint(nodesActors.size() - 1, pickedPos);
			this->BuildRepresentation();
		}
		else
		{
			nodesActors.at(indexActiveNode)->SetPosition(pickedPos);
			nodePoints->SetPoint(indexActiveNode, pickedPos);
			this->BuildRepresentation();
		}
	}
	delete pickedPos;
}

void LineWidgetRepresentation::finishUpdateActiveNode(int x, int y)
{
	if (indexActiveNode == -1)
	{
		return;
	}
	double* pickedPos = new double[3];
	if (pickFinalPosition(x, y, pickedPos))
	{
		nodesActors.at(indexActiveNode)->SetPosition(pickedPos);
		nodePoints->SetPoint(indexActiveNode, pickedPos);
		this->BuildRepresentation();
		indexActiveNode = -1;
		this->InteractionState = LineWidgetRepresentation::Outside;
	}
	delete pickedPos;
}

void LineWidgetRepresentation::updateLine(int x, int y)
{
	if (lineActor != NULL || lineActor2D != NULL)
	{
		double* pickedPos = new double[3];
		if (pickPosition(meshPropPicker, x, y, pickedPos) || pickPosition(meshPointPicker, x, y, pickedPos))
		{
			nodePoints->SetPoint(nodePoints->GetNumberOfPoints() - 1, pickedPos);
		}
		delete pickedPos;
	}
}

bool LineWidgetRepresentation::pickNode(int x, int y)
{
	for (int i = 0; i < nodesActors.size(); i++)
	{
		double* p = getDisplayPosition(nodesActors.at(i)->GetPosition());
		if (x < p[0] + pixelTolerance && x > p[0] - pixelTolerance && y < p[1] + pixelTolerance && y > p[1] - pixelTolerance)
		{
			delete p;
			indexActiveNode = i;
			return true;
		}
		delete p;
	}
	indexActiveNode = -1;
	return false;
}

int LineWidgetRepresentation::pickPosition(vtkSmartPointer<vtkAbstractPicker> picker, int x, int y, double * point)
{
	if (representation2D)
	{
		double* wP = new double[4];
		vtkInteractorObserver::ComputeDisplayToWorld(Renderer, x, y, 1, wP);//1 to avoid giant nodes
		point[0] = wP[0];
		point[1] = wP[1];
		point[2] = wP[2];
		delete wP;
		return 1;
	}
	if (picker->Pick(x, y, 0, this->Renderer))
	{
		picker->GetPickPosition(point);
		return 1;
	}
	return 0;
}

int LineWidgetRepresentation::pickFinalPosition(int x, int y, double * point)
{
	if (pickPosition(meshCellPicker, x, y, point))
	{
		return 1;
	}
	else if (pickPosition(meshPointPicker, x, y, point))
	{
		return 1;
	}
	return 0;
}

void LineWidgetRepresentation::StartWidgetInteraction(double eventPos[2])
{
	this->StartEventPosition[0] = eventPos[0];
	this->StartEventPosition[1] = eventPos[1];
}

void LineWidgetRepresentation::WidgetInteraction(double eventPos[2])
{
	this->Modified();
	this->BuildRepresentation();
}

int LineWidgetRepresentation::ComputeInteractionState(int X, int Y, int vtkNotUsed(modify))
{
	if (X != 0 && Y != 0)
	{
		if (pickNode(X, Y))
		{
			this->InteractionState = LineWidgetRepresentation::OverNode;
			return this->InteractionState;
		}
	}
	this->InteractionState = LineWidgetRepresentation::Outside;
	return this->InteractionState;
}

vtkMTimeType LineWidgetRepresentation::GetMTime()
{
	vtkMTimeType mTime = this->Superclass::GetMTime();
	if (nodesTransformFilter.size() != 0)
	{
		mTime = std::max(mTime, this->nodesTransformFilter.at(0)->GetMTime());
	}
	return mTime;
}

void LineWidgetRepresentation::BuildRepresentation()
{
	if (this->Renderer && (this->GetMTime() > this->BuildTime || (this->Renderer->GetVTKWindow() && this->Renderer->GetVTKWindow()->GetMTime() > this->BuildTime)))
	{
		if (drawLine)
		{
			if (representation2D)
			{
				if (lineActor2D != NULL)
				{
					this->Renderer->RemoveActor2D(lineActor2D);
					lineActor2D = NULL;
				}
				if (nodePoints != NULL)
				{
					vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
					polyLine->GetPointIds()->SetNumberOfIds(nodePoints->GetNumberOfPoints());
					for (unsigned int i = 0; i < nodePoints->GetNumberOfPoints(); i++)
					{
						polyLine->GetPointIds()->SetId(i, i);
					}
					// Create a cell array to store the lines in and add the lines to it
					vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
					cells->InsertNextCell(polyLine);

					// Create a polydata to store everything in
					vtkSmartPointer<vtkPolyData> polyData2D = vtkSmartPointer<vtkPolyData>::New();
					polyData2D->Initialize();
					polyData2D->SetPoints(nodePoints);
					polyData2D->SetLines(cells);
					polyData2D->Modified();
					vtkSmartPointer<vtkCoordinate> coordinateLine2D = vtkSmartPointer<vtkCoordinate>::New();
					coordinateLine2D->SetCoordinateSystemToWorld();
					vtkSmartPointer<vtkPolyDataMapper2D> mapperLine2D = vtkSmartPointer<vtkPolyDataMapper2D>::New();
					mapperLine2D->SetInputData(polyData2D);
					mapperLine2D->SetTransformCoordinate(coordinateLine2D);
					mapperLine2D->ScalarVisibilityOn();
					mapperLine2D->SetScalarModeToUsePointData();
					mapperLine2D->Update();
					lineActor2D = vtkSmartPointer<vtkActor2D>::New();
					lineActor2D->SetMapper(mapperLine2D);
					lineActor2D->GetProperty()->SetLineWidth(3);
					lineActor2D->GetProperty()->SetColor(255, 0, 0);
				}
			}
			else
			{
				if (lineActor == NULL && nodePoints != NULL)
				{
					lineSource = vtkSmartPointer<vtkLineSource>::New();
					lineSource->SetPoints(nodePoints);
					//Create a mapper and actor
					vtkSmartPointer<vtkPolyDataMapper> mapperLine = vtkSmartPointer<vtkPolyDataMapper>::New();
					mapperLine->SetInputConnection(lineSource->GetOutputPort());
					lineActor = vtkSmartPointer<vtkActor>::New();
					lineActor->GetProperty()->SetColor(255, 0, 0);
					lineActor->GetProperty()->SetLineWidth(3);
					lineActor->SetMapper(mapperLine);
				}
				else if (lineActor != NULL)
				{
					lineSource->Modified();
				}
			}
		}

		double p1[4], p2[4];
		this->Renderer->GetActiveCamera()->GetFocalPoint(p1);
		p1[3] = 1.0;
		this->Renderer->SetWorldPoint(p1);
		this->Renderer->WorldToView();
		this->Renderer->GetViewPoint(p1);

		double depth = p1[2];
		double aspect[2];
		this->Renderer->ComputeAspect();
		this->Renderer->GetAspect(aspect);

		p1[0] = -aspect[0];
		p1[1] = -aspect[1];
		this->Renderer->SetViewPoint(p1);
		this->Renderer->ViewToWorld();
		this->Renderer->GetWorldPoint(p1);

		p2[0] = aspect[0];
		p2[1] = aspect[1];
		p2[2] = depth;
		p2[3] = 1.0;
		this->Renderer->SetViewPoint(p2);
		this->Renderer->ViewToWorld();
		this->Renderer->GetWorldPoint(p2);

		double distance = sqrt(vtkMath::Distance2BetweenPoints(p1, p2));

		int *size = this->Renderer->GetRenderWindow()->GetSize();
		double viewport[4];
		this->Renderer->GetViewport(viewport);

		double x, y, scale;

		x = size[0] * (viewport[2] - viewport[0]);
		y = size[1] * (viewport[3] - viewport[1]);

		scale = sqrt(x*x + y * y);


		distance = 1000 * distance / scale;
		scale = distance * this->HandleSize;
		vtkSmartPointer<vtkTransform> T = vtkSmartPointer<vtkTransform>::New();
		T->Scale(scale, scale, scale);
		for (int i = 0; i < nodesTransformFilter.size(); i++)
		{
			if (i == indexActiveNode)
			{
				vtkSmartPointer<vtkTransform> T2 = vtkSmartPointer<vtkTransform>::New();
				T2->Scale(4 * scale, 4 * scale, 4 * scale);
				nodesTransformFilter.at(i)->SetTransform(T2);
				nodesTransformFilter.at(i)->Update();
			}
			else
			{
				nodesTransformFilter.at(i)->SetTransform(T);
				nodesTransformFilter.at(i)->Update();
			}
		}

		// Set things up
		this->BuildTime.Modified();
	}
}

double * LineWidgetRepresentation::getDisplayPosition(double * point)
{
	double* display = new double[3];
	vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer, point[0], point[1], point[2], display);
	return display;
}

void LineWidgetRepresentation::GetActors(vtkPropCollection *pc)
{
	if (lineActor != NULL)
	{
		lineActor->GetActors(pc);
	}
	for (int i = 0; i < nodesActors.size(); i++)
	{
		nodesActors.at(i)->GetActors(pc);
	}
}

void LineWidgetRepresentation::GetActors2D(vtkPropCollection *pc)
{
	if (lineActor2D != NULL)
	{
		lineActor2D->GetActors(pc);
	}
}

void LineWidgetRepresentation::ReleaseGraphicsResources(vtkWindow *w)
{
	if (lineActor != NULL)
	{
		lineActor->ReleaseGraphicsResources(w);
	}
	if (lineActor2D != NULL)
	{
		lineActor2D->ReleaseGraphicsResources(w);
	}
	for (int i = 0; i < nodesActors.size(); i++)
	{
		nodesActors.at(i)->ReleaseGraphicsResources(w);
	}
}

int LineWidgetRepresentation::RenderOverlay(vtkViewport *w)
{
	this->BuildRepresentation();
	int count = 0;
	if (lineActor != NULL)
	{
		count += lineActor->RenderOverlay(w);
	}
	if (lineActor2D != NULL)
	{
		count += lineActor2D->RenderOverlay(w);
	}
	for (int i = 0; i < nodesActors.size(); i++)
	{
		count += nodesActors.at(i)->RenderOverlay(w);
	}
	return count;
}

int LineWidgetRepresentation::RenderOpaqueGeometry(vtkViewport *w)
{
	this->BuildRepresentation();

	if (lineActor != NULL && alwaysOnTop)
	{
		GLboolean flag = GL_FALSE;
		if (lineActor->GetVisibility())
		{
			glGetBooleanv(GL_DEPTH_TEST, &flag);
			if (flag)
			{
				glDisable(GL_DEPTH_TEST);
			}
		}
	}

	int count = 0;
	if (lineActor != NULL)
	{
		count += lineActor->RenderOpaqueGeometry(w);
	}
	if (lineActor2D != NULL)
	{
		count += lineActor2D->RenderOpaqueGeometry(w);
	}
	for (int i = 0; i < nodesActors.size(); i++)
	{
		count += nodesActors.at(i)->RenderOpaqueGeometry(w);
	}
	if (lineActor != NULL && alwaysOnTop)
	{
		if (lineActor->GetVisibility() && alwaysOnTop)
		{
			glEnable(GL_DEPTH_TEST);
		}
	}
	return count;
}

int LineWidgetRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport *w)
{
	this->BuildRepresentation();
	int count = 0;
	if (lineActor != NULL)
	{
		count += lineActor->RenderTranslucentPolygonalGeometry(w);
	}
	if (lineActor2D != NULL)
	{
		count += lineActor2D->RenderTranslucentPolygonalGeometry(w);
	}
	for (int i = 0; i < nodesActors.size(); i++)
	{
		count += nodesActors.at(i)->RenderTranslucentPolygonalGeometry(w);
	}
	return count;
}

// Description:
// Does this prop have some translucent polygonal geometry?
int LineWidgetRepresentation::HasTranslucentPolygonalGeometry()
{
	this->BuildRepresentation();
	int count = 0;
	if (lineActor != NULL)
	{
		count += lineActor->HasTranslucentPolygonalGeometry();
	}
	if (lineActor2D != NULL)
	{
		count += lineActor2D->HasTranslucentPolygonalGeometry();
	}
	for (int i = 0; i < nodesActors.size(); i++)
	{
		count += nodesActors.at(i)->HasTranslucentPolygonalGeometry();
	}
	return count;
}

void LineWidgetRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
}