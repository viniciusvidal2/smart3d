#pragma once
#include <vtkInteractionWidgetsModule.h> // For export macro
#include <vtk3DWidget.h>
#include <vtkSmartPointer.h>

class LineWidget;
class Mesh;

class AlignWithAxisTool : public vtk3DWidget
{
public:
	/**
	* Instantiate the object.
	*/
	static AlignWithAxisTool *New();

	vtkTypeMacro(AlignWithAxisTool, vtk3DWidget);

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
	AlignWithAxisTool();
	~AlignWithAxisTool();

	//handles the events
	static void ProcessEvents(vtkObject* object, unsigned long event,
		void* clientdata, void* calldata);

	vtkSmartPointer<LineWidget> lineWidget = nullptr;

	// Controlling ivars
	void UpdateRepresentation();

	Mesh* mesh = nullptr;

private:
	AlignWithAxisTool(const AlignWithAxisTool&) VTK_DELETE_FUNCTION;
	void operator=(const AlignWithAxisTool&) VTK_DELETE_FUNCTION;
};