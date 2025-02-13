#include "CheckCamTool.h"

#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkCellPicker.h>
#include <vtkCellLocator.h>
#include <vtkPoints.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPlane.h>
#include <vtkPolygon.h>

#include "LineWidget.h"
#include "Draw.h"
#include "Utils.h"
#include "Camera.h"
#include "Mesh.h"

vtkStandardNewMacro(CheckCamTool);

//----------------------------------------------------------------------------
CheckCamTool::CheckCamTool() : vtk3DWidget()
{
	this->EventCallbackCommand->SetCallback(CheckCamTool::ProcessEvents);
}

//----------------------------------------------------------------------------
CheckCamTool::~CheckCamTool()
{
	destruct();
	treeMesh = nullptr;
}

//----------------------------------------------------------------------------
void CheckCamTool::SetEnabled(int enabling)
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

		createPicker();

		// listen for the following events
		if (!lineWidget)
		{
			lineWidget = vtkSmartPointer<LineWidget>::New();
			lineWidget->SetInteractor(this->Interactor);
			lineWidget->setCloseLoopOnFirstNode(true);
			lineWidget->CreateRepresentationFromMesh(mesh);
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
		destruct();

		this->InvokeEvent(vtkCommand::DisableEvent, nullptr);
		this->SetCurrentRenderer(nullptr);
	}

	this->Interactor->Render();
}

//----------------------------------------------------------------------------
void CheckCamTool::ProcessEvents(vtkObject* vtkNotUsed(object),
	unsigned long event,
	void* clientdata,
	void* vtkNotUsed(calldata))
{
	CheckCamTool* self =
		reinterpret_cast<CheckCamTool *>(clientdata);

	//okay, let's do the right thing
	switch (event)
	{
	case vtkCommand::EndInteractionEvent:
		self->checkVisibility();
		break;
	}
}

void CheckCamTool::createPicker()
{
	if (!cellPicker)
	{
		cellPicker = vtkSmartPointer<vtkCellPicker>::New();
	}
	cellPicker->InitializePickList();
	cellPicker->RemoveAllLocators();
	mesh->createPickList(cellPicker);
	cellPicker->AddLocator(mesh->getCellLocator());
}

//----------------------------------------------------------------------------
void CheckCamTool::checkVisibility()
{
	if (!lineWidget)
	{
		return;
	}
	vtkSmartPointer<LineWidgetRepresentation> rep = lineWidget->GetRepresentation();
	if (!rep->isLoopClosed())
	{
		return;
	}
	wxBeginBusyCursor();
	vtkSmartPointer<vtkPoints> pointsLine = rep->getPoints();
	clearVisibleCameras();
	for (int i = 0; i < pointsLine->GetNumberOfPoints(); i++)
	{
		validPoints.push_back(Draw::createSphere(CurrentRenderer, pointsLine->GetPoint(i), 0.01, 255, 0, 0));
	}
	//Store the initial camera
	vtkSmartPointer<vtkCamera> initialCamera = vtkSmartPointer<vtkCamera>::New();
	initialCamera->DeepCopy(CurrentRenderer->GetActiveCamera());
	for (int i = 0; i < mesh->cameras.size(); i++)
	{
		Camera* cam = mesh->cameras.at(i);
		if (cam->imagePolygon)
		{
			//To avoid wasting time and processing, we first do a simple test to see if the point is in the camera FOV.
			int cont = 0;
			bool intersections = 1;
			while (intersections != 0 && cont < validPoints.size())
			{
				if (!intersectPlaneWithLine(cam->cameraPoints.at(0), cam->imagePolygon, validPoints.at(cont)->GetCenter()))
				{
					intersections = 0;
				}
				cont++;
			}
			//If every point is seen by the camera we check if there is anything obstructing the vision
			if (intersections)
			{
				Utils::updateCamera(CurrentRenderer, cam);
				int cont = 0;
				bool pointsVisibles = 1;
				while (pointsVisibles != 0 && cont < validPoints.size())
				{
					double pointCheckCamDisplay[3];
					Utils::getDisplayPosition(CurrentRenderer, validPoints.at(cont)->GetCenter(), pointCheckCamDisplay);
					double clickPos[3];
					Utils::pickPosition(CurrentRenderer, cellPicker, pointCheckCamDisplay, clickPos);
					if (vtkMath::Distance2BetweenPoints(clickPos, validPoints.at(cont)->GetCenter()) > 0.003)
					{
						pointsVisibles = 0;
					}
					cont++;
				}
				if (pointsVisibles)
				{
					visibleCameras.push_back(cam);
				}
			}
		}
		cam = nullptr;
	}
	CurrentRenderer->SetActiveCamera(initialCamera);
	for (int i = 0; i < visibleCameras.size(); i++)
	{
		treeMesh->SetItemImage(visibleCameras.at(i)->getListItemCamera(), 0);
		visibleCameras.at(i)->actorFrustum->GetProperty()->SetColor(0, 255, 0);
	}
	treeMesh->Expand(mesh->getListItemMeshCameras());
	CurrentRenderer->GetRenderWindow()->Render();
	wxEndBusyCursor();
}

void CheckCamTool::destruct()
{
	clearVisibleCameras();
	if (lineWidget)
	{
		lineWidget->EnabledOff();
		lineWidget->RemoveObserver(this->EventCallbackCommand);
		lineWidget = nullptr;
	}
	mesh = nullptr;
	cellPicker = nullptr;
}

void CheckCamTool::clearVisibleCameras()
{
	//clear the OK icon of the tree and the color of the camera actors
	for (int i = 0; i < visibleCameras.size(); i++)
	{
		treeMesh->SetItemImage(visibleCameras.at(i)->getListItemCamera(), -1);
		visibleCameras.at(i)->actorFrustum->GetProperty()->SetColor(255, 255, 255);
	}
	visibleCameras.clear();
	for (int i = 0; i < validPoints.size(); i++)
	{
		this->CurrentRenderer->RemoveActor(validPoints.at(i));
	}
	validPoints.clear();
}

bool CheckCamTool::intersectPlaneWithLine(double * p1, vtkSmartPointer<vtkPolygon> polygon, double * pointCheckCam)
{
	double n[3];
	polygon->ComputeNormal(polygon->GetPoints()->GetNumberOfPoints(), static_cast<double*>(polygon->GetPoints()->GetData()->GetVoidPointer(0)), n);
	double bounds[6];
	polygon->GetPoints()->GetBounds(bounds);
	double t;
	double intersection[3];
	if (vtkPlane::IntersectWithLine(p1, pointCheckCam, n, polygon->GetPoints()->GetPoint(0), t, intersection))
	{
		if (polygon->PointInPolygon(intersection, polygon->GetPoints()->GetNumberOfPoints(), static_cast<double*>(polygon->GetPoints()->GetData()->GetVoidPointer(0)), bounds, n))
		{
			return true;
		}
	}
	return false;
}