#include "OpenVRDeletePointsWidget.h"

#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkNew.h>
#include <vtkEventData.h>
#include <vtkWidgetEvent.h>

#include "OpenVRDeletePointsWidgetRepresentation.h"

vtkStandardNewMacro(OpenVRDeletePointsWidget);

//----------------------------------------------------------------------
OpenVRDeletePointsWidget::OpenVRDeletePointsWidget()
{
  this->WidgetState = OpenVRDeletePointsWidget::Start;

  {
    vtkNew<vtkEventDataButton3D> ed;
    ed->SetDevice(vtkEventDataDevice::RightController);
    ed->SetInput(vtkEventDataDeviceInput::Trigger);
    ed->SetAction(vtkEventDataAction::Press);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::Button3DEvent,
      ed, vtkWidgetEvent::Select3D,
      this, OpenVRDeletePointsWidget::SelectAction3D);
  }

  {
    vtkNew<vtkEventDataButton3D> ed;
    ed->SetDevice(vtkEventDataDevice::RightController);
    ed->SetInput(vtkEventDataDeviceInput::Trigger);
    ed->SetAction(vtkEventDataAction::Release);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::Button3DEvent,
      ed, vtkWidgetEvent::EndSelect3D,
      this, OpenVRDeletePointsWidget::EndSelectAction3D);
  }

  {
    vtkNew<vtkEventDataButton3D> ed;
    ed->SetDevice(vtkEventDataDevice::RightController);
    ed->SetInput(vtkEventDataDeviceInput::Grip);
    ed->SetAction(vtkEventDataAction::Press);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::Button3DEvent,
      ed, vtkWidgetEvent::EndRotate,
      this, OpenVRDeletePointsWidget::deletePointsAction3D);
  }

  {
    vtkNew<vtkEventDataMove3D> ed;
    ed->SetDevice(vtkEventDataDevice::RightController);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::Move3DEvent,
      ed, vtkWidgetEvent::Move3D,
      this, OpenVRDeletePointsWidget::MoveAction3D);
  }
}

//----------------------------------------------------------------------
OpenVRDeletePointsWidget::~OpenVRDeletePointsWidget()
{
}

//----------------------------------------------------------------------
void OpenVRDeletePointsWidget::
SetRepresentation(OpenVRDeletePointsWidgetRepresentation* rep)
{
  this->Superclass::SetWidgetRepresentation(
    reinterpret_cast<vtkWidgetRepresentation*>(rep));
}

//----------------------------------------------------------------------
void OpenVRDeletePointsWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    this->WidgetRep = OpenVRDeletePointsWidgetRepresentation::New();
  }
}

void OpenVRDeletePointsWidget::SetEnabled(int enabling)
{
  if (!enabling)
  {
    this->WidgetState = OpenVRDeletePointsWidget::Start;
    reinterpret_cast<OpenVRDeletePointsWidgetRepresentation*>(this->WidgetRep)->disable();
  }
  this->Superclass::SetEnabled(enabling);
}

//----------------------------------------------------------------------
void OpenVRDeletePointsWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-------------------------------------------------------------------------
void OpenVRDeletePointsWidget::SelectAction3D(vtkAbstractWidget *w)
{
  OpenVRDeletePointsWidget *self = reinterpret_cast<OpenVRDeletePointsWidget *>(w);

  // We want to compute an orthogonal vector to the plane that has been selected
  int interactionState = self->WidgetRep->ComputeComplexInteractionState(
    self->Interactor, self, vtkWidgetEvent::Select3D, self->CallData);

  if (interactionState == OpenVRDeletePointsWidgetRepresentation::Outside)
  {
    return;
  }

  // We are definitely selected
  if (!self->Parent)
  {
    self->GrabFocus(self->EventCallbackCommand);
  }

  self->WidgetState = OpenVRDeletePointsWidget::Active;
  self->WidgetRep->StartComplexInteraction(
    self->Interactor, self, vtkWidgetEvent::Select3D, self->CallData);

  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
}

//----------------------------------------------------------------------
void OpenVRDeletePointsWidget::MoveAction3D(vtkAbstractWidget *w)
{
  OpenVRDeletePointsWidget *self = reinterpret_cast<OpenVRDeletePointsWidget *>(w);

  // Okay, adjust the representation
  self->WidgetRep->ComplexInteraction(
    self->Interactor, self, vtkWidgetEvent::Move3D, self->CallData);

  // moving something
  //self->EventCallbackCommand->SetAbortFlag(1); commented to allow the user to navigate with the touch pad
  //self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
}

void OpenVRDeletePointsWidget::deletePointsAction3D(vtkAbstractWidget *w)
{
  OpenVRDeletePointsWidget *self = reinterpret_cast<OpenVRDeletePointsWidget *>(w);
  reinterpret_cast<OpenVRDeletePointsWidgetRepresentation*>(self->WidgetRep)->deletePoints();
}

//----------------------------------------------------------------------
void OpenVRDeletePointsWidget::EndSelectAction3D(vtkAbstractWidget *w)
{
  OpenVRDeletePointsWidget *self = reinterpret_cast<OpenVRDeletePointsWidget *>(w);

  // See whether we're active
  if (self->WidgetState != OpenVRDeletePointsWidget::Active ||
    self->WidgetRep->GetInteractionState() == OpenVRDeletePointsWidgetRepresentation::Outside)
  {
    return;
  }

  // Return state to not selected
  self->WidgetRep->EndComplexInteraction(
    self->Interactor, self, vtkWidgetEvent::EndSelect3D, self->CallData);

  self->WidgetState = OpenVRDeletePointsWidget::Start;
  if (!self->Parent)
  {
    self->ReleaseFocus();
  }

  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
}
