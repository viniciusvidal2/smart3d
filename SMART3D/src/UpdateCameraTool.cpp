#include "UpdateCameraTool.h"

#include <sstream>

#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkImageActor.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

#include "DontShowAgainDialog.h"
#include "Utils.h"
#include "Camera.h"

vtkStandardNewMacro(UpdateCameraTool);

//----------------------------------------------------------------------------
UpdateCameraTool::UpdateCameraTool() : vtk3DWidget()
{
	this->EventCallbackCommand->SetCallback(UpdateCameraTool::ProcessEvents);
}

//----------------------------------------------------------------------------
UpdateCameraTool::~UpdateCameraTool()
{
	destruct();
}

void UpdateCameraTool::destruct()
{
	if (textActor)
	{
		this->CurrentRenderer->RemoveActor2D(textActor);
		textActor = nullptr;
		textProperty = nullptr;
	}
	if (camera)
	{
		if (camera->imageActor)
		{
			camera->imageActor->SetOpacity(1.0);
		}
		else if (camera->image360Actor)
		{
			camera->image360Actor->GetProperty()->SetOpacity(1.0);
		}
		camera = nullptr;
	}
}

//----------------------------------------------------------------------------
void UpdateCameraTool::SetEnabled(int enabling)
{
	if (!this->Interactor)
	{
		return;
	}

	if (enabling) //------------------------------------------------------------
	{
		if (this->Enabled) //already enabled, just return
		{
			return;
		}
		if (!this->CurrentRenderer)
		{
			this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(
				this->Interactor->GetLastEventPosition()[0],
				this->Interactor->GetLastEventPosition()[1]));
			if (this->CurrentRenderer == NULL)
			{
				return;
			}
		}
		this->Enabled = 1;

		//Set up text
		if (camera)
		{
			textProperty = vtkSmartPointer<vtkTextProperty>::New();
			textProperty->SetBold(1);
			textProperty->SetShadow(1);
			textProperty->SetFontFamilyToArial();
			textProperty->SetJustificationToCentered();
			textProperty->SetVerticalJustificationToCentered();

			textActor = vtkSmartPointer<vtkTextActor>::New();
			updateText();
			textActor->SetTextProperty(textProperty);
			updateTextScale();
			this->CurrentRenderer->AddActor2D(textActor);
		}
		if (showDialog)
		{
			DontShowAgainDialog* dialog = new DontShowAgainDialog(nullptr, "Intructions", 
				"----------------------------------\n"
				"W - Increase pitch/move camera up\n"
				"A - Increase yaw/move camera left\n"
				"S - Decrease pitch/move camera down\n"
				"D - Decrease yaw/move camera right\n"
				"----------------------------------\n"
				"Q - Increase focal distance\n"
				"E - Decrease focal distance\n"
				"----------------------------------\n"
				"R - Increase the step used to move the camera and change pitch/yaw\n"
				"F - Decrease the step used to move the camera and change pitch/yaw\n"
				"----------------------------------\n"
				"T - Change between move the camera and pitch/yaw\n"
			);
			dialog->ShowModal();
			showDialog = !dialog->getCheckBoxStatus();
			delete dialog;
		}

		// listen for the following events
		vtkRenderWindowInteractor *i = this->Interactor;
		i->AddObserver(vtkCommand::KeyPressEvent,
			this->EventCallbackCommand, this->Priority);
		i->AddObserver(vtkCommand::WindowResizeEvent,
			this->EventCallbackCommand, this->Priority);

		this->InvokeEvent(vtkCommand::EnableEvent, NULL);
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

		destruct();

		this->InvokeEvent(vtkCommand::DisableEvent, NULL);
		this->SetCurrentRenderer(NULL);
	}

	this->Interactor->Render();
}

//----------------------------------------------------------------------------
void UpdateCameraTool::ProcessEvents(vtkObject* vtkNotUsed(object),
	unsigned long event,
	void* clientdata,
	void* vtkNotUsed(calldata))
{
	UpdateCameraTool* self =
		reinterpret_cast<UpdateCameraTool *>(clientdata);


	//okay, let's do the right thing
	if (event == vtkCommand::KeyPressEvent)
	{
		self->OnKeyPressed();
	}
	else if (event == vtkCommand::WindowResizeEvent)
	{
		self->updateTextScale();
	}
	
}

void UpdateCameraTool::setCamera(Camera * cam)
{
	if (cam)
	{
		camera = cam;
		if (camera->imageActor)
		{
			camera->imageActor->SetOpacity(0.5);
		}
		else if (camera->image360Actor)
		{
			camera->image360Actor->GetProperty()->SetOpacity(0.5);
		}
	}
}

void UpdateCameraTool::OnKeyPressed()
{
	char key = this->Interactor->GetKeyCode();
	if (key == 'Q')
	{
		camera->updateFocalDistance(this->CurrentRenderer, camera->getFocalX() + step * 10, camera->getFocalY() + step * 10);
		Utils::updateCamera(this->CurrentRenderer, camera);
		this->CurrentRenderer->GetRenderWindow()->Render();
	}
	else if (key == 'E')
	{
		camera->updateFocalDistance(this->CurrentRenderer, camera->getFocalX() - step * 10, camera->getFocalY() - step * 10);
		Utils::updateCamera(this->CurrentRenderer, camera);
		this->CurrentRenderer->GetRenderWindow()->Render();
	}
	else if (key == 'A')
	{
		vtkSmartPointer<vtkMatrix4x4> matrixRt = camera->getMatrixRt();
		if (translate)
		{
			matrixRt->SetElement(0, 3, matrixRt->GetElement(0, 3) + step);
			camera->updateMatrixRt(this->CurrentRenderer, matrixRt);
		}
		else
		{
			vtkSmartPointer<vtkTransform> T = vtkSmartPointer<vtkTransform>::New();
			T->SetMatrix(matrixRt);
			double* yawVector = new double[3];
			camera->getYawVector(yawVector);
			T->RotateWXYZ(step, yawVector);
			delete yawVector;
			camera->updateMatrixRt(this->CurrentRenderer, T->GetMatrix());
		}
		Utils::updateCamera(this->CurrentRenderer, camera);
		this->CurrentRenderer->GetRenderWindow()->Render();
	}
	else if (key == 'D')
	{
		vtkSmartPointer<vtkMatrix4x4> matrixRt = camera->getMatrixRt();
		if (translate)
		{
			matrixRt->SetElement(0, 3, matrixRt->GetElement(0, 3) - step);
			camera->updateMatrixRt(this->CurrentRenderer, matrixRt);
		}
		else
		{
			vtkSmartPointer<vtkTransform> T = vtkSmartPointer<vtkTransform>::New();
			T->SetMatrix(matrixRt);
			double* yawVector = new double[3];
			camera->getYawVector(yawVector);
			T->RotateWXYZ(-step, yawVector);
			delete yawVector;
			camera->updateMatrixRt(this->CurrentRenderer, T->GetMatrix());
		}
		Utils::updateCamera(this->CurrentRenderer, camera);
		this->CurrentRenderer->GetRenderWindow()->Render();
	}
	else if (key == 'W')
	{
		vtkSmartPointer<vtkMatrix4x4> matrixRt = camera->getMatrixRt();
		if (translate)
		{
			matrixRt->SetElement(1, 3, matrixRt->GetElement(1, 3) + step);
			camera->updateMatrixRt(this->CurrentRenderer, matrixRt);
		}
		else
		{
			vtkSmartPointer<vtkTransform> T = vtkSmartPointer<vtkTransform>::New();
			T->SetMatrix(matrixRt);
			double* pitchVector = new double[3];
			camera->getPitchVector(pitchVector);
			T->RotateWXYZ(step, pitchVector);
			delete pitchVector;
			camera->updateMatrixRt(this->CurrentRenderer, T->GetMatrix());
		}
		Utils::updateCamera(this->CurrentRenderer, camera);
		this->CurrentRenderer->GetRenderWindow()->Render();
	}
	else if (key == 'S')
	{
		vtkSmartPointer<vtkMatrix4x4> matrixRt = camera->getMatrixRt();
		if (translate)
		{
			matrixRt->SetElement(1, 3, matrixRt->GetElement(1, 3) - step);
			camera->updateMatrixRt(this->CurrentRenderer, matrixRt);
		}
		else
		{
			vtkSmartPointer<vtkTransform> T = vtkSmartPointer<vtkTransform>::New();
			T->SetMatrix(matrixRt);
			double* pitchVector = new double[3];
			camera->getPitchVector(pitchVector);
			T->RotateWXYZ(-step, pitchVector);
			delete pitchVector;
			camera->updateMatrixRt(this->CurrentRenderer, T->GetMatrix());
		}
		Utils::updateCamera(this->CurrentRenderer, camera);
		this->CurrentRenderer->GetRenderWindow()->Render();
	}
	else if (key == 'R')
	{
		if (step < 10000)
		{
			step *= 10;
			updateText();
			this->CurrentRenderer->GetRenderWindow()->Render();
		}
	}
	else if (key == 'F')
	{
		if (step > 0.0001)
		{
			step /= 10;
			updateText();
			this->CurrentRenderer->GetRenderWindow()->Render();
		}
	}
	else if (key == 'T')
	{
		translate = !translate;
		updateText();
		this->CurrentRenderer->GetRenderWindow()->Render();
	}
}

void UpdateCameraTool::updateTextScale()
{
	double scale = this->CurrentRenderer->GetSize()[0] / 500.f;
	if (scale > 1.0 && scale < 5.0)
	{
		textActor->SetPosition(textXPosition * scale, textYPosition * scale);
		textProperty->SetFontSize(15 * scale);
	}
}

void UpdateCameraTool::updateText()
{
	std::stringstream ss;
	if (translate)
	{
		ss << "Move camera\nStep: " << step;
	}
	else
	{
		ss << "Pitch/yaw\nStep: " << step;
	}
	textActor->SetInput(ss.str().c_str());
}
