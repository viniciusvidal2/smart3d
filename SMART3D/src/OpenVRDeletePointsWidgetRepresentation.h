#pragma once
#include <vtkRenderingOpenVRModule.h> // For export macro
#include <vtkWidgetRepresentation.h>
#include <vtkSmartPointer.h>

class vtkActor;
class vtkSelection;
class vtkPointLocator;
class vtkPoints;
class vtkSelectionNode;
class vtkIdList;
class Mesh;

class OpenVRDeletePointsWidgetRepresentation : public vtkWidgetRepresentation
{
public:
  /**
  * Instantiate the class.
  */
  static OpenVRDeletePointsWidgetRepresentation *New();

  //@{
  /**
  * Standard methods for the class.
  */
  vtkTypeMacro(OpenVRDeletePointsWidgetRepresentation, vtkWidgetRepresentation);
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
  void ReleaseGraphicsResources(vtkWindow*) override;
  int RenderOpaqueGeometry(vtkViewport*) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport*) override;
  int HasTranslucentPolygonalGeometry() override;
  //@}

  void setMesh(Mesh* mesh);
  void disable();
  void deletePoints();
  void setScale(double scale);

protected:
  OpenVRDeletePointsWidgetRepresentation();
  ~OpenVRDeletePointsWidgetRepresentation() override;

  //Actors
  vtkSmartPointer<vtkActor> actorSphere = NULL;
  vtkSmartPointer<vtkActor> actorPointsToDelete = NULL;
  vtkSmartPointer<vtkSelection> selection = NULL;
  void computePointsToDelete();

  vtkSmartPointer<vtkPointLocator> pointLocator = NULL;
  vtkSmartPointer<vtkPoints> pointsMesh = NULL;
  Mesh* mesh = NULL;
  double searchRadius = 0.2;

  vtkSmartPointer<vtkSelectionNode> createSelectionNode(vtkSmartPointer<vtkIdList> idList);
  void createActorPointsToDelete();

private:
  OpenVRDeletePointsWidgetRepresentation(const OpenVRDeletePointsWidgetRepresentation&) = delete;
  void operator=(const OpenVRDeletePointsWidgetRepresentation&) = delete;
};

