#include "PoissonDialog.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/button.h>

#include "PoissonRecon.h"

wxBEGIN_EVENT_TABLE(PoissonDialog, wxDialog)
	EVT_BUTTON(idBtDefaultPoissonDialog, PoissonDialog::OnBtDefault)
wxEND_EVENT_TABLE()

PoissonDialog::PoissonDialog(wxWindow * parent, wxWindowID id, const wxString & title, const wxPoint & pos, const wxSize & size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	options = new OptionsPoisson();
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* fgSizer = new wxFlexGridSizer(2, 2, 0, 0);
	fgSizer->SetFlexibleDirection(wxBOTH);
	fgSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Depth"), 0, wxALL, 5);
	spinDepth = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 16384L, 6, 20, options->depth);
	fgSizer->Add(spinDepth, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Boundary"), 0, wxALL, 5);
	wxArrayString choicesBoundary;
	choicesBoundary.Add("Free"); choicesBoundary.Add("Dirichlet"); choicesBoundary.Add("Neumann");
	choiceBoundary = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choicesBoundary);
	choiceBoundary->SetSelection(options->boundary - 1);
	fgSizer->Add(choiceBoundary, 0, wxALL, 5);
	
	bSizer->Add(fgSizer, 0, wxEXPAND, 5);

	wxBoxSizer* bSizerBts = new wxBoxSizer(wxHORIZONTAL);

	bSizerBts->Add(new wxButton(this, idBtDefaultPoissonDialog, "Default"), 0, wxALL, 5);

	bSizerBts->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL, 5);

	bSizerBts->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);

	bSizer->Add(bSizerBts, 0, wxALIGN_RIGHT, 5);
	
	this->SetSizer(bSizer);
	this->Layout();

	this->Centre(wxBOTH);
}

PoissonDialog::~PoissonDialog()
{
	if (options)
	{
		delete options;
		options = nullptr;
	}
}

OptionsPoisson * PoissonDialog::getOptions()
{
	options->depth = spinDepth->GetValue();
	options->boundary = choiceBoundary->GetSelection() + 1;
	return options;
}

void PoissonDialog::OnBtDefault(wxCommandEvent & WXUNUSED)
{
	spinDepth->SetValue(options->depth);
	choiceBoundary->SetSelection(options->boundary - 1);
}