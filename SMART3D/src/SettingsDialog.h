#pragma once
#include <wx/dialog.h>

class wxChoice;

class SettingsDialog : public wxDialog
{
public:
	SettingsDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = "Settings", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(300, 150), long style = wxDEFAULT_DIALOG_STYLE);
	~SettingsDialog();

	//0 - reconstructor 1 - analyzer 2 - advanced
	static int getInterfaceType() { return interfaceType; };

private:
	DECLARE_EVENT_TABLE()

	void OnOK(wxCommandEvent& WXUNUSED(event));

	wxChoice* choiceInterfaceType;

	static int interfaceType;

};