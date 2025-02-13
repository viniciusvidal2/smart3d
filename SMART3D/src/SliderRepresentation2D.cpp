#include "sliderRep2D.h"

#include <vtkObjectFactory.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkRegularPolygonSource.h>
#include <vtkTransformFilter.h>
#include <vtkTransform.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkCubeSource.h>
#include <vtkActor2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkWindow.h>

#include "Calibration.h"

vtkStandardNewMacro(sliderRep2D);


void sliderRep2D::setCalibration(Calibration * calib)
{
	this->calib = calib;
}

//----------------------------------------------------------------------
sliderRep2D::sliderRep2D()
{
	//Text Property
	textProperty = vtkSmartPointer<vtkTextProperty>::New();
	textProperty->SetBold(1);
	textProperty->SetShadow(1);
	textProperty->SetFontFamilyToArial();
	textProperty->SetJustificationToCentered();
	textProperty->SetVerticalJustificationToCentered();

	//Limits
	maxTextActor = vtkSmartPointer<vtkTextActor>::New();
	maxTextActor->SetInput("");
	maxTextActor->SetTextProperty(textProperty);

	minTextActor = vtkSmartPointer<vtkTextActor>::New();
	minTextActor->SetInput("");
	minTextActor->SetTextProperty(textProperty);

	//Title
	titleTextActor = vtkSmartPointer<vtkTextActor>::New();
	titleTextActor->SetInput("");
	titleTextActor->SetTextProperty(textProperty);

	//Value
	valueTextActor = vtkSmartPointer<vtkTextActor>::New();
	valueTextActor->SetInput("");
	valueTextActor->SetTextProperty(textProperty);


	//Slider
	vtkSmartPointer<vtkTransform> T = vtkSmartPointer<vtkTransform>::New();
	T->Identity();

	vtkSmartPointer<vtkRegularPolygonSource> polygonSource = vtkSmartPointer<vtkRegularPolygonSource>::New();
	polygonSource->SetNumberOfSides(50);
	polygonSource->SetRadius(sliderRadius);
	polygonSource->SetCenter(0, 0, 0);

	sliderTransFilter = vtkSmartPointer<vtkTransformFilter>::New();
	sliderTransFilter->SetTransform(T);
	sliderTransFilter->SetInputConnection(polygonSource->GetOutputPort());
	sliderTransFilter->Update();

	vtkSmartPointer<vtkPolyDataMapper2D> sliderMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
	sliderMapper->SetInputConnection(sliderTransFilter->GetOutputPort());

	sliderActor = vtkSmartPointer<vtkActor2D>::New();
	sliderActor->GetProperty()->SetColor(0, 0, 1);
	sliderActor->SetMapper(sliderMapper);


	vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
	cubeSource->SetCenter(0, 0, 0);
	cubeSource->SetXLength(xLenghtTube * 2);
	cubeSource->SetYLength(yLenghtTube * 2);
	cubeSource->SetZLength(1);

	sliderTubeTransFilter = vtkSmartPointer<vtkTransformFilter>::New();
	sliderTubeTransFilter->SetTransform(T);
	sliderTubeTransFilter->SetInputConnection(cubeSource->GetOutputPort());
	sliderTubeTransFilter->Update();

	vtkSmartPointer<vtkPolyDataMapper2D> sliderTubeMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
	sliderTubeMapper->SetInputConnection(sliderTubeTransFilter->GetOutputPort());

	sliderTubeActor = vtkSmartPointer<vtkActor2D>::New();
	sliderTubeActor->GetProperty()->SetColor(1, 1, 1);
	sliderTubeActor->SetMapper(sliderTubeMapper);
}

sliderRep2D::~sliderRep2D()
{
	if (titleTextActor != NULL)
	{
		this->GetRenderer()->RemoveActor(titleTextActor);
		titleTextActor = NULL;
	}
	if (minTextActor != NULL)
	{
		this->GetRenderer()->RemoveActor(minTextActor);
		minTextActor = NULL;
	}
	if (maxTextActor != NULL)
	{
		this->GetRenderer()->RemoveActor(maxTextActor);
		maxTextActor = NULL;
	}
	if (valueTextActor != NULL)
	{
		this->GetRenderer()->RemoveActor(valueTextActor);
		valueTextActor = NULL;
	}
	if (sliderTubeActor != NULL)
	{
		this->GetRenderer()->RemoveActor2D(sliderTubeActor);
		sliderTubeActor = NULL;
	}
	if (sliderActor != NULL)
	{
		this->GetRenderer()->RemoveActor2D(sliderActor);
		sliderActor = NULL;
	}
}

//----------------------------------------------------------------------
void sliderRep2D::SetTitleText(const char* label)
{
	titleTextActor->SetInput(label);
}

//----------------------------------------------------------------------
const char* sliderRep2D::GetTitleText()
{
	return titleTextActor->GetInput();
}

//----------------------------------------------------------------------
void sliderRep2D::StartWidgetInteraction(double eventPos[2])
{
	if (eventPos[0] > xMinSlider && eventPos[0] < xMaxSlider && eventPos[1] > yMinSlider && eventPos[1] < yMaxSlider)
	{
		this->InteractionState = vtkSliderRepresentation::Slider;
		return;
	}
	if (eventPos[0] > xMinTube && eventPos[0] < xMaxTube && eventPos[1] > yMinTube && eventPos[1] < yMaxTube)
	{
		this->InteractionState = vtkSliderRepresentation::Tube;
		this->ComputePickPosition(eventPos);
		return;
	}
	this->InteractionState = vtkSliderRepresentation::Outside;
}

//----------------------------------------------------------------------
void sliderRep2D::WidgetInteraction(double eventPos[2])
{
	double t = this->ComputePickPosition(eventPos);
	this->SetValue(this->MinimumValue + t * (this->MaximumValue - this->MinimumValue));
	this->BuildRepresentation();
}

void sliderRep2D::SetMaxLimitText(const char* text)
{
	maxTextActor->SetInput(text);
}

void sliderRep2D::SetMinLimitText(const char* text)
{
	minTextActor->SetInput(text);
}

//----------------------------------------------------------------------
void sliderRep2D::PlaceWidget(double *vtkNotUsed(bds[6]))
{
	// Position the handles at the end of the lines
	this->BuildRepresentation();
}

//----------------------------------------------------------------------
double sliderRep2D::ComputePickPosition(double eventPos[2])
{
	if (xMinTube == -1)
	{
		return 0.0;
	}
	this->PickedT = (eventPos[0] - xMinTube) / (xMaxTube - xMinTube);
	this->PickedT = (this->PickedT < 0 ? 0.0 :
		(this->PickedT > 1.0 ? 1.0 : this->PickedT));

	return this->PickedT;
}

//----------------------------------------------------------------------
void sliderRep2D::Highlight(int highlight)
{
	if (highlight)
	{
		sliderActor->GetProperty()->SetColor(1, 0, 0);
	}
	else
	{
		sliderActor->GetProperty()->SetColor(0, 0, 1);
	}
}


//----------------------------------------------------------------------
void sliderRep2D::BuildRepresentation()
{
	if (this->GetMTime() > this->BuildTime || (this->Renderer && this->Renderer->GetVTKWindow() && this->Renderer->GetVTKWindow()->GetMTime() > this->BuildTime))
	{
		int *size = this->Renderer->GetSize();
		if (0 == size[0] || 0 == size[1])
		{
			// Renderer has no size yet: wait until the next
			// BuildRepresentation...
			return;
		}
		double scale = size[0] / 500.f;
		if (scale*xLenghtTube > 150)
		{
			scale = 150.f / xLenghtTube;
		}
		if (scale*xLenghtTube < 70)
		{
			scale = 70.f / xLenghtTube;
		}
		//Sliders
		vtkSmartPointer<vtkTransform> T = vtkSmartPointer<vtkTransform>::New();
		T->Scale(scale, scale, 1);
		sliderTubeTransFilter->SetTransform(T);
		sliderTubeTransFilter->Update();
		sliderTransFilter->SetTransform(T);
		sliderTransFilter->Update();

		double xCenterTube = (xLenghtTube + 30) * scale;
		double yCenterTube = (yLenghtTube + 35) * scale;
		double xHalfTube = xLenghtTube * scale;
		double yHalfTube = (yLenghtTube + 5) * scale;// + 5 to make easier to select 
		xMinTube = xCenterTube - xHalfTube;
		xMaxTube = xCenterTube + xHalfTube;
		yMinTube = yCenterTube - yHalfTube;
		yMaxTube = yCenterTube + yHalfTube;
		sliderTubeActor->SetPosition(xCenterTube, yCenterTube);

		double t = (this->Value - this->MinimumValue) / (this->MaximumValue - this->MinimumValue);
		double xSlider = (xMaxTube - xMinTube)*t;
		double halfSlider = sliderRadius * scale;
		xMinSlider = xMinTube + xSlider - halfSlider;
		xMaxSlider = xMinTube + xSlider + halfSlider;
		yMinSlider = yCenterTube - halfSlider;
		yMaxSlider = yCenterTube + halfSlider;
		sliderActor->SetPosition(xMinTube + xSlider, yCenterTube);

		//Texts
		titleTextActor->SetPosition(xCenterTube, yCenterTube - 20 * scale);
		char label[256];
		if (calib != NULL)
		{
			snprintf(label, sizeof(label), this->LabelFormat, this->Value * calib->getScaleFactor());
		}
		else
		{
			snprintf(label, sizeof(label), this->LabelFormat, this->Value);
		}
		valueTextActor->SetInput(label);
		valueTextActor->SetPosition(xMinTube + xSlider, yCenterTube + 20 * scale);
		minTextActor->SetPosition(xMinTube, yCenterTube - 20 * scale);
		maxTextActor->SetPosition(xMaxTube, yCenterTube - 20 * scale);
		textProperty->SetFontSize(15 * scale);

		this->BuildTime.Modified();
	}
}

//----------------------------------------------------------------------
void sliderRep2D::GetActors2D(vtkPropCollection *pc)
{
	pc->AddItem(maxTextActor);
	pc->AddItem(minTextActor);
	pc->AddItem(titleTextActor);
	pc->AddItem(valueTextActor);
	pc->AddItem(sliderTubeActor);
	pc->AddItem(sliderActor);
}

//----------------------------------------------------------------------
void sliderRep2D::ReleaseGraphicsResources(vtkWindow *w)
{
	maxTextActor->ReleaseGraphicsResources(w);
	minTextActor->ReleaseGraphicsResources(w);
	titleTextActor->ReleaseGraphicsResources(w);
	valueTextActor->ReleaseGraphicsResources(w);
	sliderTubeActor->ReleaseGraphicsResources(w);
	sliderActor->ReleaseGraphicsResources(w);
}

//----------------------------------------------------------------------
int sliderRep2D::RenderOpaqueGeometry(vtkViewport *viewport)
{
	this->BuildRepresentation();
	int count = sliderTubeActor->RenderOpaqueGeometry(viewport);
	count += sliderActor->RenderOpaqueGeometry(viewport);
	count += maxTextActor->RenderOpaqueGeometry(viewport);
	count += minTextActor->RenderOpaqueGeometry(viewport);
	count += titleTextActor->RenderOpaqueGeometry(viewport);
	count += valueTextActor->RenderOpaqueGeometry(viewport);
	return count;
}

//----------------------------------------------------------------------
int sliderRep2D::RenderOverlay(vtkViewport *viewport)
{
	this->BuildRepresentation();
	int count = sliderTubeActor->RenderOverlay(viewport);
	count += sliderActor->RenderOverlay(viewport);
	count += maxTextActor->RenderOverlay(viewport);
	count += minTextActor->RenderOverlay(viewport);
	count += titleTextActor->RenderOverlay(viewport);
	count += valueTextActor->RenderOverlay(viewport);
	return count;
}
