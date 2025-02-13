#pragma once
#include <vtkInteractionWidgetsModule.h> // For export macro
#include <vtk3DWidget.h>
#include <vtkSmartPointer.h>
#include <vector>

class vtkPolygon;
class vtkCellPicker;
class vtkActor;
class wxTreeListCtrl;
class LineWidget;
class Mesh;
class Camera;

class CheckCamTool : public vtk3DWidget
{
public:
	/**
	* Instantiate the object.
	*/
	static CheckCamTool *New();

	vtkTypeMacro(CheckCamTool, vtk3DWidget);

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

	//@{
	/**
	* Set the mesh tree
	*/
	void setTreeMesh(wxTreeListCtrl* treeMesh)
	{
		this->treeMesh = treeMesh;
	};
	//@}

protected:
	CheckCamTool();
	~CheckCamTool();

	//handles the events
	static void ProcessEvents(vtkObject* object, unsigned long event,
		void* clientdata, void* calldata);

	vtkSmartPointer<LineWidget> lineWidget = NULL;
	vtkSmartPointer<vtkCellPicker> cellPicker = NULL;

	void createPicker();
	void checkVisibility();
	void destruct();
	void clearVisibleCameras();
	bool intersectPlaneWithLine(double * p1, vtkSmartPointer<vtkPolygon> polygon, double * pointCheckCam);

	Mesh* mesh = NULL;
	std::vector<Camera*> visibleCameras;
	std::vector<vtkSmartPointer<vtkActor>> validPoints;
	wxTreeListCtrl* treeMesh = NULL;

private:
	CheckCamTool(const CheckCamTool&) VTK_DELETE_FUNCTION;
	void operator=(const CheckCamTool&) VTK_DELETE_FUNCTION;
};
