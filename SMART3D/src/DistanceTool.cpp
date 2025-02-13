#include "DistanceTool.h"

#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCaptionActor2D.h>
#include <vtkMath.h>
#include <vtkPoints.h>

#include <wx/textdlg.h>
#include <wx/msgdlg.h>

#include "LineWidget.h"
#include "Draw.h"
#include "Utils.h"
#include "Calibration.h"
#include "Mesh.h"

vtkStandardNewMacro(DistanceTool);

bool DistanceTool::hasFinished() const
{
	if (lineWidget)
	{
		return lineWidget->hasFinished();
	}
	return false;
}

void DistanceTool::updateCalibration()
{
	wxString realMeasureTxt = wxGetTextFromUser("Please insert the real object measure with the measure unit", "Calibration", "Ex.30cm");
	double realMeasure;
	realMeasureTxt.ToDouble(&realMeasure);
	if (realMeasure <= 0)
	{
		wxMessageBox("Wrong Number", "Error", wxICON_ERROR);
		return;
	}
	char c;
	wxString measureUnit;
	for (int i = 0; i < realMeasureTxt.length(); i++)
	{
		c = realMeasureTxt.at(i);
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		{
			measureUnit << c;
		}
	}

	mesh->setCalibration(new Calibration(realMeasure / distance, measureUnit.ToStdString()));
	this->UpdateRepresentation();
	this->Interactor->GetRenderWindow()->Render();
}

//----------------------------------------------------------------------------
DistanceTool::DistanceTool() : vtk3DWidget()
{
	this->EventCallbackCommand->SetCallback(DistanceTool::ProcessEvents);
}

//----------------------------------------------------------------------------
DistanceTool::~DistanceTool()
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
	}
	mesh = nullptr;
}

//----------------------------------------------------------------------------
void DistanceTool::SetEnabled(int enabling)
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
			lineWidget->setMaxNumberOfNodes(2);
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

//----------------------------------------------------------------------------
void DistanceTool::ProcessEvents(vtkObject* vtkNotUsed(object),
	unsigned long event,
	void* clientdata,
	void* vtkNotUsed(calldata))
{
	DistanceTool* self =
		reinterpret_cast<DistanceTool *>(clientdata);

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
void DistanceTool::UpdateRepresentation()
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
		if (pointsLine->GetNumberOfPoints() != 2)
		{
			return;
		}
		double point0[3];
		double point1[3];
		pointsLine->GetPoint(0, point0);
		pointsLine->GetPoint(1, point1);
		this->distance = sqrt(vtkMath::Distance2BetweenPoints(point0, point1));
		double attachmentPoint[3];
		Utils::getMidpoint(point0, point1, attachmentPoint);
		if (!textActor)
		{
			textActor = Draw::createText(this->CurrentRenderer, attachmentPoint, "", 24, 1.0, 1.0, 1.0);
		}
		textActor->SetAttachmentPoint(attachmentPoint);
		textActor->SetCaption(mesh->getCalibration()->getCalibratedText(distance,
			abs(point0[0] - point1[0]),
			abs(point0[1] - point1[1]),
			abs(point0[2] - point1[2])).c_str());
	}

}
