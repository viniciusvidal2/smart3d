#pragma once
#include <vtkInteractionWidgetsModule.h> // For export macro
#include <vtk3DWidget.h>
#include <vtkSmartPointer.h>

class vtkCellPicker;
class ImplicitPlaneWidget;
class vtkActor;
class vtkPoints;
class LineWidget;
class wxTreeListCtrl;
class Mesh;

class VolumeTool : public vtk3DWidget
{
public:
	/**
	* Instantiate the object.
	*/
	static VolumeTool *New();

	vtkTypeMacro(VolumeTool, vtk3DWidget);

	//@{
	/**
	* Methods that satisfy the superclass' API.
	*/
	virtual void SetEnabled(int);
	virtual void PlaceWidget(double bounds[6]) {};
	//@}


	void setMesh(Mesh* mesh)
	{
		if (mesh)
		{
			this->mesh = mesh;
		}
	};
	void setTree(wxTreeListCtrl* tree)
	{
		if (tree)
		{
			treeMesh = tree;
		}
	};

	void enterKeyPressed();

	bool isVolumeComputed() const { return volumeComputed; };

protected:
	VolumeTool();
	~VolumeTool();

	//handles the events
	static void ProcessEvents(vtkObject* object, unsigned long event,
		void* clientdata, void* calldata);

	// Controlling ivars
	void UpdateRepresentation();

	void OnKeyPress();
	void OnRightButton();

	void computeVolume();

	vtkSmartPointer<vtkCellPicker> cellPicker = nullptr;
	vtkSmartPointer<ImplicitPlaneWidget> planeWidget = nullptr;
	vtkSmartPointer<vtkActor> planeActor = nullptr;
	vtkSmartPointer<vtkActor> polygonActor = nullptr;
	vtkSmartPointer<vtkPoints> pointsPolygon = nullptr;
	float polygonHeight = 1.0;
	vtkSmartPointer<LineWidget> lineWidget = nullptr;

	Mesh* mesh = nullptr;
	wxTreeListCtrl* treeMesh = nullptr;

	bool volumeComputed = false;
	bool updatePolygon = false;

private:
	VolumeTool(const VolumeTool&) VTK_DELETE_FUNCTION;
	void operator=(const VolumeTool&) VTK_DELETE_FUNCTION;
};
