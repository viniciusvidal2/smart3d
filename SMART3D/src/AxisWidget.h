#pragma once
#include "vtkInteractionWidgetsModule.h" // For export macro
#include "vtkInteractorObserver.h"

class vtkActor2D;
class vtkPolyData;
class vtkProp;
class AxisWidgetWidgetObserver;
class vtkRenderer;

class AxisWidget : public vtkInteractorObserver
{
public:
	static AxisWidget* New();
	vtkTypeMacro(AxisWidget, vtkInteractorObserver);
	void PrintSelf(ostream& os, vtkIndent indent) override;

	//@{
	/**
	* Set/get the orientation marker to be displayed in this widget.
	*/
	virtual void SetOrientationMarker(vtkProp *prop);
	vtkGetObjectMacro(OrientationMarker, vtkProp);
	//@}

	/**
	* Enable/disable the widget. Default is 0 (disabled).
	*/
	void SetEnabled(int) override;

	/**
	* Callback to keep the camera for the orientation marker up to date with the
	* camera in the parent renderer.
	*/
	void ExecuteCameraUpdateEvent(vtkObject *o, unsigned long event, void *calldata);

	//@{
	/**
	* Set/get whether to allow this widget to be interactively moved/scaled.
	* Default is On.
	*/
	void SetInteractive(vtkTypeBool state);
	vtkGetMacro(Interactive, vtkTypeBool);
	vtkBooleanMacro(Interactive, vtkTypeBool);
	//@}

	//@{
	/**
	* Set/get the color of the outline of this widget.  The outline is visible
	* when (in interactive mode) the cursor is over this widget.
	* Default is white (1,1,1).
	*/
	void SetOutlineColor(double r, double g, double b);
	double *GetOutlineColor();
	//@}

	//@{
	/**
	* Set/get the viewport to position/size this widget.
	* Coordinates are expressed as (xmin,ymin,xmax,ymax), where each
	* coordinate is 0 <= coordinate <= 1.0.
	* Default is bottom left corner (0,0,0.2,0.2).
	* Note that this viewport is scaled with respect to the viewport of the
	* current renderer i.e. if the viewport of the current renderer is
	* (0.5, 0.5, 0.75, 0.75) and Viewport is set to (0, 0, 1, 1), the orientation
	* marker will be confined to a viewport of (0.5, 0.5, 0.75, 0.75) in the
	* render window.
	* \sa SetCurrentRenderer()
	*/
	vtkSetVector4Macro(Viewport, double);
	vtkGetVector4Macro(Viewport, double);
	//@}

	//@{
	/**
	* The tolerance representing the distance to the widget (in pixels)
	* in which the cursor is considered to be on the widget, or on a
	* widget feature (e.g., a corner point or edge).
	*/
	vtkSetClampMacro(Tolerance, int, 1, 10);
	vtkGetMacro(Tolerance, int);
	//@}

	//@{
	/**
	* Need to reimplement this->Modified() because of the
	* vtkSetVector4Macro/vtkGetVector4Macro use
	*/
	void Modified() override;
	//@}

protected:
	AxisWidget();
	~AxisWidget() override;

	vtkRenderer *Renderer;
	vtkProp     *OrientationMarker;
	vtkPolyData *Outline;
	vtkActor2D  *OutlineActor;

	unsigned long StartEventObserverId;

	static void ProcessEvents(vtkObject *object, unsigned long event,
		void *clientdata, void *calldata);

	// ProcessEvents() dispatches to these methods.
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void OnMouseMove();

	// observer to update the renderer's camera
	AxisWidgetWidgetObserver *Observer;

	vtkTypeBool Interactive;
	int Tolerance;
	int Moving;

	// viewport to position/size this widget
	double Viewport[4];

	// used to compute relative movements
	int StartPosition[2];

	// Manage the state of the widget
	int State;
	enum WidgetState
	{
		Outside = 0,
		Inside,
		Translating,
		AdjustingP1,
		AdjustingP2,
		AdjustingP3,
		AdjustingP4
	};


	// use to determine what state the mouse is over, edge1 p1, etc.
	// returns a state from the WidgetState enum above
	int ComputeStateBasedOnPosition(int X, int Y, int *pos1, int *pos2);

	// set the cursor to the correct shape based on State argument
	void SetCursor(int state);

	// adjust the viewport depending on state
	void MoveWidget(int X, int Y);
	void ResizeTopLeft(int X, int Y);
	void ResizeTopRight(int X, int Y);
	void ResizeBottomLeft(int X, int Y);
	void ResizeBottomRight(int X, int Y);

	void SquareRenderer();
	void UpdateOutline();

	// Used to reverse compute the Viewport ivar with respect to the current
	// renderer viewport
	void UpdateViewport();
	// Used to compute and set the viewport on the internal renderer based on the
	// Viewport ivar. The computed viewport will be with respect to the whole
	// render window
	void UpdateInternalViewport();

private:
	AxisWidget(const AxisWidget&) = delete;
	void operator=(const AxisWidget&) = delete;

	//set up the actors and observers created by this widget
	void SetupWindowInteraction();
	//tear down up the actors and observers created by this widget
	void TearDownWindowInteraction();
};
