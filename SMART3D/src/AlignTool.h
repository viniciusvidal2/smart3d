#pragma once
#include <vtkInteractionWidgetsModule.h> // For export macro
#include <vtk3DWidget.h>
#include <vtkSmartPointer.h>

class Mesh;
class LineWidget;
class vtkPoints;

class AlignTool : public vtk3DWidget
{
public:
	/**
	* Instantiate the object.
	*/
	static AlignTool *New();

	vtkTypeMacro(AlignTool, vtk3DWidget);

	//@{
	/**
	* Methods that satisfy the superclass' API.
	*/
	virtual void SetEnabled(int);
	virtual void PlaceWidget(double bounds[6]) {};
	//@}

	//@{
	/**
	* Set the mesh that is going to be used as source
	*/
	void setMeshSource(Mesh* mesh)
	{
		if (!meshSource)
		{
			meshSource = mesh;
		}
	};
	//@}

	//@{
	/**
	* Set the mesh that is going to be used as target
	*/
	void setMeshTarget(Mesh* mesh) {
		if (!meshTarget)
		{
			meshTarget = mesh;
		}
	};
	//@}

	void enterKeyPressed();

protected:
	AlignTool();
	~AlignTool();

	// set wich mesh will be used to do the picking
	void setLineWidgetMesh(Mesh* mesh);

	vtkSmartPointer<LineWidget> lineWidget = nullptr;

	Mesh* meshSource = nullptr;
	Mesh* meshTarget = nullptr;

	vtkSmartPointer<vtkPoints> pointsSource = nullptr;
	vtkSmartPointer<vtkPoints> pointsTarget = nullptr;

private:
	AlignTool(const AlignTool&) VTK_DELETE_FUNCTION;
	void operator=(const AlignTool&) VTK_DELETE_FUNCTION;
};
