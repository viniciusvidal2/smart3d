#include "AlignWithAxisTool.h"

#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>

#include "LineWidget.h"
#include "LineWidgetRepresentation.h"
#include "Utils.h"
#include "Mesh.h"

vtkStandardNewMacro(AlignWithAxisTool);

//----------------------------------------------------------------------------
AlignWithAxisTool::AlignWithAxisTool() : vtk3DWidget()
{
	this->EventCallbackCommand->SetCallback(AlignWithAxisTool::ProcessEvents);
}

//----------------------------------------------------------------------------
AlignWithAxisTool::~AlignWithAxisTool()
{
	if (lineWidget)
	{
		lineWidget->EnabledOff();
		lineWidget->RemoveObserver(this->EventCallbackCommand);
		lineWidget = nullptr;
	}
	mesh = nullptr;
}

//----------------------------------------------------------------------------
void AlignWithAxisTool::SetEnabled(int enabling)
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

		if (lineWidget)
		{
			lineWidget = vtkSmartPointer<LineWidget>::New();
			lineWidget->SetInteractor(this->Interactor);
			lineWidget->setMaxNumberOfNodes(2);
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
void AlignWithAxisTool::ProcessEvents(vtkObject* vtkNotUsed(object),
	unsigned long event,
	void* clientdata,
	void* vtkNotUsed(calldata))
{
	AlignWithAxisTool* self =
		reinterpret_cast<AlignWithAxisTool *>(clientdata);

	//okay, let's do the right thing
	switch (event)
	{
	case vtkCommand::EndInteractionEvent:
		self->UpdateRepresentation();
		break;
	}
}

//----------------------------------------------------------------------------
void AlignWithAxisTool::UpdateRepresentation()
{
	if (lineWidget)
	{
		vtkSmartPointer<vtkPoints> pointsLine = lineWidget->GetRepresentation()->getPoints();
		if (!pointsLine)
		{
			return;
		}
		if (pointsLine->GetNumberOfPoints() != 2)
		{
			return;
		}
		wxBeginBusyCursor();
		double point0[3];
		double point1[3];
		double upZVector[3] = {0.0, 0.0, 1.0};
		pointsLine->GetPoint(0, point0);
		pointsLine->GetPoint(1, point1);
		double vector[3];
		vtkMath::Subtract(point1, point0, vector);
		mesh->transform(this->CurrentRenderer, Utils::getTransformToAlignVectors(vector, upZVector));
		this->CurrentRenderer->ResetCamera(mesh->getPolyData()->GetBounds());
		this->CurrentRenderer->ResetCameraClippingRange();
		wxEndBusyCursor();
		this->Interactor->Render();
		this->SetEnabled(false);
	}

}
