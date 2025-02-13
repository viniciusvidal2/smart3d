#pragma once
#include <vtkRenderingOpenVRModule.h> // For export macro
#include <vtkAbstractWidget.h>

class OpenVRElevationWidgetRepresentation;
class Mesh;

class OpenVRElevationWidget : public vtkAbstractWidget
{
public:
  /**
  * Instantiate the object.
  */
  static OpenVRElevationWidget *New();

  //@{
  /**
  * Standard vtkObject methods
  */
  vtkTypeMacro(OpenVRElevationWidget, vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  /**
  * Specify an instance of vtkWidgetRepresentation used to represent this
  * widget in the scene. Note that the representation is a subclass of vtkProp
  * so it can be added to the renderer independent of the widget.
  */
  void SetRepresentation(OpenVRElevationWidgetRepresentation *rep);

  /**
  * Create the default widget representation if one is not set.
  */
  void CreateDefaultRepresentation() override;

  void SetEnabled(int enabling) override;

  void setMesh(Mesh* mesh);

protected:
  OpenVRElevationWidget();
  ~OpenVRElevationWidget() override;

  // Manage the state of the widget
  int WidgetState;
  enum _WidgetState { Start = 0, Active };

  /**
  * callback
  */
  static void SelectAction3D(vtkAbstractWidget*);
  static void EndSelectAction3D(vtkAbstractWidget*);
  static void TrackpadAction3D(vtkAbstractWidget*);
  static void GripAction3D(vtkAbstractWidget*);

  Mesh* mesh = NULL;

private:
  OpenVRElevationWidget(const OpenVRElevationWidget&) = delete;
  void operator=(const OpenVRElevationWidget&) = delete;
};
