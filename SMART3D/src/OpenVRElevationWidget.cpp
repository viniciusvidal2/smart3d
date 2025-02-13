#include "OpenVRElevationWidget.h"

#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkNew.h>
#include <vtkEventData.h>
#include <vtkWidgetEvent.h>

#include "OpenVRElevationWidgetRepresentation.h"
#include "Mesh.h"

vtkStandardNewMacro(OpenVRElevationWidget);

//----------------------------------------------------------------------
OpenVRElevationWidget::OpenVRElevationWidget()
{
  this->WidgetState = OpenVRElevationWidget::Start;

  /*{
    vtkNew<vtkEventDataButton3D> ed;
    ed->SetDevice(vtkEventDataDevice::RightController);
    ed->SetInput(vtkEventDataDeviceInput::Trigger);
    ed->SetAction(vtkEventDataAction::Press);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::Button3DEvent,
      ed, vtkWidgetEvent::Select3D,
      this, OpenVRElevationWidget::SelectAction3D);
  }*/
  
  /*{
    vtkNew<vtkEventDataButton3D> ed;
    ed->SetDevice(vtkEventDataDevice::RightController);
    ed->SetInput(vtkEventDataDeviceInput::TrackPad);
    ed->SetAction(vtkEventDataAction::Press);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::Button3DEvent,
      ed, vtkWidgetEvent::Move3D,
      this, OpenVRElevationWidget::TrackpadAction3D);
  }*/

  {
    vtkNew<vtkEventDataButton3D> ed;
    ed->SetDevice(vtkEventDataDevice::RightController);
    ed->SetInput(vtkEventDataDeviceInput::Grip);
    ed->SetAction(vtkEventDataAction::Press);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::Button3DEvent,
      ed, vtkWidgetEvent::Move,
      this, OpenVRElevationWidget::GripAction3D);
  }

  /*{
    vtkNew<vtkEventDataButton3D> ed;
    ed->SetDevice(vtkEventDataDevice::RightController);
    ed->SetInput(vtkEventDataDeviceInput::Trigger);
    ed->SetAction(vtkEventDataAction::Release);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::Button3DEvent,
      ed, vtkWidgetEvent::EndSelect3D,
      this, OpenVRElevationWidget::EndSelectAction3D);
  }*/
}

//----------------------------------------------------------------------
OpenVRElevationWidget::~OpenVRElevationWidget()
{
}

//----------------------------------------------------------------------
void OpenVRElevationWidget::
SetRepresentation(OpenVRElevationWidgetRepresentation* rep)
{
  this->Superclass::SetWidgetRepresentation(
    reinterpret_cast<vtkWidgetRepresentation*>(rep));
}

//----------------------------------------------------------------------
void OpenVRElevationWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    this->WidgetRep = OpenVRElevationWidgetRepresentation::New();
  }
}

void OpenVRElevationWidget::SetEnabled(int enabling)
{
  if (enabling)
  {
    if (mesh != NULL)
    {
      mesh->setMeshVisibility(false);
    }
  }
  else
  {
    if (mesh != NULL)
    {
      mesh->setMeshVisibility(true);
    }
    this->WidgetState = OpenVRElevationWidget::Start;
  }
  this->Superclass::SetEnabled(enabling);
}

void OpenVRElevationWidget::setMesh(Mesh * mesh)
{
  this->mesh = mesh;
}

//----------------------------------------------------------------------
void OpenVRElevationWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-------------------------------------------------------------------------
void OpenVRElevationWidget::SelectAction3D(vtkAbstractWidget *w)
{
  OpenVRElevationWidget *self = reinterpret_cast<OpenVRElevationWidget *>(w);

  // We want to compute an orthogonal vector to the plane that has been selected
  int interactionState = self->WidgetRep->ComputeComplexInteractionState(
    self->Interactor, self, vtkWidgetEvent::Select3D, self->CallData);

  if (interactionState == OpenVRElevationWidgetRepresentation::Outside)
  {
    return;
  }

  // We are definitely selected
  if (!self->Parent)
  {
    self->GrabFocus(self->EventCallbackCommand);
  }

  self->WidgetState = OpenVRElevationWidget::Active;
  self->WidgetRep->StartComplexInteraction(
    self->Interactor, self, vtkWidgetEvent::Select3D, self->CallData);

  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
}

void OpenVRElevationWidget::TrackpadAction3D(vtkAbstractWidget *w)
{
  OpenVRElevationWidget *self = reinterpret_cast<OpenVRElevationWidget *>(w);

  // Okay, adjust the representation
  self->WidgetRep->ComplexInteraction(
    self->Interactor, self, vtkWidgetEvent::Move3D, self->CallData);

  // moving something
  self->EventCallbackCommand->SetAbortFlag(1);
  self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
}

void OpenVRElevationWidget::GripAction3D(vtkAbstractWidget *w)
{
  OpenVRElevationWidget *self = reinterpret_cast<OpenVRElevationWidget *>(w);


  OpenVRElevationWidgetRepresentation *rep = reinterpret_cast<OpenVRElevationWidgetRepresentation *>(self->WidgetRep);
  rep->setDefineZeroByPoint(!rep->getDefineZeroByPoint());

  // moving something
  self->EventCallbackCommand->SetAbortFlag(1);
  self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
}

//----------------------------------------------------------------------
void OpenVRElevationWidget::EndSelectAction3D(vtkAbstractWidget *w)
{
  OpenVRElevationWidget *self = reinterpret_cast<OpenVRElevationWidget *>(w);

  // See whether we're active
  if (self->WidgetState != OpenVRElevationWidget::Active ||
    self->WidgetRep->GetInteractionState() == OpenVRElevationWidgetRepresentation::Outside)
  {
    return;
  }

  // Return state to not selected
  self->WidgetRep->EndComplexInteraction(
    self->Interactor, self, vtkWidgetEvent::EndSelect3D, self->CallData);

  self->WidgetState = OpenVRElevationWidget::Start;
  if (!self->Parent)
  {
    self->ReleaseFocus();
  }

  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
}
