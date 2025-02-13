#pragma once
#include <wx/dialog.h>

class wxChoice;
struct OptionsSSD;

class SSDDialog : public wxDialog
{
public:
	SSDDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = "SSD", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(300, 140), long style = wxDEFAULT_DIALOG_STYLE);
	~SSDDialog();

	OptionsSSD* getOptions();

private:
	DECLARE_EVENT_TABLE()

	void OnBtDefault(wxCommandEvent& WXUNUSED(event));
	
	OptionsSSD* options = nullptr;

	wxSpinCtrl* spinDepth;
	wxChoice* choiceBoundary;
};
enum EnumSSDDialog
{
	idBtDefaultSSDDialog
};