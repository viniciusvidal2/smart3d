#include "SnapshotDialog.h"

#include <wx\sizer.h>
#include <wx\stattext.h>
#include <wx\filepicker.h>
#include <wx\slider.h>
#include <wx\checkbox.h>
#include <wx\msgdlg.h>

wxBEGIN_EVENT_TABLE(SnapshotDialog, wxDialog)
EVT_BUTTON(wxID_OK, SnapshotDialog::OnOK)
EVT_SLIDER(idMagSlider, SnapshotDialog::OnSliderChanged)
wxEND_EVENT_TABLE()

SnapshotDialog::SnapshotDialog(wxWindow * parent, wxWindowID id, const wxString & title, const wxPoint & pos, const wxSize & size, long style, int* resolution) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	//Sizers
	boxSizer = new wxBoxSizer(wxVERTICAL);
	auto boxSizerMag = new wxBoxSizer(wxHORIZONTAL);
	auto boxSizerDir = new wxBoxSizer(wxHORIZONTAL);
	auto boxSizerCheckBox = new wxBoxSizer(wxHORIZONTAL);
	auto boxSizerBts = new wxBoxSizer(wxHORIZONTAL);

	//Main message
	auto mainMessage = new wxStaticText(this, wxID_ANY, "Choose the final resolution, the directory and the file name to save\n the images, if you use the hotkey P the settings will be the same and\n there will be an numerical addition to the file name", wxDefaultPosition, wxDefaultSize);
	boxSizer->Add(mainMessage, 0, wxALL, 5);
	boxSizer->AddSpacer(10);

	boxSizer->Add(boxSizerMag, 0, wxEXPAND, 5);
	//Magnification
	auto magMessage = new wxStaticText(this, wxID_ANY, "Magnification", wxDefaultPosition, wxDefaultSize);
	boxSizerMag->Add(magMessage, 0, wxALL, 5);
	defaultValue = 5;
	magSlider = new wxSlider(this, idMagSlider, defaultValue, 1, 20, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	boxSizerMag->Add(magSlider, 0, wxALL, 5);
	if (resolution)
	{
		textResolution << "Output resolution: " << defaultValue * resolution[0] << "x" << defaultValue * resolution[1];
		windowResolution = resolution;
	}
	else
	{
		textResolution = "";
	}
	resMessage = new wxStaticText(this, wxID_ANY, textResolution, wxDefaultPosition, wxDefaultSize);
	boxSizerMag->Add(resMessage, 0, wxALL, 5);

	boxSizer->Add(boxSizerDir, 0, wxEXPAND, 5);
	//Directory
	auto dirMessage = new wxStaticText(this, wxID_ANY, "Directory to save", wxDefaultPosition, wxDefaultSize);
	boxSizerDir->Add(dirMessage, 0, wxALL, 5);
	filePicker = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, "Choose the directory to save the snapshot", "PNG files (*.png)|*.png", wxDefaultPosition, wxDefaultSize, wxFLP_SAVE | wxFLP_USE_TEXTCTRL);
	boxSizerDir->Add(filePicker, 0, wxALL, 5);

	boxSizer->Add(boxSizerCheckBox, 0, wxEXPAND, 5);
	checkBoxTransp = new wxCheckBox(this, wxID_ANY, "Transparent background");
	boxSizerCheckBox->Add(checkBoxTransp, 0, wxALL, 5);

	boxSizer->Add(boxSizerBts, 0, wxALIGN_RIGHT, 5);
	//Buttons
	boxSizerBts->Add(new wxButton(this, wxID_OK, "OK", wxDefaultPosition, wxDefaultSize), 0, wxALL, 5);
	boxSizerBts->Add(new wxButton(this, wxID_CANCEL, "Cancel", wxDefaultPosition, wxDefaultSize), 0, wxALL, 5);

	this->SetSizer(boxSizer);
	this->Layout();
	boxSizer->Fit(this);
	this->Centre(wxBOTH);
}

SnapshotDialog::~SnapshotDialog()
{
}

wxString SnapshotDialog::getPath()
{
	return filePicker->GetPath();
}

int SnapshotDialog::getMagnification()
{
	return magSlider->GetValue();
}

bool SnapshotDialog::getAlpha()
{
	return checkBoxTransp->IsChecked();
}

void SnapshotDialog::OnOK(wxCommandEvent & WXUNUSED)
{
	if (filePicker->GetPath() != "")
	{
		EndModal(wxID_OK);
	}
	else
	{
		wxMessageBox("You need to select a directory.", "Error", wxICON_ERROR, this);
	}
}

void SnapshotDialog::OnSliderChanged(wxCommandEvent& event)
{
	textResolution = "";
	textResolution << "Output resolution: " << magSlider->GetValue()*windowResolution[0] << "x" << magSlider->GetValue()*windowResolution[1];
	resMessage->SetLabelText(textResolution);
	this->Layout();
	boxSizer->Fit(this);
}
