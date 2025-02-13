#include "ViewTool.h"

#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkWidgetEvent.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkMath.h>
#include <vtkCamera.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>


#include <wx/treelist.h>
#include <wx/msgdlg.h>

#include "Mesh.h"
#include "Utils.h"

vtkStandardNewMacro(ViewTool);


//-------------------------------------------------------------------------
ViewTool::ViewTool()
{
	this->WidgetState = ViewTool::Start;

	this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
		vtkWidgetEvent::Select,
		this, ViewTool::SelectAction);
	this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
		vtkWidgetEvent::EndSelect,
		this, ViewTool::EndSelectAction);
	this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
		vtkWidgetEvent::Move,
		this, ViewTool::MoveAction);
}

//-------------------------------------------------------------------------
ViewTool::~ViewTool() = default;

//-------------------------------------------------------------------------
void ViewTool::SetCursor(int cState)
{
	if (cState != ViewToolRepresentation::Outside)
	{
		this->RequestCursorShape(VTK_CURSOR_HAND);
	}
	else
	{
		this->RequestCursorShape(VTK_CURSOR_DEFAULT);
	}
}

Mesh * ViewTool::getMeshFromTree()
{
	wxTreeListItem item;
	if (meshVector->size() == 1)//Avoid annoying the user if there is just one mesh on the tree
	{
		item = meshVector->at(0)->getListItemMesh();
	}
	else
	{
		wxTreeListItems itens;
		if (treeMesh->GetSelections(itens))
		{
			item = itens[0];
		}
	}
	if (!item.IsOk())
	{
		wxMessageBox("Please select some mesh on the tree", "Error", wxICON_ERROR);
		return nullptr;
	}
	//If the person selects a texture or a camera, get the mesh item
	while (treeMesh->GetItemParent(item) != treeMesh->GetRootItem())
	{
		item = treeMesh->GetItemParent(item);
	}
	for (int i = 0; i < meshVector->size(); i++)
	{
		if (item == meshVector->at(i)->getListItemMesh())
		{
			treeMesh->CheckItem(item, wxCHK_CHECKED);
			meshVector->at(i)->setMeshVisibility(true);
			return meshVector->at(i);
		}
	}
	return nullptr;
}

void ViewTool::changeView(vtkSmartPointer<vtkRenderer> renderer, int interactionState)
{
	Mesh* mesh = getMeshFromTree();
	if (!mesh)
	{
		return;
	}
	vtkSmartPointer<vtkCamera> activeCamera = renderer->GetActiveCamera();
	vtkSmartPointer<vtkPolyData> polyData = mesh->getPolyData();
	double* bounds = polyData->GetBounds();
	double* center = polyData->GetCenter();
	double xBounds = (bounds[1] - bounds[0]) / 2.f;
	double yBounds = (bounds[3] - bounds[2]) / 2.f;
	double zBounds = (bounds[5] - bounds[4]) / 2.f;
	double distance;
	switch (interactionState)
	{
	case ViewToolRepresentation::TopView:
	case ViewToolRepresentation::BottomView:
		distance = sqrt(pow(xBounds, 2) + pow(yBounds, 2));
		break;
	case ViewToolRepresentation::LeftView:
	case ViewToolRepresentation::RightView:
		distance = sqrt(pow(xBounds, 2) + pow(zBounds, 2));
		break;
	case ViewToolRepresentation::FrontView:
	case ViewToolRepresentation::BackView:
		distance = sqrt(pow(yBounds, 2) + pow(zBounds, 2));
		break;
	default:
		distance = 0.0;
		break;
	}
	distance = distance / tan(vtkMath::RadiansFromDegrees(activeCamera->GetViewAngle() / 2.f));
	double position[3] = { center[0], center[1], center[2] };
	double viewUp[3];
	switch (interactionState)
	{
	case ViewToolRepresentation::TopView:
		position[2] += distance;
		Utils::createDoubleVector(0, 1, 0, viewUp);
		break;
	case ViewToolRepresentation::BottomView:
		position[2] -= distance;
		Utils::createDoubleVector(0, 1, 0, viewUp);
		break;
	case ViewToolRepresentation::LeftView:
		position[1] += distance;
		Utils::createDoubleVector(0, 0, 1, viewUp);
		break;
	case ViewToolRepresentation::RightView:
		position[1] -= distance;
		Utils::createDoubleVector(0, 0, 1, viewUp);
		break;
	case ViewToolRepresentation::FrontView:
		position[0] += distance;
		Utils::createDoubleVector(0, 0, 1, viewUp);
		break;
	case ViewToolRepresentation::BackView:
		position[0] -= distance;
		Utils::createDoubleVector(0, 0, 1, viewUp);
		break;
	default:
		position[2] += distance;
		Utils::createDoubleVector(0, 1, 0, viewUp);
		break;
	}
	activeCamera->SetFocalPoint(center);
	activeCamera->SetPosition(position);
	activeCamera->SetViewUp(viewUp);
	renderer->ResetCameraClippingRange();
}

//-------------------------------------------------------------------------
void ViewTool::SelectAction(vtkAbstractWidget *w)
{
	ViewTool *self = reinterpret_cast<ViewTool*>(w);

	// Picked something inside the widget
	int X = self->Interactor->GetEventPosition()[0];
	int Y = self->Interactor->GetEventPosition()[1];
	self->WidgetRep->ComputeInteractionState(X, Y);
	if (self->WidgetRep->GetInteractionState() == ViewToolRepresentation::Outside)
	{
		return;
	}

	// We are definitely selected
	self->GrabFocus(self->EventCallbackCommand);
	self->WidgetState = ViewTool::Selected;

	// This is redundant but necessary on some systems (windows) because the
	// cursor is switched during OS event processing and reverts to the default
	// cursor (i.e., the MoveAction may have set the cursor previously, but this
	// method is necessary to maintain the proper cursor shape)..
	self->SetCursor(self->WidgetRep->GetInteractionState());

	// convert to normalized viewport coordinates
	double XF = static_cast<double>(X);
	double YF = static_cast<double>(Y);
	self->CurrentRenderer->DisplayToNormalizedDisplay(XF, YF);
	self->CurrentRenderer->NormalizedDisplayToViewport(XF, YF);
	self->CurrentRenderer->ViewportToNormalizedViewport(XF, YF);
	double eventPos[2];
	eventPos[0] = XF;
	eventPos[1] = YF;
	self->WidgetRep->StartWidgetInteraction(eventPos);

	self->EventCallbackCommand->SetAbortFlag(1);
	self->StartInteraction();
	self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
}

//-------------------------------------------------------------------------
void ViewTool::TranslateAction(vtkAbstractWidget *w)
{
	ViewTool *self = reinterpret_cast<ViewTool*>(w);

	if (self->WidgetRep->GetInteractionState() == ViewToolRepresentation::Outside)
	{
		return;
	}

	// We are definitely selected
	self->GrabFocus(self->EventCallbackCommand);
	self->WidgetState = ViewTool::Selected;

	// Picked something inside the widget
	int X = self->Interactor->GetEventPosition()[0];
	int Y = self->Interactor->GetEventPosition()[1];

	// This is redundant but necessary on some systems (windows) because the
	// cursor is switched during OS event processing and reverts to the default
	// cursor.
	self->SetCursor(self->WidgetRep->GetInteractionState());

	// convert to normalized viewport coordinates
	double XF = static_cast<double>(X);
	double YF = static_cast<double>(Y);
	self->CurrentRenderer->DisplayToNormalizedDisplay(XF, YF);
	self->CurrentRenderer->NormalizedDisplayToViewport(XF, YF);
	self->CurrentRenderer->ViewportToNormalizedViewport(XF, YF);
	double eventPos[2];
	eventPos[0] = XF;
	eventPos[1] = YF;
	self->WidgetRep->StartWidgetInteraction(eventPos);

	self->EventCallbackCommand->SetAbortFlag(1);
	self->StartInteraction();
	self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
}

//-------------------------------------------------------------------------
void ViewTool::MoveAction(vtkAbstractWidget *w)
{
	ViewTool *self = reinterpret_cast<ViewTool*>(w);

	// compute some info we need for all cases
	int X = self->Interactor->GetEventPosition()[0];
	int Y = self->Interactor->GetEventPosition()[1];

	// Set the cursor appropriately
	if (self->WidgetState == ViewTool::Start)
	{
		int stateBefore = self->WidgetRep->GetInteractionState();
		self->WidgetRep->ComputeInteractionState(X, Y);
		int stateAfter = self->WidgetRep->GetInteractionState();
		self->SetCursor(stateAfter);
		return;
	}
	// Okay, adjust the representation (the widget is currently selected)
	double newEventPosition[2];
	newEventPosition[0] = static_cast<double>(X);
	newEventPosition[1] = static_cast<double>(Y);
	self->WidgetRep->WidgetInteraction(newEventPosition);

	// start a drag
	self->EventCallbackCommand->SetAbortFlag(1);
	self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
	self->Render();
}


//-------------------------------------------------------------------------
void ViewTool::EndSelectAction(vtkAbstractWidget *w)
{
	ViewTool *self = reinterpret_cast<ViewTool*>(w);

	if (self->WidgetRep->GetInteractionState() == ViewToolRepresentation::Outside ||
		self->WidgetState != ViewTool::Selected)
	{
		return;
	}

	if (self->WidgetRep->GetInteractionState() != ViewToolRepresentation::Outside)
	{
		self->changeView(self->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer(), self->WidgetRep->GetInteractionState());
		self->Render();
	}

	// Return state to not selected
	self->ReleaseFocus();
	self->WidgetState = ViewTool::Start;

	// stop adjusting
	self->EventCallbackCommand->SetAbortFlag(1);
	self->EndInteraction();
	self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
}

//----------------------------------------------------------------------
void ViewTool::CreateDefaultRepresentation()
{
	if (!this->WidgetRep)
	{
		this->WidgetRep = ViewToolRepresentation::New();
	}
}

//-------------------------------------------------------------------------
void ViewTool::SelectRegion(double* vtkNotUsed(eventPos[2]))
{
	this->InvokeEvent(vtkCommand::WidgetActivateEvent, nullptr);
}

//-------------------------------------------------------------------------
void ViewTool::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
}