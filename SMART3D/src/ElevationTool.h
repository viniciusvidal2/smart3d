#pragma once
#include <vtkInteractionWidgetsModule.h> // For export macro
#include <vtk3DWidget.h>
#include <vtkSmartPointer.h>

class vtkPolyData;
class vtkElevationFilter;
class vtkActor;
class vtkSliderWidget;
class vtkPropPicker;
class vtkPoints;
class vtkXYPlotActor;
class wxTreeListCtrl;
class captionActor2D;
class LineWidget;
class Mesh;

class ElevationTool : public vtk3DWidget
{
public:
	/**
	* Instantiate the object.
	*/
	static ElevationTool *New();

	vtkTypeMacro(ElevationTool, vtk3DWidget);

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
	void setMesh(Mesh* mesh);
	//@}

	//@{
	/**
	* Set the mesh tree
	*/
	void setMeshTree(wxTreeListCtrl* treeMesh) { this->treeMesh = treeMesh; };
	//@}

protected:
	ElevationTool();
	~ElevationTool();

	//Delete what is necessary to properly disable/kill this class
	void destruct();

	//handles the events
	static void ProcessEvents(vtkObject* object, unsigned long event,
		void* clientdata, void* calldata);

	// ProcessEvents() dispatches to these methods.
	void OnMiddleButtonDown();
	void OnKeyPressed();

	//Actors
	vtkSmartPointer<captionActor2D> textActor = nullptr;

	//ElevationMap
	vtkSmartPointer<vtkPolyData> outputElevationFilter = nullptr;
	vtkSmartPointer<vtkElevationFilter> elevationFilter = nullptr;
	vtkSmartPointer<vtkActor> elevationMapActor = nullptr;

	//Slider
	vtkSmartPointer<vtkSliderWidget> sliderWidget = nullptr;
	void createSlider();
	void OnSliderChanged();

	void updateElevationMap();
	void updateText();
	double getElevationDistance(double dist);
	double getPointElevation(double* point);


	// Do the picking
	vtkSmartPointer<vtkPropPicker> propPicker = nullptr;
	Mesh* mesh = nullptr;

	//Set the zero by clicking in the mesh
	bool setZeroByMouse = false;

	double* bounds = nullptr;

	int getMousePosition(double * point);

	//ElevationProfile
	vtkSmartPointer<LineWidget> lineWidget = nullptr;
	void createElevationProfileLine();
	/*
	test if point is inside PointA--------
							  |           |
							  |           |
							  |         pointB
							  -------------
	*/
	bool isInsideBoundingBox(double* point, double* pointA, double* pointB);
	void increasePointDensity(vtkSmartPointer<vtkPoints> points, double* point1, double* point2);
	vtkSmartPointer<vtkActor> elevationProfileActor = nullptr;
	//Plot
	vtkSmartPointer<vtkXYPlotActor> plotElevation = nullptr;
	void createPlot(vtkSmartPointer<vtkPoints> points, double* point0Line, double* point1Line);

	//Avoid updating if there is no need
	double* lastElevationHeight = nullptr;

	wxTreeListCtrl * treeMesh = nullptr;

private:
	ElevationTool(const ElevationTool&) VTK_DELETE_FUNCTION;
	void operator=(const ElevationTool&) VTK_DELETE_FUNCTION;
};

