#pragma once
#include <vtkAbstractWidget.h>
#include "LineWidgetRepresentation.h"

class Mesh;

class LineWidget : public vtkAbstractWidget
{
public:
  /**
  * Method to instantiate class.
  */
  static LineWidget *New();

  //@{
  /**
  * Standard methods for class.
  */
  vtkTypeMacro(LineWidget, vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  /**
  * Specify an instance of vtkWidgetRepresentation used to represent this
  * widget in the scene. Note that the representation is a subclass of vtkProp
  * so it can be added to the renderer independent of the widget.
  */
  void SetRepresentation(LineWidgetRepresentation *r)
  {
    this->Superclass::SetWidgetRepresentation(reinterpret_cast<LineWidgetRepresentation*>(r));
  }

  /**
  * Return the representation as a vtkBorderRepresentation.
  */
  LineWidgetRepresentation *GetRepresentation()
  {
    return reinterpret_cast<LineWidgetRepresentation*>(this->WidgetRep);
  }

  /**
  * Create the default widget representation if one is not set.
  */
  void CreateDefaultRepresentation() override;

  void CreateRepresentationFromMesh(Mesh* mesh);

  void SetEnabled(int enabling) override;

  void setMaxNumberOfNodes(int maxnumberOfNodes);

  bool hasFinished();

  void setCloseLoopOnFirstNode(bool closeLoop);

  //widget state
  enum _WidgetState { Start = 0, Define, Manipulate, Finished };

  int getWidgetState();

protected:
  LineWidget();
  ~LineWidget() override;

  /**
  * Subclasses generally implement this method. The SelectRegion() method
  * offers a subclass the chance to do something special if the interior
  * of the widget is selected.
  */
  virtual void SelectRegion(double eventPos[2]);

  //processes the registered events
  static void SelectAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);
  static void MoveAction(vtkAbstractWidget*);
  static void CtrlKeyPress(vtkAbstractWidget*);

  //widget state
  int WidgetState;

  int maxNumberOfNodes = VTK_INT_MAX;
  bool closeLoopOnFirstNode = false;

private:
  LineWidget(const LineWidget&) = delete;
  void operator=(const LineWidget&) = delete;
};