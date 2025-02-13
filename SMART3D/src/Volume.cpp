#include "Volume.h"

#include <vtkRenderer.h>
#include <vtkCaptionActor2D.h>
#include <vtkPolyData.h>
#include <vtkActor.h>
#include <vtkTransformFilter.h>
#include <vtkTransform.h>
#include <vtkMassProperties.h>
#include <vtkMapper.h>

#include <wx/msgdlg.h>

Volume::Volume(bool visible)
{
	this->visible = visible;
}

Volume::~Volume()
{

}

void Volume::destruct(vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree)
{
	if (text)
	{
		renderer->RemoveActor(text);
		text = nullptr;
	}
	if (actor)
	{
		renderer->RemoveActor(actor);
		actor = nullptr;
	}
	if (listItem)
	{
		tree->DeleteItem(listItem);
		listItem = nullptr;
	}
}

void Volume::setVisibility(bool visibility)
{
	if (actor && text)
	{
		actor->SetVisibility(visibility);
		text->SetVisibility(visibility);
		this->visible = visibility;
	}
}

bool Volume::getVisibility()
{
	return visible;
}

void Volume::updateText(const std::string& measureUnit)
{
	if (text)
	{
		wxString ss;
		ss << "#" << index << " - " << volume << measureUnit << "\u00B3";
		text->SetCaption(ss.utf8_str());
	}
}

void Volume::updateCalibration(double calibration, const std::string& measureUnit)
{
	if (polyData)
	{
		vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
		transform->Scale(calibration, calibration, calibration);
		vtkSmartPointer<vtkTransformFilter> transformFilter = vtkSmartPointer<vtkTransformFilter>::New();
		transformFilter->SetInputData(polyData);
		transformFilter->SetTransform(transform);
		transformFilter->Update();

		//Compute the volume
		vtkSmartPointer<vtkMassProperties> massProperties = vtkSmartPointer<vtkMassProperties>::New();
		massProperties->SetInputConnection(transformFilter->GetOutputPort());
		massProperties->Update();

		if ((massProperties->GetVolume() - massProperties->GetVolumeProjected()) * 10000 > massProperties->GetVolume())//If we are updating the calibration this is really unlikely to happen
		{
			wxMessageBox("Something went wrong", "Error", wxICON_ERROR);
		}
		else
		{
			volume = massProperties->GetVolume();
			updateText(measureUnit);
		}
	}
}

void Volume::transform(vtkSmartPointer<vtkTransform> T)
{
	if (actor)
	{
		vtkSmartPointer<vtkTransformFilter> transformFilter = vtkSmartPointer<vtkTransformFilter>::New();
		transformFilter->SetTransform(T);
		transformFilter->SetInputData(actor->GetMapper()->GetInputAsDataSet());
		transformFilter->Update();
		actor->GetMapper()->SetInputConnection(transformFilter->GetOutputPort());
		updateTextPosition();
	}
}

void Volume::updateTextPosition()
{
	text->SetAttachmentPoint(actor->GetCenter());
}
