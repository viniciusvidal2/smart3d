#pragma once
#include <vtkSliderRepresentation.h>
#include <vtkSmartPointer.h>

class vtkTextActor;
class vtkTextProperty;
class vtkActor2D;
class vtkTransformFilter;
class Calibration;

class sliderRep2D : public vtkSliderRepresentation
{
public:
	/**
	* Instantiate the class.
	*/
	static sliderRep2D *New();

	//@{
	/**
	* Standard methods for the class.
	*/
	vtkTypeMacro(sliderRep2D, vtkSliderRepresentation);
	//@}

	//@{
	/**
	* Specify the label text for this widget. If the value is not set, or set
	* to the empty string "", then the label text is not displayed.
	*/
	void SetTitleText(const char*) VTK_OVERRIDE;
	const char* GetTitleText() VTK_OVERRIDE;
	//@}

	//@{
	/**
	* Set the text Limits.
	*/
	void SetMaxLimitText(const char* text);
	void SetMinLimitText(const char* text);
	//@}

	//@{
	/**
	* Methods to interface with the vtkSliderWidget. The PlaceWidget() method
	* assumes that the parameter bounds[6] specifies the location in display space
	* where the widget should be placed.
	*/
	void PlaceWidget(double bounds[6]) VTK_OVERRIDE;
	void BuildRepresentation() VTK_OVERRIDE;
	void StartWidgetInteraction(double eventPos[2]) VTK_OVERRIDE;
	void WidgetInteraction(double newEventPos[2]) VTK_OVERRIDE;
	void Highlight(int) VTK_OVERRIDE;
	//@}

	//@{
	/**
	* Methods supporting the rendering process.
	*/
	void GetActors2D(vtkPropCollection*) VTK_OVERRIDE;
	void ReleaseGraphicsResources(vtkWindow*) VTK_OVERRIDE;
	int RenderOverlay(vtkViewport*) VTK_OVERRIDE;
	int RenderOpaqueGeometry(vtkViewport*) VTK_OVERRIDE;
	//@}

	void setCalibration(Calibration* calib);

protected:
	sliderRep2D();
	~sliderRep2D() VTK_OVERRIDE;

	// Determine the parameter t along the slider
	virtual double ComputePickPosition(double eventPos[2]);


	//Texts
	vtkSmartPointer<vtkTextProperty> textProperty;
	vtkSmartPointer<vtkTextActor> titleTextActor = NULL;
	vtkSmartPointer<vtkTextActor> minTextActor = NULL;
	vtkSmartPointer<vtkTextActor> maxTextActor = NULL;
	vtkSmartPointer<vtkTextActor> valueTextActor = NULL;
	//Slider
	double xLenghtTube = 80;
	double yLenghtTube = 2;
	double sliderRadius = 8;
	double xMinTube = -1;
	double xMaxTube = -1;
	double yMinTube = -1;
	double yMaxTube = -1;
	double xMinSlider = -1;
	double xMaxSlider = -1;
	double yMinSlider = -1;
	double yMaxSlider = -1;
	vtkSmartPointer<vtkActor2D> sliderTubeActor = NULL;
	vtkSmartPointer<vtkActor2D> sliderActor = NULL;

	vtkSmartPointer<vtkTransformFilter> sliderTubeTransFilter;
	vtkSmartPointer<vtkTransformFilter> sliderTransFilter;

	// internal variables used for computation
	double X;

	Calibration* calib = NULL;

private:
	sliderRep2D(const sliderRep2D&) VTK_DELETE_FUNCTION;
	void operator=(const sliderRep2D&) VTK_DELETE_FUNCTION;
};