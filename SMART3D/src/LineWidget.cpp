#include "LineWidget.h"

#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkWidgetEvent.h>
#include <vtkEvent.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActor.h>

#include "Mesh.h"

vtkStandardNewMacro(LineWidget);


int LineWidget::getWidgetState()
{
  return WidgetState;
}

//-------------------------------------------------------------------------
LineWidget::LineWidget()
{
  this->WidgetState = LineWidget::Start;

  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
    vtkWidgetEvent::Select,
    this, LineWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
    vtkWidgetEvent::EndSelect,
    this, LineWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
    vtkWidgetEvent::Move,
    this, LineWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::KeyPressEvent,
    vtkEvent::ControlModifier, 90, 1, "Z",
    vtkWidgetEvent::Delete,
    this, LineWidget::CtrlKeyPress);
}

//-------------------------------------------------------------------------
LineWidget::~LineWidget() = default;


void LineWidget::SetEnabled(int enabling)
{
  // The handle widgets are not actually enabled until they are placed.
  // The handle widgets take their representation from the vtkContourRepresentation.
  if (enabling)
  {
    this->WidgetState = LineWidget::Start;
  }
  this->Superclass::SetEnabled(enabling);
}

//-------------------------------------------------------------------------
void LineWidget::SelectAction(vtkAbstractWidget *w)
{
  vtkSmartPointer<LineWidget> self = reinterpret_cast<LineWidget*>(w);
  //self->GrabFocus(self->EventCallbackCommand);
  vtkSmartPointer<LineWidgetRepresentation> rep = reinterpret_cast<LineWidgetRepresentation*>(self->WidgetRep);
  // Picked something inside the widget
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  if (self->WidgetState == LineWidget::Define)
  {
    if (rep->getNumberOfNodes() + 1 >= self->maxNumberOfNodes)
    {
      if (rep->addFinalNode(X, Y))
      {
        rep->Modified();
        self->Render();
        self->WidgetState = LineWidget::Finished;
        self->EventCallbackCommand->SetAbortFlag(1);
        self->EndInteraction();
        self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
      }
      return;
    }
    else if (self->closeLoopOnFirstNode && rep->getNumberOfNodes() >= 3 && rep->isOverFirstNode(X, Y))
    {
      rep->closeLoop();
      rep->Modified();
      self->Render();
      self->WidgetState = LineWidget::Finished;
      self->EventCallbackCommand->SetAbortFlag(1);
      self->EndInteraction();
      self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
      return;
    }
  }

  switch (self->WidgetState)
  {
  case LineWidget::Start:
  case LineWidget::Define:
  {
    rep->addNode(X, Y);
    rep->Modified();
    self->WidgetState = LineWidget::Define;
    self->EventCallbackCommand->SetAbortFlag(1);
    break;
  }
  case LineWidget::Finished:
  {
    rep->ComputeInteractionState(X, Y);
    if (rep->GetInteractionState() == LineWidgetRepresentation::OverNode)
    {
      self->WidgetState = LineWidget::Manipulate;
      self->EventCallbackCommand->SetAbortFlag(1);
    }
    break;
  }
  default:
    break;
  }
  double eventPos[2];
  eventPos[0] = X;
  eventPos[1] = Y;
  self->WidgetRep->StartWidgetInteraction(eventPos);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
  self->Render();
}

//-------------------------------------------------------------------------
void LineWidget::MoveAction(vtkAbstractWidget *w)
{
  LineWidget *self = reinterpret_cast<LineWidget*>(w);
  vtkSmartPointer<LineWidgetRepresentation> rep = reinterpret_cast<LineWidgetRepresentation*>(self->WidgetRep);

  // compute some info we need for all cases
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  if (self->WidgetState == LineWidget::Manipulate)
  {
    if (rep->GetInteractionState() == LineWidgetRepresentation::OverNode)
    {
		rep->updateActiveNode(X, Y);
    }
  }
  else if (self->WidgetState == LineWidget::Define)
  {
    rep->updateLine(X, Y);
  }
  else if (self->WidgetState == LineWidget::Finished)
  {
    rep->ComputeInteractionState(X, Y);
    rep->Modified();
    if (rep->GetInteractionState() == LineWidgetRepresentation::Outside)
    {
      self->Render();
      return;
    }
  }
  // start a drag
  self->EventCallbackCommand->SetAbortFlag(1);
  self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
  self->Render();
}

void LineWidget::CtrlKeyPress(vtkAbstractWidget *w)
{
  LineWidget *self = reinterpret_cast<LineWidget*>(w);
  vtkSmartPointer<LineWidgetRepresentation> rep = reinterpret_cast<LineWidgetRepresentation*>(self->WidgetRep);

  char key = self->Interactor->GetKeyCode();
  if (key == 'Z')
  {
    if (self->WidgetState == LineWidget::Finished)
    {
      int X = self->Interactor->GetEventPosition()[0];
      int Y = self->Interactor->GetEventPosition()[1];
      rep->ComputeInteractionState(X, Y);
      if (rep->GetInteractionState() == LineWidgetRepresentation::OverNode)
      {
        rep->removeActiveNode();
      }
      else
      {
        rep->reset();
      }
      rep->Modified();
    }
    else if (self->WidgetState == LineWidget::Start || self->WidgetState == LineWidget::Define)
    {
      rep->removeLastNode();
      rep->Modified();
    }

    if (rep->getNumberOfNodes() < self->maxNumberOfNodes && !rep->isLoopClosed())
    {
      self->WidgetState = LineWidget::Define;
    }
    else if (rep->getNumberOfNodes() == 0)
    {
      self->WidgetState = LineWidget::Start;
    }
    self->EventCallbackCommand->SetAbortFlag(1);
    self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
    self->Render();
  }
}


//-------------------------------------------------------------------------
void LineWidget::EndSelectAction(vtkAbstractWidget *w)
{
  LineWidget *self = reinterpret_cast<LineWidget*>(w);
  vtkSmartPointer<LineWidgetRepresentation> rep = reinterpret_cast<LineWidgetRepresentation*>(self->WidgetRep);

  if (self->WidgetRep->GetInteractionState() == LineWidgetRepresentation::Outside ||
    self->WidgetState != LineWidget::Manipulate)
  {
    return;
  }
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  rep->finishUpdateActiveNode(X, Y);
  self->WidgetState = LineWidget::Finished;

  // stop adjusting
  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void LineWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    this->WidgetRep = LineWidgetRepresentation::New();
  }
}

void LineWidget::CreateRepresentationFromMesh(Mesh * mesh)
{
	if (!this->WidgetRep)
	{
		this->WidgetRep = LineWidgetRepresentation::New();
	}
	LineWidgetRepresentation* rep = reinterpret_cast<LineWidgetRepresentation*>(this->WidgetRep);
	for (const auto& actor : mesh->actors)
	{
		rep->addProp(actor);
	}
	if (mesh->actors.size() == 1)// if we have a lot of actors the cell locator will delay the picking
	{
		rep->addLocator(mesh->getCellLocator());
	}
}

void LineWidget::setMaxNumberOfNodes(int maxnumberOfNodes)
{
  this->maxNumberOfNodes = maxnumberOfNodes;
}

bool LineWidget::hasFinished()
{
  return (this->WidgetState == LineWidget::Finished);
}

void LineWidget::setCloseLoopOnFirstNode(bool closeLoop)
{
  this->closeLoopOnFirstNode = closeLoop;
}

//-------------------------------------------------------------------------
void LineWidget::SelectRegion(double* vtkNotUsed(eventPos[2]))
{
  this->InvokeEvent(vtkCommand::WidgetActivateEvent, nullptr);
}

//-------------------------------------------------------------------------
void LineWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}