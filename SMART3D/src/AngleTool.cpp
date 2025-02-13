#include "AngleTool.h"

#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

#include <vtkCaptionActor2D.h>
#include <vtkMath.h>
#include <vtkPoints.h>

#include "LineWidget.h"
#include "LineWidgetRepresentation.h"
#include "Mesh.h"
#include "Utils.h"
#include "Draw.h"

vtkStandardNewMacro(AngleTool);

//----------------------------------------------------------------------------
AngleTool::AngleTool() : vtk3DWidget()
{
	this->EventCallbackCommand->SetCallback(AngleTool::ProcessEvents);
}

//----------------------------------------------------------------------------
AngleTool::~AngleTool()
{
	if (textActor)
	{
		this->CurrentRenderer->RemoveActor2D(textActor);
		textActor = nullptr;
	}
	if (lineWidget)
	{
		lineWidget->EnabledOff();
		lineWidget->RemoveObserver(this->EventCallbackCommand);
		lineWidget = nullptr;
	}
	mesh = nullptr;
}

//----------------------------------------------------------------------------
void AngleTool::SetEnabled(int enabling)
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
		if (!lineWidget)
		{
			lineWidget = vtkSmartPointer<LineWidget>::New();
			lineWidget->SetInteractor(this->Interactor);
			lineWidget->setMaxNumberOfNodes(3);
			lineWidget->CreateRepresentationFromMesh(mesh);
		}
		lineWidget->EnabledOn();
		lineWidget->AddObserver(vtkCommand::StartInteractionEvent, this->EventCallbackCommand, this->Priority);
		lineWidget->AddObserver(vtkCommand::InteractionEvent, this->EventCallbackCommand, this->Priority);
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
		if (textActor)
		{
			this->CurrentRenderer->RemoveActor2D(textActor);
			textActor = nullptr;
		}
		if (lineWidget)
		{
			lineWidget->EnabledOff();
			lineWidget->RemoveObserver(this->EventCallbackCommand);
			lineWidget = nullptr;
		}
		mesh = nullptr;

		this->InvokeEvent(vtkCommand::DisableEvent, nullptr);
		this->SetCurrentRenderer(nullptr);
	}

	this->Interactor->Render();
}

double AngleTool::getAngle(double * point1, double * point2, double * point3)
{
	double v1[3];
	double v2[3];
	vtkMath::Subtract(point1, point2, v1);
	vtkMath::Subtract(point3, point2, v2);
	return vtkMath::DegreesFromRadians(vtkMath::AngleBetweenVectors(v1, v2));
}

//----------------------------------------------------------------------------
void AngleTool::ProcessEvents(vtkObject* vtkNotUsed(object),
	unsigned long event,
	void* clientdata,
	void* vtkNotUsed(calldata))
{
	AngleTool* self = reinterpret_cast<AngleTool *>(clientdata);

	//okay, let's do the right thing
	switch (event)
	{
	case vtkCommand::StartInteractionEvent:
	case vtkCommand::InteractionEvent:
	case vtkCommand::EndInteractionEvent:
		self->UpdateRepresentation();
		break;
	}
}

//----------------------------------------------------------------------------
void AngleTool::UpdateRepresentation()
{
	if (lineWidget)
	{
		vtkSmartPointer<vtkPoints> pointsLine = lineWidget->GetRepresentation()->getPoints();
		if (!pointsLine)
		{
			if (textActor)
			{
				this->CurrentRenderer->RemoveActor2D(textActor);
				textActor = nullptr;
				this->Interactor->Render();
			}
			return;
		}
		if (pointsLine->GetNumberOfPoints() != 3)
		{
			if (textActor)
			{
				this->CurrentRenderer->RemoveActor2D(textActor);
				textActor = nullptr;
				this->Interactor->Render();
			}
			return;
		}
		double point0[3];
		double point1[3];
		double point2[3];
		pointsLine->GetPoint(0, point0);
		pointsLine->GetPoint(1, point1);
		pointsLine->GetPoint(2, point2);
		if (!textActor)
		{
			textActor = Draw::createText(this->CurrentRenderer, point1, "", 24, 1.0, 1.0, 1.0);
		}
		const wxString ss = wxString::Format("%.2f", getAngle(point0, point1, point2)) + "\u00B0";
		textActor->SetAttachmentPoint(point1);
		textActor->SetCaption(ss.utf8_str());
	}
}