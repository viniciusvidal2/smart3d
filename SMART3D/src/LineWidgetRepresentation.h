#pragma once
#include <vtkWidgetRepresentation.h>
#include <vtkSmartPointer.h>


class vtkCellLocator;
class vtkPoints;
class vtkAbstractPicker;
class vtkPropPicker;
class vtkCellPicker;
class vtkPointPicker;
class vtkTransformPolyDataFilter;
class vtkActor;
class vtkActor2D;
class vtkLineSource;

class LineWidgetRepresentation : public vtkWidgetRepresentation
{
public:
	/**
	* Instantiate this class.
	*/
	static LineWidgetRepresentation *New();

	//@{
	/**
	* Define standard methods.
	*/
	vtkTypeMacro(LineWidgetRepresentation, vtkWidgetRepresentation);
	void PrintSelf(ostream& os, vtkIndent indent) override;
	//@}

	//@{
	/**
	* Specify opposite corners of the box defining the boundary of the
	* widget. By default, these coordinates are in the normalized viewport
	* coordinate system, with Position the lower left of the outline, and
	* Position2 relative to Position. Note that using these methods are
	* affected by the ProportionalResize flag. That is, if the aspect ratio of
	* the representation is to be preserved (e.g., ProportionalResize is on),
	* then the rectangle (Position,Position2) is a bounding rectangle.
	*/
	//vtkViewportCoordinateMacro(Position);
	//vtkViewportCoordinateMacro(Position2);
	//@}

	/**
	* Define the various states that the representation can be in.
	*/
	enum _InteractionState
	{
		Outside = 0,
		OverNode
	};

	/**
	* Return the MTime of this object. It takes into account MTimes
	* of position coordinates and border's property.
	*/
	vtkMTimeType GetMTime() override;

	//@{
	/**
	* Subclasses should implement these methods. See the superclasses'
	* documentation for more information.
	*/
	void BuildRepresentation() override;
	void StartWidgetInteraction(double eventPos[2]) override;
	void WidgetInteraction(double eventPos[2]) override;
	int ComputeInteractionState(int X, int Y, int modify = 0) override;
	//@}

	//@{
	/**
	* These methods are necessary to make this representation behave as
	* a vtkProp.
	*/
	void GetActors(vtkPropCollection*) override;
	void GetActors2D(vtkPropCollection*) override;
	void ReleaseGraphicsResources(vtkWindow*) override;
	int RenderOverlay(vtkViewport*) override;
	int RenderOpaqueGeometry(vtkViewport*) override;
	int RenderTranslucentPolygonalGeometry(vtkViewport*) override;
	int HasTranslucentPolygonalGeometry() override;
	//@}

	void addProp(vtkSmartPointer<vtkProp> prop);
	void addLocator(vtkSmartPointer<vtkCellLocator> locator);

	void setAlwaysOnTop(bool alwaysOnTop);

	void addNode(int x, int y);
	bool addFinalNode(int x, int y);
	void updateActiveNode(int x, int y);
	void finishUpdateActiveNode(int x, int y);
	void updateLine(int x, int y);
	void removeLastNode();
	void removeActiveNode();
	void resetPickers();
	void reset();
	vtkSmartPointer<vtkPoints> getPoints();

	bool isOverFirstNode(int x, int y);
	void closeLoop();
	bool isLoopClosed();
	int getNumberOfNodes();

	void set2DRepresentation(bool is2D);
	void setDrawLine(bool drawLine);
	void setSphereColor(double* color);

protected:
	LineWidgetRepresentation();
	~LineWidgetRepresentation() override;

	bool pickNode(int x, int y);

	//Picker used in the continous draw
	vtkSmartPointer<vtkPropPicker> meshPropPicker = NULL;
	//Picker used to place the nodes in the mesh
	vtkSmartPointer<vtkCellPicker> meshCellPicker = NULL;
	//Picker used to place the nodes in the point cloud
	vtkSmartPointer<vtkPointPicker> meshPointPicker = NULL;

	//Actors
	std::vector<vtkSmartPointer<vtkTransformPolyDataFilter>> nodesTransformFilter;
	std::vector<vtkSmartPointer<vtkActor>> nodesActors;
	vtkSmartPointer<vtkPoints> nodePoints = NULL;
	vtkSmartPointer<vtkActor> lineActor = NULL;
	vtkSmartPointer<vtkActor2D> lineActor2D = NULL;
	vtkSmartPointer<vtkLineSource> lineSource = NULL;

	bool alwaysOnTop = true;
	bool loopClosed = false;

	int pickPosition(vtkSmartPointer<vtkAbstractPicker> picker, int x, int y, double * point);
	int pickFinalPosition(int x, int y, double * point);
	void createNode(double* point);
	void deleteNode(int id);

	int indexActiveNode = -1;
	int pixelTolerance = 10;
	double* getDisplayPosition(double* point);
	bool representation2D = false;
	// Draw the line connecting the points
	bool drawLine = true;
	// Sphere color RGB
	double sphereColor[3] = {0.0, 1.0, 0.0};

private:
	LineWidgetRepresentation(const LineWidgetRepresentation&) = delete;
	void operator=(const LineWidgetRepresentation&) = delete;
};