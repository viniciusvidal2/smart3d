#include "TransformDialog.h"

#include <vtkTransform.h>

#include <wx\sizer.h>
#include <wx\stattext.h>
#include <wx\spinctrl.h>
#include <wx\button.h>

wxBEGIN_EVENT_TABLE(TransformDialog, wxDialog)
EVT_BUTTON(wxID_OK, TransformDialog::OnOK)
EVT_BUTTON(idAlignWithTool, TransformDialog::OnAlignWithTool)
EVT_HOTKEY(WXK_RETURN, TransformDialog::OnEnter)
wxEND_EVENT_TABLE()

TransformDialog::TransformDialog(wxWindow * parent, wxWindowID id, const wxString & title, const wxPoint & pos, const wxSize & size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	auto bSizer = new wxBoxSizer(wxVERTICAL);

	bSizer->Add(new wxStaticText(this, wxID_ANY, "Rotation (degrees)"), 0, wxALL, 5);

	wxFlexGridSizer* flexGridRotation = new wxFlexGridSizer(1, 6, 0, 0);

	wxSize sizeSpin(60, wxDefaultSize.y);

	flexGridRotation->Add(new wxStaticText(this, wxID_ANY, "X"), 0, wxALL, 5);
	spinRotationX = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, sizeSpin, wxSP_ARROW_KEYS, -1000, 1000);
	flexGridRotation->Add(spinRotationX, 0, wxALL, 5);

	flexGridRotation->Add(new wxStaticText(this, wxID_ANY, "Y"), 0, wxALL, 5);
	spinRotationY = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, sizeSpin, wxSP_ARROW_KEYS, -1000, 1000);
	flexGridRotation->Add(spinRotationY, 0, wxALL, 5);

	flexGridRotation->Add(new wxStaticText(this, wxID_ANY, "Z"), 0, wxALL, 5);
	spinRotationZ = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, sizeSpin, wxSP_ARROW_KEYS, -1000, 1000);
	flexGridRotation->Add(spinRotationZ, 0, wxALL, 5);

	bSizer->Add(flexGridRotation, 0, wxALL, 5);


	bSizer->Add(new wxStaticText(this, wxID_ANY, "Translation"), 0, wxALL, 5);

	wxFlexGridSizer* flexGridTranslation = new wxFlexGridSizer(1, 6, 0, 0);

	flexGridTranslation->Add(new wxStaticText(this, wxID_ANY, "X"), 0, wxALL, 5);
	spinTranslationX = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, sizeSpin, wxSP_ARROW_KEYS, -1000, 1000);
	flexGridTranslation->Add(spinTranslationX, 0, wxALL, 5);

	flexGridTranslation->Add(new wxStaticText(this, wxID_ANY, "Y"), 0, wxALL, 5);
	spinTranslationY = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, sizeSpin, wxSP_ARROW_KEYS, -1000, 1000);
	flexGridTranslation->Add(spinTranslationY, 0, wxALL, 5);

	flexGridTranslation->Add(new wxStaticText(this, wxID_ANY, "Z"), 0, wxALL, 5);
	spinTranslationZ = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, sizeSpin, wxSP_ARROW_KEYS, -1000, 1000);
	flexGridTranslation->Add(spinTranslationZ, 0, wxALL, 5);

	bSizer->Add(flexGridTranslation, 0, wxALL, 5);

	wxBoxSizer* bSizerBts = new wxBoxSizer(wxHORIZONTAL);

	bSizerBts->Add(new wxButton(this, idAlignWithTool, "Align Z with tool"), 0, wxALL, 5);
	btOK = new wxButton(this, wxID_OK, "OK");
	bSizerBts->Add(btOK, 0, wxALL, 5);
	bSizerBts->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);

	bSizer->Add(bSizerBts, 0, wxALIGN_RIGHT, 5);

	this->SetSizer(bSizer);
	this->Layout();

	this->Centre(wxBOTH);
	this->RegisterHotKey(WXK_RETURN, wxMOD_NONE, WXK_RETURN);
}

TransformDialog::~TransformDialog()
{
}

void TransformDialog::OnOK(wxCommandEvent & WXUNUSED)
{
	transform = vtkSmartPointer<vtkTransform>::New();
	transform->RotateX(spinRotationX->GetValue());
	transform->RotateY(spinRotationY->GetValue());
	transform->RotateZ(spinRotationZ->GetValue());
	transform->Translate(spinTranslationX->GetValue(), spinTranslationY->GetValue(), spinTranslationZ->GetValue());
	EndModal(wxID_OK);
}

void TransformDialog::OnEnter(wxKeyEvent & event)
{
	btOK->SetFocus();
	OnOK((wxCommandEvent)NULL);
}

void TransformDialog::OnAlignWithTool(wxCommandEvent & WXUNUSED)
{
	EndModal(idAlignWithTool);
}
