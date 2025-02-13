#include "AxisWidget.h"

#include <vtkObjectFactory.h>
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCoordinate.h>

#include <vtkActor2D.h>
#include <vtkProperty2D.h>
#include <vtkPolyDataMapper2D.h>

vtkStandardNewMacro(AxisWidget);

vtkCxxSetObjectMacro(AxisWidget, OrientationMarker, vtkProp);

class AxisWidgetWidgetObserver : public vtkCommand
{
public:
  static AxisWidgetWidgetObserver *New()
  {
    return new AxisWidgetWidgetObserver;
  };

  AxisWidgetWidgetObserver()
  {
    this->OrientationMarkerWidget = nullptr;
  }

  void Execute(vtkObject* wdg, unsigned long event, void *calldata) override
  {
    if (this->OrientationMarkerWidget)
    {
      this->OrientationMarkerWidget->ExecuteCameraUpdateEvent(wdg, event, calldata);
    }
  }

  AxisWidget *OrientationMarkerWidget;
};

//-------------------------------------------------------------------------
AxisWidget::AxisWidget()
{
  this->StartEventObserverId = 0;
  this->EventCallbackCommand->SetCallback(AxisWidget::ProcessEvents);

  this->Observer = AxisWidgetWidgetObserver::New();
  this->Observer->OrientationMarkerWidget = this;

  this->Tolerance = 0;
  this->Moving = 0;

  this->Viewport[0] = 0.0;
  this->Viewport[1] = 0.0;
  this->Viewport[2] = 0.2;
  this->Viewport[3] = 0.2;

  this->Renderer = vtkRenderer::New();
  this->Renderer->SetLayer(1);
  this->Renderer->InteractiveOff();

  this->Priority = 0.55;
  this->OrientationMarker = nullptr;
  this->State = AxisWidget::Outside;
  this->Interactive = 0;

  this->Outline = vtkPolyData::New();
  this->Outline->Allocate();
  vtkPoints *points = vtkPoints::New();
  vtkIdType ptIds[5];
  ptIds[4] = ptIds[0] = points->InsertNextPoint(1, 1, 0);
  ptIds[1] = points->InsertNextPoint(2, 1, 0);
  ptIds[2] = points->InsertNextPoint(2, 2, 0);
  ptIds[3] = points->InsertNextPoint(1, 2, 0);

  this->Outline->SetPoints(points);
  this->Outline->InsertNextCell(VTK_POLY_LINE, 5, ptIds);

  vtkCoordinate *tcoord = vtkCoordinate::New();
  tcoord->SetCoordinateSystemToDisplay();

  vtkPolyDataMapper2D *mapper = vtkPolyDataMapper2D::New();
  mapper->SetInputData(this->Outline);
  mapper->SetTransformCoordinate(tcoord);

  this->OutlineActor = vtkActor2D::New();
  this->OutlineActor->SetMapper(mapper);
  this->OutlineActor->SetPosition(0, 0);
  this->OutlineActor->SetPosition2(1, 1);
  this->OutlineActor->VisibilityOff();
  this->OutlineActor->SetPickable(false);
  this->OutlineActor->SetDragable(false);

  points->Delete();
  mapper->Delete();
  tcoord->Delete();
}

//-------------------------------------------------------------------------
AxisWidget::~AxisWidget()
{
  if (this->Enabled)
  {
    this->TearDownWindowInteraction();
  }

  this->Observer->Delete();
  this->Observer = nullptr;
  this->Renderer->Delete();
  this->Renderer = nullptr;
  this->SetOrientationMarker(nullptr);
  this->OutlineActor->Delete();
  this->Outline->Delete();
}

//-------------------------------------------------------------------------
void AxisWidget::SetEnabled(int value)
{
  if (!this->Interactor)
  {
    vtkErrorMacro("The interactor must be set prior to enabling/disabling widget");
  }

  if (value != this->Enabled)
  {
    if (value)
    {
      if (!this->OrientationMarker)
      {
        vtkErrorMacro("An orientation marker must be set prior to enabling/disabling widget");
        return;
      }

      if (!this->CurrentRenderer)
      {
        int *pos = this->Interactor->GetLastEventPosition();
        this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(pos[0], pos[1]));

        if (!this->CurrentRenderer)
        {
          return;
        }
      }

      this->UpdateInternalViewport();

      this->SetupWindowInteraction();
      this->Enabled = 1;
      this->InvokeEvent(vtkCommand::EnableEvent, nullptr);
    }
    else
    {
      this->InvokeEvent(vtkCommand::DisableEvent, nullptr);
      this->Enabled = 0;
      this->TearDownWindowInteraction();
      this->SetCurrentRenderer(nullptr);
    }
  }
}

//-------------------------------------------------------------------------
void AxisWidget::SetupWindowInteraction()
{
  vtkRenderWindow* renwin = this->CurrentRenderer->GetRenderWindow();
  renwin->AddRenderer(this->Renderer);
  if (renwin->GetNumberOfLayers() < 2)
  {
    renwin->SetNumberOfLayers(2);
  }

  this->CurrentRenderer->AddViewProp(this->OutlineActor);

  this->Renderer->AddViewProp(this->OrientationMarker);
  this->OrientationMarker->VisibilityOn();
  this->OrientationMarker->SetPickable(false);
  this->OrientationMarker->SetDragable(false);

  // We need to copy the camera before the compositing observer is called.
  // Compositing temporarily changes the camera to display an image.
  this->StartEventObserverId = this->CurrentRenderer->AddObserver(vtkCommand::StartEvent, this->Observer, 1);
}

//-------------------------------------------------------------------------
void AxisWidget::TearDownWindowInteraction()
{
  if (this->StartEventObserverId != 0)
  {
    this->CurrentRenderer->RemoveObserver(this->StartEventObserverId);
  }

  this->Interactor->RemoveObserver(this->EventCallbackCommand);

  this->OrientationMarker->VisibilityOff();
  this->Renderer->RemoveViewProp(this->OrientationMarker);

  this->CurrentRenderer->RemoveViewProp(this->OutlineActor);

  // if the render window is still around, remove our renderer from it
  vtkRenderWindow* renwin = this->CurrentRenderer->GetRenderWindow();
  if (renwin)
  {
    renwin->RemoveRenderer(this->Renderer);
  }
}

//-------------------------------------------------------------------------
void AxisWidget::ExecuteCameraUpdateEvent(vtkObject *vtkNotUsed(o),
  unsigned long vtkNotUsed(event),
  void *vtkNotUsed(calldata))
{
  if (!this->CurrentRenderer)
  {
    return;
  }

  vtkCamera *cam = this->CurrentRenderer->GetActiveCamera();
  double pos[3], fp[3], viewup[3];
  cam->GetPosition(pos);
  cam->GetFocalPoint(fp);
  cam->GetViewUp(viewup);

  cam = this->Renderer->GetActiveCamera();
  cam->SetPosition(pos);
  cam->SetFocalPoint(fp);
  cam->SetViewUp(viewup);
  this->Renderer->ResetCamera();

  this->UpdateOutline();
}

//-------------------------------------------------------------------------
int AxisWidget::ComputeStateBasedOnPosition(int X, int Y,
  int *pos1, int *pos2)
{
  return AxisWidget::Outside;
}

//-------------------------------------------------------------------------
void AxisWidget::SetCursor(int state)
{
  switch (state)
  {
  case AxisWidget::AdjustingP1:
    this->RequestCursorShape(VTK_CURSOR_SIZESW);
    break;
  case AxisWidget::AdjustingP3:
    this->RequestCursorShape(VTK_CURSOR_SIZENE);
    break;
  case AxisWidget::AdjustingP2:
    this->RequestCursorShape(VTK_CURSOR_SIZESE);
    break;
  case AxisWidget::AdjustingP4:
    this->RequestCursorShape(VTK_CURSOR_SIZENW);
    break;
  case AxisWidget::Translating:
    this->RequestCursorShape(VTK_CURSOR_SIZEALL);
    break;
  case AxisWidget::Inside:
    this->RequestCursorShape(VTK_CURSOR_SIZEALL);
    break;
  case AxisWidget::Outside:
    this->RequestCursorShape(VTK_CURSOR_DEFAULT);
    break;
  }
}

//-------------------------------------------------------------------------
void AxisWidget::ProcessEvents(vtkObject* vtkNotUsed(object),
  unsigned long event,
  void *clientdata,
  void* vtkNotUsed(calldata))
{
}

//-------------------------------------------------------------------------
void AxisWidget::SquareRenderer()
{
  int *size = this->Renderer->GetSize();
  if (size[0] == 0 || size[1] == 0)
  {
    return;
  }

  double vp[4];
  this->Renderer->GetViewport(vp);

  this->Renderer->NormalizedDisplayToDisplay(vp[0], vp[1]);
  this->Renderer->NormalizedDisplayToDisplay(vp[2], vp[3]);

  // get the minimum viewport edge size
  //
  double dx = vp[2] - vp[0];
  double dy = vp[3] - vp[1];

  if (dx != dy)
  {
    double delta = dx < dy ? dx : dy;

    switch (this->State)
    {
    case AxisWidget::AdjustingP1:
      vp[2] = vp[0] + delta;
      vp[3] = vp[1] + delta;
      break;
    case AxisWidget::AdjustingP2:
      vp[0] = vp[2] - delta;
      vp[3] = vp[1] + delta;
      break;
    case AxisWidget::AdjustingP3:
      vp[0] = vp[2] - delta;
      vp[1] = vp[3] - delta;
      break;
    case AxisWidget::AdjustingP4:
      vp[2] = vp[0] + delta;
      vp[1] = vp[3] - delta;
      break;
    case AxisWidget::Translating:
      delta = (dx + dy)*0.5;
      vp[0] = ((vp[0] + vp[2]) - delta)*0.5;
      vp[1] = ((vp[1] + vp[3]) - delta)*0.5;
      vp[2] = vp[0] + delta;
      vp[3] = vp[1] + delta;
      break;
    }
    this->Renderer->DisplayToNormalizedDisplay(vp[0], vp[1]);
    this->Renderer->DisplayToNormalizedDisplay(vp[2], vp[3]);
    this->Renderer->SetViewport(vp);
    this->UpdateViewport();
  }
}

//-------------------------------------------------------------------------
void AxisWidget::UpdateOutline()
{
  double vp[4];
  this->Renderer->GetViewport(vp);

  this->Renderer->NormalizedDisplayToDisplay(vp[0], vp[1]);
  this->Renderer->NormalizedDisplayToDisplay(vp[2], vp[3]);

  vtkPoints *points = this->Outline->GetPoints();

  points->SetPoint(0, vp[0] + 1, vp[1] + 1, 0);
  points->SetPoint(1, vp[2] - 1, vp[1] + 1, 0);
  points->SetPoint(2, vp[2] - 1, vp[3] - 1, 0);
  points->SetPoint(3, vp[0] + 1, vp[3] - 1, 0);
}

//-------------------------------------------------------------------------
void AxisWidget::SetInteractive(vtkTypeBool interact)
{
}

//-------------------------------------------------------------------------
void AxisWidget::ResizeTopLeft(int X, int Y)
{
  int dx = X - this->StartPosition[0];
  int dy = Y - this->StartPosition[1];
  int delta = (abs(dx) + abs(dy)) / 2;

  if (dx <= 0 && dy >= 0) // make bigger
  {
    dx = -delta;
    dy = delta;
  }
  else if (dx >= 0 && dy <= 0) // make smaller
  {
    dx = delta;
    dy = -delta;
  }
  else
  {
    return;
  }

  double currentViewport[4];
  this->CurrentRenderer->GetViewport(currentViewport);
  this->CurrentRenderer->NormalizedDisplayToDisplay(
    currentViewport[0], currentViewport[1]);
  this->CurrentRenderer->NormalizedDisplayToDisplay(
    currentViewport[2], currentViewport[3]);

  double vp[4];
  this->Renderer->GetViewport(vp);
  this->Renderer->NormalizedDisplayToDisplay(vp[0], vp[1]);
  this->Renderer->NormalizedDisplayToDisplay(vp[2], vp[3]);

  double newPos[4] = { vp[0] + dx, vp[1], vp[2], vp[3] + dy };

  if (newPos[0] < currentViewport[0])
  {
    newPos[0] = currentViewport[0];
  }
  if (newPos[0] > newPos[2] - this->Tolerance)  // keep from making it too small
  {
    newPos[0] = newPos[2] - this->Tolerance;
  }
  if (newPos[3] > currentViewport[3])
  {
    newPos[3] = currentViewport[3];
  }
  if (newPos[3] < newPos[1] + this->Tolerance)
  {
    newPos[3] = newPos[1] + this->Tolerance;
  }

  this->StartPosition[0] = static_cast<int>(newPos[0]);
  this->StartPosition[1] = static_cast<int>(newPos[3]);

  this->Renderer->DisplayToNormalizedDisplay(newPos[0], newPos[1]);
  this->Renderer->DisplayToNormalizedDisplay(newPos[2], newPos[3]);

  this->Renderer->SetViewport(newPos);
  this->UpdateViewport();
}

//-------------------------------------------------------------------------
void AxisWidget::ResizeTopRight(int X, int Y)
{
  int dx = X - this->StartPosition[0];
  int dy = Y - this->StartPosition[1];
  int delta = (abs(dx) + abs(dy)) / 2;

  if (dx >= 0 && dy >= 0) // make bigger
  {
    dx = delta;
    dy = delta;
  }
  else if (dx <= 0 && dy <= 0) // make smaller
  {
    dx = -delta;
    dy = -delta;
  }
  else
  {
    return;
  }

  double currentViewport[4];
  this->CurrentRenderer->GetViewport(currentViewport);
  this->CurrentRenderer->NormalizedDisplayToDisplay(
    currentViewport[0], currentViewport[1]);
  this->CurrentRenderer->NormalizedDisplayToDisplay(
    currentViewport[2], currentViewport[3]);

  double vp[4];
  this->Renderer->GetViewport(vp);
  this->Renderer->NormalizedDisplayToDisplay(vp[0], vp[1]);
  this->Renderer->NormalizedDisplayToDisplay(vp[2], vp[3]);

  double newPos[4] = { vp[0], vp[1], vp[2] + dx, vp[3] + dy };

  if (newPos[2] > currentViewport[2])
  {
    newPos[2] = currentViewport[2];
  }
  if (newPos[2] < newPos[0] + this->Tolerance)  // keep from making it too small
  {
    newPos[2] = newPos[0] + this->Tolerance;
  }
  if (newPos[3] > currentViewport[3])
  {
    newPos[3] = currentViewport[3];
  }
  if (newPos[3] < newPos[1] + this->Tolerance)
  {
    newPos[3] = newPos[1] + this->Tolerance;
  }

  this->StartPosition[0] = static_cast<int>(newPos[2]);
  this->StartPosition[1] = static_cast<int>(newPos[3]);

  this->Renderer->DisplayToNormalizedDisplay(newPos[0], newPos[1]);
  this->Renderer->DisplayToNormalizedDisplay(newPos[2], newPos[3]);

  this->Renderer->SetViewport(newPos);
  this->UpdateViewport();
}

//-------------------------------------------------------------------------
void AxisWidget::ResizeBottomRight(int X, int Y)
{
  int dx = X - this->StartPosition[0];
  int dy = Y - this->StartPosition[1];
  int delta = (abs(dx) + abs(dy)) / 2;

  if (dx >= 0 && dy <= 0) // make bigger
  {
    dx = delta;
    dy = -delta;
  }
  else if (dx <= 0 && dy >= 0) // make smaller
  {
    dx = -delta;
    dy = delta;
  }
  else
  {
    return;
  }

  double currentViewport[4];
  this->CurrentRenderer->GetViewport(currentViewport);
  this->CurrentRenderer->NormalizedDisplayToDisplay(
    currentViewport[0], currentViewport[1]);
  this->CurrentRenderer->NormalizedDisplayToDisplay(
    currentViewport[2], currentViewport[3]);

  double vp[4];
  this->Renderer->GetViewport(vp);
  this->Renderer->NormalizedDisplayToDisplay(vp[0], vp[1]);
  this->Renderer->NormalizedDisplayToDisplay(vp[2], vp[3]);

  double newPos[4] = { vp[0], vp[1] + dy, vp[2] + dx, vp[3] };

  if (newPos[2] > currentViewport[2])
  {
    newPos[2] = currentViewport[2];
  }
  if (newPos[2] < newPos[0] + this->Tolerance)  // keep from making it too small
  {
    newPos[2] = newPos[0] + this->Tolerance;
  }
  if (newPos[1] < currentViewport[1])
  {
    newPos[1] = currentViewport[1];
  }
  if (newPos[1] > newPos[3] - this->Tolerance)
  {
    newPos[1] = newPos[3] - this->Tolerance;
  }

  this->StartPosition[0] = static_cast<int>(newPos[2]);
  this->StartPosition[1] = static_cast<int>(newPos[1]);

  this->Renderer->DisplayToNormalizedDisplay(newPos[0], newPos[1]);
  this->Renderer->DisplayToNormalizedDisplay(newPos[2], newPos[3]);

  this->Renderer->SetViewport(newPos);
  this->UpdateViewport();
}

//-------------------------------------------------------------------------
void AxisWidget::ResizeBottomLeft(int X, int Y)
{
  int dx = X - this->StartPosition[0];
  int dy = Y - this->StartPosition[1];
  int delta = (abs(dx) + abs(dy)) / 2;

  if (dx <= 0 && dy <= 0) // make bigger
  {
    dx = -delta;
    dy = -delta;
  }
  else if (dx >= 0 && dy >= 0) // make smaller
  {
    dx = delta;
    dy = delta;
  }
  else
  {
    return;
  }

  double currentViewport[4];
  this->CurrentRenderer->GetViewport(currentViewport);
  this->CurrentRenderer->NormalizedDisplayToDisplay(
    currentViewport[0], currentViewport[1]);
  this->CurrentRenderer->NormalizedDisplayToDisplay(
    currentViewport[2], currentViewport[3]);

  double vp[4];
  this->Renderer->GetViewport(vp);
  this->Renderer->NormalizedDisplayToDisplay(vp[0], vp[1]);
  this->Renderer->NormalizedDisplayToDisplay(vp[2], vp[3]);

  double newPos[4] = { vp[0] + dx, vp[1] + dy, vp[2], vp[3] };

  if (newPos[0] < currentViewport[0])
  {
    newPos[0] = currentViewport[0];
  }
  if (newPos[0] > newPos[2] - this->Tolerance)  // keep from making it too small
  {
    newPos[0] = newPos[2] - this->Tolerance;
  }
  if (newPos[1] < currentViewport[1])
  {
    newPos[1] = currentViewport[1];
  }
  if (newPos[1] > newPos[3] - this->Tolerance)
  {
    newPos[1] = newPos[3] - this->Tolerance;
  }

  this->StartPosition[0] = static_cast<int>(newPos[0]);
  this->StartPosition[1] = static_cast<int>(newPos[1]);

  this->Renderer->DisplayToNormalizedDisplay(newPos[0], newPos[1]);
  this->Renderer->DisplayToNormalizedDisplay(newPos[2], newPos[3]);

  this->Renderer->SetViewport(newPos);
  this->UpdateViewport();
}

//-------------------------------------------------------------------------
void AxisWidget::SetOutlineColor(double r, double g, double b)
{
  this->OutlineActor->GetProperty()->SetColor(r, g, b);
  if (this->Interactor)
  {
    this->Interactor->Render();
  }
}

//-------------------------------------------------------------------------
double* AxisWidget::GetOutlineColor()
{
  return this->OutlineActor->GetProperty()->GetColor();
}

//-------------------------------------------------------------------------
void AxisWidget::UpdateViewport()
{
  if (!this->CurrentRenderer)
  {
    return;
  }
  double currentViewport[4];
  this->CurrentRenderer->GetViewport(currentViewport);

  double vp[4];
  this->Renderer->GetViewport(vp);

  double cvpRange[2];
  for (int i = 0; i < 2; ++i)
  {
    cvpRange[i] = currentViewport[i + 2] - currentViewport[i];
    this->Viewport[i] = (vp[i] - currentViewport[i]) / cvpRange[i];
    this->Viewport[i + 2] = (vp[i + 2] - currentViewport[i]) / cvpRange[i];
  }
}

//-------------------------------------------------------------------------
void AxisWidget::UpdateInternalViewport()
{
  if (!this->Renderer || !this->GetCurrentRenderer())
  {
    return;
  }

  // Compute the viewport for the widget w.r.t. to the current renderer
  double currentViewport[4];
  this->CurrentRenderer->GetViewport(currentViewport);
  double vp[4], currentViewportRange[2];
  for (int i = 0; i < 2; ++i)
  {
    currentViewportRange[i] = currentViewport[i + 2] - currentViewport[i];
    vp[i] = this->Viewport[i] * currentViewportRange[i] +
      currentViewport[i];
    vp[i + 2] = this->Viewport[i + 2] * currentViewportRange[i] +
      currentViewport[i];
  }
  this->Renderer->SetViewport(vp);
}

//-------------------------------------------------------------------------
void AxisWidget::Modified()
{
  //this->UpdateInternalViewport();
  //this->vtkInteractorObserver::Modified();
}

//-------------------------------------------------------------------------
void AxisWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "OrientationMarker: " << this->OrientationMarker << endl;
  os << indent << "Interactive: " << this->Interactive << endl;
  os << indent << "Tolerance: " << this->Tolerance << endl;
  os << indent << "Viewport: (" << this->Viewport[0] << ", "
    << this->Viewport[1] << ", " << this->Viewport[2] << ", "
    << this->Viewport[3] << ")\n";
}

