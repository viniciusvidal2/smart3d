#include "InteractorStyle.h"

#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkAbstractPicker.h>
#include <vtkPropPicker.h>
#include <vtkCellPicker.h>
#include <vtkPolygon.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkImageActor.h>
#include <vtkPoints.h>
#include <vtkImageMapper3D.h>
#include <vtkPlane.h>
#include <vtkMapper.h>

#include <wx/treelist.h>

#include "OutputErrorWindow.h"
#include "Utils.h"
#include "Camera.h"
#include "Mesh.h"

vtkStandardNewMacro(InteractorStyle);

InteractorStyle::~InteractorStyle()
{
}

void InteractorStyle::setMode360Image(bool mode360Image)
{
	this->mode360Image = mode360Image;
	OutputErrorWindow::setSuppressMessages(this->mode360Image);
}

void InteractorStyle::OnLeftButtonDown()
{
	leftButtonDown = true;
	if (!mode360Image)
	{
		// Forward events
		vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
	}
}

void InteractorStyle::OnLeftButtonUp()
{
	leftButtonDown = false;
	if (!mode360Image)
	{
		// Forward events
		vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
	}
}


void InteractorStyle::OnMouseWheelForward()
{
	if (mode360Image)
	{
		if (GetDefaultRenderer()->GetActiveCamera()->GetViewAngle() > 10)
		{
			GetDefaultRenderer()->GetActiveCamera()->SetViewAngle(GetDefaultRenderer()->GetActiveCamera()->GetViewAngle() - 2.0);
			GetDefaultRenderer()->GetRenderWindow()->Render();
		}
		return;
	}
	if (flyToPoint && picker->Pick(this->GetInteractor()->GetEventPosition()[0], this->GetInteractor()->GetEventPosition()[1], 0, this->GetDefaultRenderer()))
	{
		this->Interactor->FlyTo(this->GetDefaultRenderer(), picker->GetPickPosition());
	}
	else
	{
		// Forward events
		vtkInteractorStyleTrackballCamera::OnMouseWheelForward();
	}
}
void InteractorStyle::OnMouseWheelBackward()
{
	if (mode360Image)
	{
		if (GetDefaultRenderer()->GetActiveCamera()->GetViewAngle() < 100)
		{
			GetDefaultRenderer()->GetActiveCamera()->SetViewAngle(GetDefaultRenderer()->GetActiveCamera()->GetViewAngle() + 2.0);
			GetDefaultRenderer()->GetRenderWindow()->Render();
		}
		return;
	}
	// Forward events
	vtkInteractorStyleTrackballCamera::OnMouseWheelBackward();
}
void InteractorStyle::OnMouseMove()
{
	if (mode360Image)
	{
		if (GetDefaultRenderer() && leftButtonDown)
		{
			vtkSmartPointer<vtkCamera> camera = GetDefaultRenderer()->GetActiveCamera();
			// Camera Parameters ///////////////////////////////////////////////////
			double *focalPoint = camera->GetFocalPoint();
			double *viewUp = camera->GetViewUp();
			double *position = camera->GetPosition();
			double axis[3];
			axis[0] = -camera->GetViewTransformMatrix()->GetElement(0, 0);
			axis[1] = -camera->GetViewTransformMatrix()->GetElement(0, 1);
			axis[2] = -camera->GetViewTransformMatrix()->GetElement(0, 2);
			// Build The transformatio /////////////////////////////////////////////////
			vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
			transform->Identity();
			transform->Translate(position[0], position[1], position[2]);
			//The motion of the mouse will define the azimuth and elevation
			transform->RotateWXYZ((GetInteractor()->GetEventPosition()[0] - GetInteractor()->GetLastEventPosition()[0]) / 2.0, viewUp); // Azimuth
			transform->RotateWXYZ((GetInteractor()->GetEventPosition()[1] - GetInteractor()->GetLastEventPosition()[1]) / 2.0, axis);   // Elevation
			transform->Translate(-position[0], -position[1], -position[2]);

			double newPosition[3];
			transform->TransformPoint(position, newPosition); // Transform Position
			double newFocalPoint[3];
			transform->TransformPoint(focalPoint, newFocalPoint); // Transform Focal Point

			camera->SetPosition(newPosition);
			camera->SetFocalPoint(newFocalPoint);

			GetDefaultRenderer()->ResetCameraClippingRange();
			GetDefaultRenderer()->GetRenderWindow()->Render();
		}
		return;
	}
	// Forward events
	vtkInteractorStyleTrackballCamera::OnMouseMove();
}

void InteractorStyle::doubleClick()
{
	if (meshVector->size() > 0)
	{
		//Double click to go to the camera
		double clickPos[3];
		cellPicker->Pick(this->GetInteractor()->GetEventPosition()[0], this->GetInteractor()->GetEventPosition()[1], 0, this->GetDefaultRenderer());
		cellPicker->GetPickPosition(clickPos);
		if (cellPicker->GetMapper() != NULL)
		{
			for (auto mesh : *meshVector)
			{
				for (auto cam : mesh->cameras)
				{
					if (cam->imageActor)
					{
						if (cellPicker->GetMapper() == cam->imageActor->GetMapper())
						{
							Utils::updateCamera(this->GetDefaultRenderer(), cam);
							treeMesh->Select(cam->getListItemCamera());
							this->GetDefaultRenderer()->GetRenderWindow()->Render();
							break;
						}
					}
					else if (cam->image360ClickPoint)
					{
						if (cellPicker->GetMapper() == cam->image360ClickPoint->GetMapper())
						{
							cam->setVisibility(true);
							treeMesh->CheckItem(cam->getListItemCamera(), wxCHK_CHECKED);
							cam->createImageActor(this->GetDefaultRenderer());
							this->setMode360Image(true);
							mesh->update360ClickPoints(this->GetDefaultRenderer(), cam);
							Utils::updateCamera(this->GetDefaultRenderer(), cam);
							this->GetDefaultRenderer()->GetRenderWindow()->Render();
							break;
						}
					}
				}
			}
		}
	}
}

//Utils
int InteractorStyle::getMousePosition(vtkSmartPointer<vtkAbstractPicker> picker, double * point)
{
	if (picker->Pick(this->GetInteractor()->GetEventPosition()[0], this->GetInteractor()->GetEventPosition()[1], 0, this->GetDefaultRenderer()))
	{
		picker->GetPickPosition(point);
		return 1;
	}
	return 0;
}

int InteractorStyle::getMousePosition(vtkSmartPointer<vtkAbstractPicker> picker, double * pointDisplay, double * point)
{
	if (picker->Pick(pointDisplay[0], pointDisplay[1], 0, this->GetDefaultRenderer()))
	{
		picker->GetPickPosition(point);
		return 1;
	}
	return 0;
}

int InteractorStyle::getMousePosition(double * point)
{
	picker->Pick(this->GetInteractor()->GetEventPosition()[0], this->GetInteractor()->GetEventPosition()[1], 0, this->GetDefaultRenderer());
	picker->GetPickPosition(point);
	if (picker->GetActor() != NULL)
	{
		return 1;
	}
	return 0;
}

int InteractorStyle::getMousePosition(double * point, vtkSmartPointer<vtkActor> actor)
{
	if (actor == NULL)
	{
		return getMousePosition(point);
	}
	picker->Pick(this->GetInteractor()->GetEventPosition()[0], this->GetInteractor()->GetEventPosition()[1], 0, this->GetDefaultRenderer());
	picker->GetPickPosition(point);
	if (picker->GetActor() == actor)
	{
		return 1;
	}
	return 0;
}

double * InteractorStyle::pickPosition(double * pointDisplay)
{
	double* clickPos = new double[3];
	picker->Pick(pointDisplay, this->GetDefaultRenderer());
	picker->GetPickPosition(clickPos);
	if (picker->GetActor() != NULL)
	{
		return clickPos;
	}
	delete clickPos;
	return NULL;
}

bool InteractorStyle::IntersectPlaneWithLine(double * p1, vtkSmartPointer<vtkPolygon> polygon, double * pointCheckCam)
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
			//Draw::createSphere(this->GetDefaultRenderer(), pointIntersection, 0.007, 0, 0, 255);
			return true;
		}
		//Draw::createSphere(this->GetDefaultRenderer(), pointIntersection, 0.007, 255, 0, 0);
	}
	return false;
}

double * InteractorStyle::getDisplayPosition(double * point)
{
	double* display = new double[3];
	vtkInteractorObserver::ComputeWorldToDisplay(this->GetDefaultRenderer(), point[0], point[1], point[2], display);
	return display;
}
