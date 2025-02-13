#include "SettingsDialog.h"

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/button.h>

wxBEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
	EVT_BUTTON(wxID_OK, SettingsDialog::OnOK)
wxEND_EVENT_TABLE()

//Interface
int SettingsDialog::interfaceType = 2;

SettingsDialog::SettingsDialog(wxWindow * parent, wxWindowID id, const wxString & title, const wxPoint & pos, const wxSize & size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* bSizerInterface = new wxBoxSizer(wxHORIZONTAL);

	//Interface
	wxStaticBoxSizer* sbSizerInterface = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Interface"), wxVERTICAL);

	wxFlexGridSizer* fgSizerInterface = new wxFlexGridSizer(1, 2, 0, 0);
	fgSizerInterface->SetFlexibleDirection(wxBOTH);
	fgSizerInterface->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	fgSizerInterface->Add(new wxStaticText(sbSizerInterface->GetStaticBox(), wxID_ANY, "Interface Type"), 0, wxALL, 5);
	wxArrayString choicesInterface;
	choicesInterface.Add("Reconstructor"); choicesInterface.Add("Analyzer"); choicesInterface.Add("Advanced");
	choiceInterfaceType = new wxChoice(sbSizerInterface->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, choicesInterface);
	choiceInterfaceType->SetSelection(interfaceType);
	fgSizerInterface->Add(choiceInterfaceType, 0, wxALL, 5);

	sbSizerInterface->Add(fgSizerInterface, 1, wxEXPAND, 5);

	bSizerInterface->Add(sbSizerInterface, 0, wxEXPAND, 5);

	//Add mesh and texture box sizers
	bSizer->Add(bSizerInterface, 0, wxEXPAND, 5);

	wxBoxSizer* bSizerBts = new wxBoxSizer(wxHORIZONTAL);

	bSizerBts->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL, 5);

	bSizerBts->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);

	bSizer->Add(bSizerBts, 0, wxALIGN_RIGHT, 5);
	this->SetSizer(bSizer);
	this->Layout();

	this->Centre(wxBOTH);
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::OnOK(wxCommandEvent & WXUNUSED)
{
	//Interface
	interfaceType = choiceInterfaceType->GetSelection();
	EndModal(wxID_OK);
}