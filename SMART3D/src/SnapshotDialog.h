#pragma once
#include <wx\dialog.h>

class wxBoxSizer;
class wxStaticText;
class wxString;
class wxFilePickerCtrl;
class wxSlider;
class wxCheckBox;
class wxButton;

class SnapshotDialog : public wxDialog
{
public:
	SnapshotDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = "Snapshot", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE, int* resolution = NULL);
	~SnapshotDialog();

	wxString getPath();
	int getMagnification();
	bool getAlpha();

private:
	DECLARE_EVENT_TABLE()

	void OnOK(wxCommandEvent& WXUNUSED(event));
	void OnSliderChanged(wxCommandEvent& event);

	//Sizers
	wxBoxSizer* boxSizer;

	//Texts
	wxStaticText* resMessage;
	wxString textResolution;

	//Dir
	wxFilePickerCtrl* filePicker;

	//Magnification
	wxSlider* magSlider;

	//Transparency
	wxCheckBox* checkBoxTransp;

	//Resolution
	int defaultValue;
	int* windowResolution;
};

enum {
	idMagSlider
};