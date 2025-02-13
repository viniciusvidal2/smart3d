#pragma once
#include <vtkWidgetRepresentation.h>
#include <vtkSmartPointer.h>

class vtkActor2D;
class wxImage;

class ViewToolRepresentation : public vtkWidgetRepresentation
{
public:
	/**
	* Instantiate this class.
	*/
	static ViewToolRepresentation *New();

	//@{
	/**
	* Define standard methods.
	*/
	vtkTypeMacro(ViewToolRepresentation, vtkWidgetRepresentation);
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
		TopView,
		BottomView,
		FrontView,
		BackView,
		LeftView,
		RightView
	};

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
	void GetActors2D(vtkPropCollection*) override;
	void ReleaseGraphicsResources(vtkWindow*) override;
	int RenderOverlay(vtkViewport*) override;
	int RenderOpaqueGeometry(vtkViewport*) override;
	int RenderTranslucentPolygonalGeometry(vtkViewport*) override;
	int HasTranslucentPolygonalGeometry() override;
	//@}

protected:
	ViewToolRepresentation();
	~ViewToolRepresentation() override;

	// Keep track of start position when moving border
	double StartPosition[2];

	double HalfButtonSize = 16;
	/*TopView,0
	BottomView,1
	FrontView,2
	BackView,3
	LeftView,4
	RightView,5*/
	std::vector<vtkSmartPointer<vtkActor2D>> viewActors;
	std::vector<wxImage> viewImages;
	std::vector<int*> viewActorsDisplayPosition;

	bool isInsideButton(int x, int y, int* pos);

private:
	ViewToolRepresentation(const ViewToolRepresentation&) = delete;
	void operator=(const ViewToolRepresentation&) = delete;
};