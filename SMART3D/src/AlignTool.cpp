#include "AlignTool.h"

#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkTransform.h>
#include <vtkRenderer.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkLandmarkTransform.h>

#include <wx/msgdlg.h>
#include "Utils.h"
#include "Mesh.h"
#include "LineWidget.h"

vtkStandardNewMacro(AlignTool);

//----------------------------------------------------------------------------
AlignTool::AlignTool() : vtk3DWidget()
{
}

//----------------------------------------------------------------------------
AlignTool::~AlignTool()
{
	if (lineWidget)
	{
		lineWidget->EnabledOff();
		lineWidget = nullptr;
	}
	meshSource = nullptr;
	meshTarget = nullptr;
	pointsSource = nullptr;
	pointsTarget = nullptr;
}

//----------------------------------------------------------------------------
void AlignTool::SetEnabled(int enabling)
{
	if (!this->Interactor)
	{
		return;
	}

	if (enabling) //------------------------------------------------------------
	{
		if (this->Enabled || !meshSource || !meshTarget) //already enabled, just return
		{
			return;
		}
		wxMessageBox("The " + Utils::getFileName(meshSource->filePath) + " is the source and the target is " +
			Utils::getFileName(meshTarget->filePath), "Information", wxICON_INFORMATION);
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

		this->setLineWidgetMesh(meshSource);

		this->InvokeEvent(vtkCommand::EnableEvent, nullptr);
	}

	else //disabling----------------------------------------------------------
	{
		if (!this->Enabled) //already disabled, just return
		{
			return;
		}
		this->Enabled = 0;

		if (lineWidget)
		{
			lineWidget->EnabledOff();
			lineWidget = nullptr;
		}
		meshSource = nullptr;
		meshTarget = nullptr;
		pointsSource = nullptr;
		pointsTarget = nullptr;
		this->InvokeEvent(vtkCommand::DisableEvent, nullptr);
		this->SetCurrentRenderer(nullptr);
	}

	this->Interactor->Render();
}

void AlignTool::setLineWidgetMesh(Mesh * mesh)
{
	if (!mesh)
	{
		return;
	}
	if (lineWidget)
	{
		lineWidget->EnabledOff();
		lineWidget = nullptr;
	}
	lineWidget = vtkSmartPointer<LineWidget>::New();
	lineWidget->SetInteractor(this->Interactor);
	lineWidget->setCloseLoopOnFirstNode(true);
	lineWidget->CreateRepresentationFromMesh(mesh);
	lineWidget->EnabledOn();
}

void AlignTool::enterKeyPressed()
{
	if (lineWidget)
	{
		if (!lineWidget->GetRepresentation()->isLoopClosed())
		{
			wxMessageBox("You need to close the loop", "Warning", wxICON_WARNING);
			return;
		}
		//Last point equal to the first
		const unsigned int numberOfPoints = lineWidget->GetRepresentation()->getPoints()->GetNumberOfPoints() - 1;
		vtkSmartPointer<vtkPoints> lineWidgetPoints = lineWidget->GetRepresentation()->getPoints();
		if (!pointsSource)
		{
			if (numberOfPoints < 4)
			{
				wxMessageBox("You need to select at least 4 points!", "Error", wxICON_ERROR);
				return;
			}
			pointsSource = vtkSmartPointer<vtkPoints>::New();
			for (size_t i = 0; i < numberOfPoints; i++)
			{
				pointsSource->InsertNextPoint(lineWidgetPoints->GetPoint(i));
			}
			this->setLineWidgetMesh(meshTarget);
			wxMessageBox("Select " + std::to_string(pointsSource->GetNumberOfPoints()) + " points in the " +
				Utils::getFileName(meshTarget->filePath) + " mesh/point cloud", "Information", wxICON_INFORMATION);
		}
		else
		{
			if (!pointsTarget)
			{
				if (numberOfPoints < pointsSource->GetNumberOfPoints() || numberOfPoints > pointsSource->GetNumberOfPoints())
				{
					wxMessageBox("You should select " + std::to_string(pointsSource->GetNumberOfPoints()) + " points!", "Error", wxICON_ERROR);
					if (lineWidget->GetRepresentation()->isLoopClosed())
					{
						setLineWidgetMesh(meshTarget);
						this->Interactor->Render();
					}
					return;
				}
				pointsTarget = vtkSmartPointer<vtkPoints>::New();
				for (size_t i = 0; i < numberOfPoints; i++)
				{
					pointsTarget->InsertNextPoint(lineWidgetPoints->GetPoint(i));
				}
				if (lineWidget)
				{
					lineWidget->EnabledOff();
					lineWidget->RemoveObserver(this->EventCallbackCommand);
					lineWidget = nullptr;
				}
				// Do the matching
				wxBeginBusyCursor();
				vtkNew<vtkLandmarkTransform> landmarkTransform;
				landmarkTransform->SetSourceLandmarks(pointsSource);
				landmarkTransform->SetTargetLandmarks(pointsTarget);
				vtkNew<vtkTransform> T;
				T->SetMatrix(landmarkTransform->GetMatrix());
				meshSource->transform(CurrentRenderer, T);
				wxEndBusyCursor();
				this->Interactor->Render();
			}
		}
	}
}