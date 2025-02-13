#pragma once
#include <wx/dialog.h>

class wxChoice;
struct OptionsPoisson;

class PoissonDialog : public wxDialog
{
public:
	PoissonDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = "Poisson", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(300, 140), long style = wxDEFAULT_DIALOG_STYLE);
	~PoissonDialog();

	OptionsPoisson* getOptions();

private:
	DECLARE_EVENT_TABLE()

	void OnBtDefault(wxCommandEvent& WXUNUSED(event));
	
	OptionsPoisson* options = nullptr;

	wxSpinCtrl* spinDepth;
	wxChoice* choiceBoundary;
};
enum EnumPoissonDialog
{
	idBtDefaultPoissonDialog
};