#pragma once
#include <wx\dialog.h>

class wxCheckBox;

class DontShowAgainDialog : public wxDialog
{
public:
	DontShowAgainDialog(wxWindow* parent, const wxString& title = "", const wxString& mainMessage = "",
		wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
	~DontShowAgainDialog() {};

	bool getCheckBoxStatus() const;

private:

	//Don't show again checkBox
	wxCheckBox* checkBoxDontShowAgain;
};