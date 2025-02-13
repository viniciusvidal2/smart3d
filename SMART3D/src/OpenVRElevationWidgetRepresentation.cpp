#include "OpenVRElevationWidgetRepresentation.h"

#include <vtkObjectFactory.h>
#include <vtkTextActor3D.h>
#include <vtkTextProperty.h>
#include <vtkPolyData.h>
#include <vtkElevationFilter.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkLineSource.h>
#include <vtkPointData.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkTransform.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

#include "Calibration.h"
#include "Mesh.h"

vtkStandardNewMacro(OpenVRElevationWidgetRepresentation);

//----------------------------------------------------------------------------
OpenVRElevationWidgetRepresentation::OpenVRElevationWidgetRepresentation()
{
  this->InteractionState = OpenVRElevationWidgetRepresentation::Outside;

  // The text
  textActor = vtkSmartPointer<vtkTextActor3D>::New();

  vtkSmartPointer<vtkTextProperty> prop = this->textActor->GetTextProperty();
  textActor->ForceOpaqueOn();

  prop->SetFontFamilyToTimes();
  prop->SetFrame(1);
  prop->SetFrameWidth(12);
  prop->SetFrameColor(0.0, 0.0, 0.0);
  prop->SetBackgroundOpacity(1.0);
  prop->SetBackgroundColor(0.0, 0.0, 0.0);
  prop->SetFontSize(20);
}

//----------------------------------------------------------------------------
OpenVRElevationWidgetRepresentation::~OpenVRElevationWidgetRepresentation()
{
  elevationMapActor = NULL;
}

void OpenVRElevationWidgetRepresentation::setMesh(Mesh * mesh)
{
  if (mesh != NULL)
  {
    this->mesh = mesh;
    vtkSmartPointer<vtkPolyData> polyMesh = mesh->getPolyData();
    bounds = polyMesh->GetBounds();
    elevationFilter = vtkSmartPointer<vtkElevationFilter>::New();
    elevationFilter->SetInputData(polyMesh);
    //To avoid the bad vector problem with the elevation filter
    if (bounds[5] < 0)
    {
      elevationFilter->SetLowPoint(0.0, 0.0, bounds[5] * 0.99999);
    }
    else
    {
      elevationFilter->SetLowPoint(0.0, 0.0, bounds[5] * 1.00001);
    }
    elevationFilter->SetHighPoint(0.0, 0.0, bounds[4]);
    elevationFilter->Update();
    outputElevationFilter = vtkSmartPointer<vtkPolyData>::New();
    outputElevationFilter->DeepCopy(vtkPolyData::SafeDownCast(elevationFilter->GetOutput()));
    updateElevationMap();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->ScalarVisibilityOn();
    mapper->SetScalarModeToUsePointData();
    mapper->SetInputData(outputElevationFilter);
    elevationMapActor = vtkSmartPointer<vtkActor>::New();
    elevationMapActor->SetMapper(mapper);

    sliderIncrement = abs(bounds[5] - bounds[4]) / 10.0;
  }
}

void OpenVRElevationWidgetRepresentation::pointPicked(double* point)
{
  if (defineZeroByPoint)
  {
    lastIncrement = point[2] - bounds[4];
    updateElevationMap();
    this->Modified();
    defineZeroByPoint = false;
    return;
  }
  lastPointZ = point[2];
  double distance = lastPointZ - (lastIncrement + bounds[4]);
  textActor->SetInput(mesh->getCalibration()->getCalibratedText(distance).c_str());

  showBilboard(0, 0, 0, 50);

  if (lineSource == NULL)
  {
    lineSource = vtkSmartPointer<vtkLineSource>::New();
    lineSource->SetPoint1(0, 0, 0);
    lineSource->SetPoint2(0, 0, -1);
    lineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    lineActor = vtkSmartPointer<vtkActor>::New();
    lineMapper->SetInputConnection(lineSource->GetOutputPort());
    lineActor->SetMapper(lineMapper);
    lineActor->SetVisibility(false);
  }
  lineSource->SetPoint1(point);
  lineSource->SetPoint2(textActor->GetPosition());
  lineActor->SetVisibility(true);
  lineMapper->Modified();
}

void OpenVRElevationWidgetRepresentation::setDefineZeroByPoint(bool setZero)
{
  defineZeroByPoint = setZero;
}

bool OpenVRElevationWidgetRepresentation::getDefineZeroByPoint()
{
  return defineZeroByPoint;
}

void OpenVRElevationWidgetRepresentation::updateElevationMap()
{
  if (lastIncrement < 0)
  {
    lastIncrement = 0;
  }
  if (lastIncrement + bounds[4] > bounds[5])
  {
    lastIncrement = bounds[5] - bounds[4];
  }
  elevationFilter->SetHighPoint(0.0, 0.0, lastIncrement + bounds[4]);
  elevationFilter->Update();
  outputElevationFilter->GetPointData()->SetScalars(elevationFilter->GetOutput()->GetPointData()->GetArray("Elevation"));
  updateText();
}

void OpenVRElevationWidgetRepresentation::updateText()
{
  if (lastPointZ != -1.0)
  {
    double distance = lastPointZ - (lastIncrement + bounds[4]);
    textActor->SetInput(mesh->getCalibration()->getCalibratedText(distance).c_str());
  }
}

void OpenVRElevationWidgetRepresentation::showBilboard(double r, double g, double b, int size)
{
  vtkOpenVRRenderWindow* renWin = vtkOpenVRRenderWindow::SafeDownCast(this->Renderer->GetRenderWindow());
  vtkRenderer *ren = this->Renderer;
  if (!renWin || !ren)
  {
    return;
  }

  renWin->UpdateHMDMatrixPose();
  double dop[3];
  ren->GetActiveCamera()->GetDirectionOfProjection(dop);
  double vr[3];
  double *vup = renWin->GetPhysicalViewUp();
  double dtmp[3];
  double vupdot = vtkMath::Dot(dop, vup);
  if (fabs(vupdot) < 0.999)
  {
    dtmp[0] = dop[0] - vup[0] * vupdot;
    dtmp[1] = dop[1] - vup[1] * vupdot;
    dtmp[2] = dop[2] - vup[2] * vupdot;
    vtkMath::Normalize(dtmp);
  }
  else
  {
    renWin->GetPhysicalViewDirection(dtmp);
  }
  vtkMath::Cross(dtmp, vup, vr);
  vtkNew<vtkMatrix4x4> rot;
  for (int i = 0; i < 3; ++i)
  {
    rot->SetElement(0, i, vr[i]);
    rot->SetElement(1, i, vup[i]);
    rot->SetElement(2, i, -dtmp[i]);
  }
  rot->Transpose();
  double orient[3];
  vtkTransform::GetOrientation(orient, rot);
  vtkTextProperty *prop = this->textActor->GetTextProperty();
  this->textActor->SetOrientation(orient);
  this->textActor->RotateX(-30.0);

  double tpos[3];
  double scale = renWin->GetPhysicalScale();
  ren->GetActiveCamera()->GetPosition(tpos);
  tpos[0] += (0.7*scale*dop[0] - 0.1*scale*vr[0] - 0.4*scale*vup[0]);
  tpos[1] += (0.7*scale*dop[1] - 0.1*scale*vr[1] - 0.4*scale*vup[1]);
  tpos[2] += (0.7*scale*dop[2] - 0.1*scale*vr[2] - 0.4*scale*vup[2]);
  this->textActor->SetPosition(tpos);
  // scale should cover 10% of FOV
  double fov = ren->GetActiveCamera()->GetViewAngle();
  double tsize = 0.1*2.0*atan(fov*0.5); // 10% of fov
  tsize /= 200.0;  // about 200 pixel texture map
  scale *= tsize;
  this->textActor->SetScale(scale, scale, scale);

  prop->SetFrame(1);
  prop->SetFrameColor(1.0, 1.0, 1.0);
  prop->SetBackgroundOpacity(1.0);
  prop->SetBackgroundColor(r, g, b);
  prop->SetFontSize(size);
}

int OpenVRElevationWidgetRepresentation::ComputeComplexInteractionState(
  vtkRenderWindowInteractor *,
  vtkAbstractWidget *,
  unsigned long, void *calldata, int)
{
  this->InteractionState = OpenVRElevationWidgetRepresentation::Deleting;
  return this->InteractionState;
}

void OpenVRElevationWidgetRepresentation::StartComplexInteraction(
  vtkRenderWindowInteractor *,
  vtkAbstractWidget *,
  unsigned long, void *calldata)
{

}

void OpenVRElevationWidgetRepresentation::ComplexInteraction(
  vtkRenderWindowInteractor *,
  vtkAbstractWidget *,
  unsigned long, void *calldata)
{
  vtkEventData *edata = static_cast<vtkEventData *>(calldata);
  vtkEventDataDevice3D *edd = edata->GetAsEventDataDevice3D();
  if (edd)
  {
    double trackpadPos[2];
    edd->GetTrackPadPosition(trackpadPos);
    lastIncrement += trackpadPos[1]*sliderIncrement;
    updateElevationMap();
    this->Modified();
  }
}

void OpenVRElevationWidgetRepresentation::EndComplexInteraction(
  vtkRenderWindowInteractor *,
  vtkAbstractWidget *,
  unsigned long, void *)
{
  this->InteractionState = OpenVRElevationWidgetRepresentation::Outside;
}


void OpenVRElevationWidgetRepresentation::GetActors(vtkPropCollection * p)
{
  if (this->elevationMapActor != NULL)
  {
    elevationMapActor->GetActors(p);
  }
}

//----------------------------------------------------------------------------
void OpenVRElevationWidgetRepresentation::ReleaseGraphicsResources(vtkWindow *w)
{
  if (this->elevationMapActor != NULL)
  {
    this->elevationMapActor->ReleaseGraphicsResources(w);
  }
  if (this->textActor != NULL)
  {
    this->textActor->ReleaseGraphicsResources(w);
  }
  if (this->lineActor != NULL)
  {
    this->lineActor->ReleaseGraphicsResources(w);
  }
}

//----------------------------------------------------------------------------
int OpenVRElevationWidgetRepresentation::RenderOpaqueGeometry(vtkViewport *v)
{
  if (!this->GetVisibility())
  {
    return 0;
  }
  int count = 0;
  if (this->elevationMapActor != NULL)
  {
    count += this->elevationMapActor->RenderOpaqueGeometry(v);
  }
  if (this->textActor != NULL)
  {
    count += this->textActor->RenderOpaqueGeometry(v);
  }
  if (this->lineActor != NULL)
  {
    count += this->lineActor->RenderOpaqueGeometry(v);
  }
  return count;
}

//-----------------------------------------------------------------------------
int OpenVRElevationWidgetRepresentation::RenderTranslucentPolygonalGeometry(
  vtkViewport *v)
{
  if (!this->GetVisibility())
  {
    return 0;
  }
  int count = 0;
  if (this->elevationMapActor != NULL)
  {
    count += this->elevationMapActor->RenderTranslucentPolygonalGeometry(v);
  }
  if (this->textActor != NULL)
  {
    count += this->textActor->RenderTranslucentPolygonalGeometry(v);
  }
  if (this->lineActor != NULL)
  {
    count += this->lineActor->RenderTranslucentPolygonalGeometry(v);
  }
  return count;
}

//-----------------------------------------------------------------------------
int OpenVRElevationWidgetRepresentation::HasTranslucentPolygonalGeometry()
{
  if (!this->GetVisibility())
  {
    return 0;
  }

  int result = 0;
  if (this->elevationMapActor != NULL)
  {
    result |= this->elevationMapActor->HasTranslucentPolygonalGeometry();
  }
  if (this->textActor != NULL)
  {
    result |= this->textActor->HasTranslucentPolygonalGeometry();
  }
  if (this->lineActor != NULL)
  {
    result |= this->lineActor->HasTranslucentPolygonalGeometry();
  }
  return result;
}

//----------------------------------------------------------------------------
void OpenVRElevationWidgetRepresentation::PlaceWidget(double bds[6])
{
}

//----------------------------------------------------------------------------
void OpenVRElevationWidgetRepresentation::BuildRepresentation()
{
}

//----------------------------------------------------------------------------
void OpenVRElevationWidgetRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
