#pragma once
#include <vtkRenderingOpenVRModule.h> // For export macro
#include <vtkAbstractWidget.h>

class OpenVRDeletePointsWidgetRepresentation;

class OpenVRDeletePointsWidget : public vtkAbstractWidget
{
public:
  /**
  * Instantiate the object.
  */
  static OpenVRDeletePointsWidget *New();

  //@{
  /**
  * Standard vtkObject methods
  */
  vtkTypeMacro(OpenVRDeletePointsWidget, vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  /**
  * Specify an instance of vtkWidgetRepresentation used to represent this
  * widget in the scene. Note that the representation is a subclass of vtkProp
  * so it can be added to the renderer independent of the widget.
  */
  void SetRepresentation(OpenVRDeletePointsWidgetRepresentation *rep);

  /**
  * Create the default widget representation if one is not set.
  */
  void CreateDefaultRepresentation() override;

  void SetEnabled(int enabling) override;

protected:
  OpenVRDeletePointsWidget();
  ~OpenVRDeletePointsWidget() override;

  // Manage the state of the widget
  int WidgetState;
  enum _WidgetState { Start = 0, Active };

  /**
  * callback
  */
  static void SelectAction3D(vtkAbstractWidget*);
  static void EndSelectAction3D(vtkAbstractWidget*);
  static void MoveAction3D(vtkAbstractWidget*);
  static void deletePointsAction3D(vtkAbstractWidget*);

private:
  OpenVRDeletePointsWidget(const OpenVRDeletePointsWidget&) = delete;
  void operator=(const OpenVRDeletePointsWidget&) = delete;
};
