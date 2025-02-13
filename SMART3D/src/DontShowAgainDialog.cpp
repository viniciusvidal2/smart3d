#include "DontShowAgainDialog.h"

#include <wx\sizer.h>
#include <wx\stattext.h>
#include <wx\checkbox.h>
#include "wx\button.h"

DontShowAgainDialog::DontShowAgainDialog(wxWindow * parent,const wxString& title, const wxString& mainMessage, wxWindowID id, const wxPoint & pos, const wxSize & size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	//Sizers
	auto boxSizer = new wxBoxSizer(wxVERTICAL);
	auto boxSizerCheckBox = new wxBoxSizer(wxHORIZONTAL);
	auto boxSizerBts = new wxBoxSizer(wxHORIZONTAL);

	//Main message
	boxSizer->Add(new wxStaticText(this, wxID_ANY, mainMessage), 0, wxALL, 5);
	boxSizer->AddSpacer(10);


	boxSizer->Add(boxSizerCheckBox, 0, wxEXPAND, 5);
	checkBoxDontShowAgain = new wxCheckBox(this, wxID_ANY, "Don't show this message again");
	boxSizerCheckBox->Add(checkBoxDontShowAgain, 0, wxALL, 5);

	boxSizer->Add(boxSizerBts, 0, wxALIGN_RIGHT, 5);
	//Buttons
	boxSizerBts->Add(new wxButton(this, wxID_OK, "OK", wxDefaultPosition, wxDefaultSize), 0, wxALL, 5);

	this->SetSizer(boxSizer);
	this->Layout();
	boxSizer->Fit(this);
	this->Centre(wxBOTH);
}

bool DontShowAgainDialog::getCheckBoxStatus() const
{
	return checkBoxDontShowAgain->IsChecked();
}
