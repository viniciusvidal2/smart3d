#pragma once
#include <vtkRenderingOpenVRModule.h> // For export macro
#include <vtkWidgetRepresentation.h>
#include <vtkSmartPointer.h>

class vtkPolyData;
class vtkTextActor3D;
class vtkLineSource;
class vtkElevationFilter;
class vtkPolyDataMapper;
class vtkActor;
class Mesh;

class OpenVRElevationWidgetRepresentation : public vtkWidgetRepresentation
{
public:
  /**
  * Instantiate the class.
  */
  static OpenVRElevationWidgetRepresentation *New();

  //@{
  /**
  * Standard methods for the class.
  */
  vtkTypeMacro(OpenVRElevationWidgetRepresentation, vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  // Enums define the state of the representation relative to the mouse pointer
  // position. Used by ComputeInteractionState() to communicate with the
  // widget. Note that ComputeInteractionState() and several other methods
  // must be implemented by subclasses.
  enum _InteractionState { Outside = 0, Deleting };

  //@{
  /**
  * Methods to interface with the vtkOpenVRPanelWidget.
  */
  void BuildRepresentation() override;
  void PlaceWidget(double bounds[6]) override;
  void StartComplexInteraction(
    vtkRenderWindowInteractor *iren,
    vtkAbstractWidget *widget,
    unsigned long event, void *calldata) override;
  void ComplexInteraction(
    vtkRenderWindowInteractor *iren,
    vtkAbstractWidget *widget,
    unsigned long event, void *calldata) override;
  int ComputeComplexInteractionState(
    vtkRenderWindowInteractor *iren,
    vtkAbstractWidget *widget,
    unsigned long event, void *calldata, int modify = 0) override;
  void EndComplexInteraction(
    vtkRenderWindowInteractor *iren,
    vtkAbstractWidget *widget,
    unsigned long event, void *calldata) override;
  //@}

  //@{
  /**
  * Methods supporting the rendering process.
  */
  void GetActors(vtkPropCollection* p);
  void ReleaseGraphicsResources(vtkWindow*) override;
  int RenderOpaqueGeometry(vtkViewport*) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport*) override;
  int HasTranslucentPolygonalGeometry() override;
  //@}

  void setMesh(Mesh* mesh);

  void pointPicked(double* point);

  void setDefineZeroByPoint(bool setZero);

  bool getDefineZeroByPoint();

  vtkSmartPointer<vtkActor> elevationMapActor = NULL;

protected:
  OpenVRElevationWidgetRepresentation();
  ~OpenVRElevationWidgetRepresentation() override;

  //ElevationMap
  vtkSmartPointer<vtkPolyData> outputElevationFilter = NULL;
  vtkSmartPointer<vtkElevationFilter> elevationFilter = NULL;

  //Numeric indicator
  vtkSmartPointer<vtkTextActor3D>  textActor = NULL;
  vtkSmartPointer<vtkLineSource> lineSource = NULL;
  vtkSmartPointer<vtkPolyDataMapper> lineMapper = NULL;
  vtkSmartPointer<vtkActor> lineActor = NULL;

  double* bounds = NULL;
  double sliderIncrement = 1.0;
  double lastIncrement = 0.0;
  Mesh* mesh = NULL;
  double lastPointZ = -1.0;
  bool defineZeroByPoint = false;

  void updateElevationMap();
  void updateText();
  void showBilboard(double r, double g, double b, int size);

private:
  OpenVRElevationWidgetRepresentation(const OpenVRElevationWidgetRepresentation&) = delete;
  void operator=(const OpenVRElevationWidgetRepresentation&) = delete;
};

