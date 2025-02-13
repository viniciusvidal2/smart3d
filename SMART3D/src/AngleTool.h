#pragma once

#include "vtkInteractionWidgetsModule.h" // For export macro
#include "vtk3DWidget.h"
#include <vtkSmartPointer.h>

class vtkCaptionActor2D;
class LineWidget;
class Mesh;

class AngleTool : public vtk3DWidget
{
public:
  /**
  * Instantiate the object.
  */
  static AngleTool *New();

  vtkTypeMacro(AngleTool, vtk3DWidget);

  //@{
  /**
  * Methods that satisfy the superclass' API.
  */
  virtual void SetEnabled(int);
  virtual void PlaceWidget(double bounds[6]) {};
  //@}

  //@{
  /**
  * Set the mesh that is going to be used
  */
  void setMesh(Mesh* mesh) 
  {
	  if (mesh)
	  {
		  this->mesh = mesh;
	  }
  };
  //@}

protected:
  AngleTool();
  ~AngleTool();

  //handles the events
  static void ProcessEvents(vtkObject* object, unsigned long event,
    void* clientdata, void* calldata);

  Mesh* mesh = nullptr;

  vtkSmartPointer<LineWidget> lineWidget = nullptr;

  //Actors
  vtkSmartPointer<vtkCaptionActor2D> textActor = nullptr;

  // Controlling ivars
  void UpdateRepresentation();

  /**
  * Get the angle(in degrees) between point1, point2 and point3.
  * point1
  * |
  * |
  * point2 - - - - point3
  */
  double getAngle(double* point1, double* point2, double* point3);

private:
  AngleTool(const AngleTool&) VTK_DELETE_FUNCTION;
  void operator=(const AngleTool&) VTK_DELETE_FUNCTION;
};