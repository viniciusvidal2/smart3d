#pragma once
#include <vtkInteractionWidgetsModule.h> // For export macro
#include <vtk3DWidget.h>
#include <vtkSmartPointer.h>

class vtkCaptionActor2D;
class LineWidget;
class Mesh;

class DistanceTool : public vtk3DWidget
{
public:
	/**
	* Instantiate the object.
	*/
	static DistanceTool *New();

	vtkTypeMacro(DistanceTool, vtk3DWidget);

	//@{
	/**
	* Methods that satisfy the superclass' API.
	*/
	virtual void SetEnabled(int);
	virtual void PlaceWidget(double bounds[6]) {};
	//@}

	//Set the mesh that is going to be used
	void setMesh(Mesh* mesh)
	{
		if (mesh)
		{
			this->mesh = mesh;
		}
	};


	//True if the measure is finished.
	bool hasFinished() const;

	void updateCalibration();

protected:
	DistanceTool();
	~DistanceTool();

	//handles the events
	static void ProcessEvents(vtkObject* object, unsigned long event,
		void* clientdata, void* calldata);

	vtkSmartPointer<LineWidget> lineWidget = nullptr;

	//Actors
	vtkSmartPointer<vtkCaptionActor2D> textActor = nullptr;

	// Controlling ivars
	void UpdateRepresentation();

	Mesh* mesh = nullptr;

	double distance = -1;

private:
	DistanceTool(const DistanceTool&) VTK_DELETE_FUNCTION;
	void operator=(const DistanceTool&) VTK_DELETE_FUNCTION;
};
