#include "AboutDialog.h"

#include <wx\sizer.h>
#include <wx\stattext.h>
#include <wx\statbmp.h>
#include <wx\hyperlink.h>
#include <wx\statline.h>

AboutDialog::AboutDialog(wxWindow * parent, wxWindowID id, const wxString & title,
	const wxPoint & pos, const wxSize & size, long style) : wxDialog(parent, id, title, pos, size, style)
{
  auto bSizerMART = new wxBoxSizer(wxHORIZONTAL);

  bSizerMART->Add(new wxStaticBitmap(this, wxID_ANY, wxICON(AAAAAPROGRAM)), 0, wxALL, 5);
  wxString s;
  s << "SM.A.R.T. 3D - Surface/Mesh Analysis and Reconstruction Tool " << MART_VERSION_MAJOR << "." << MART_VERSION_MINOR << "." << MART_VERSION_PATCH;
  wxStaticText* label = new wxStaticText(this, wxID_ANY, s);
  wxFont fontBig(*wxNORMAL_FONT);
  fontBig.SetPointSize(fontBig.GetPointSize() + 2);
  fontBig.SetWeight(wxFONTWEIGHT_BOLD);
  label->SetFont(fontBig);
  bSizerMART->Add(label, 0, wxALL | wxALIGN_CENTER, 5);

  auto bSizer = new wxBoxSizer(wxVERTICAL);

  bSizer->Add(bSizerMART, 0, wxALL, 5);

  bSizer->Add(new wxStaticText(this, wxID_ANY, "Project supported by INESC P&&D Brasil\nCoordinator - Leonardo de Mello Honório\nIntegrants - Marcelo Roberto Petry and Juliano Emir Nunes Masson"), 0, wxALL | wxALIGN_CENTER, 5);

  bSizer->Add(new wxHyperlinkCtrl(this, wxID_ANY, "INESC P&D Brasil", "http://inescbrasil.org.br/") , 0, wxALL | wxALIGN_CENTER, 5);

  bSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 5);

  wxStaticText* txtPart = new wxStaticText(this, wxID_ANY, "Este trabalho foi desenvolvido no âmbito do projeto “PD-4380-0002/2015- ESTUDO E DESENVOLVIMENTO DE METODOLOGIAS DE INSPEÇÃO AUTÔNOMA EM LINHAS DE TRANSMISSÃO”, financiado pelas empresas do grupo TBE (Transmissoras Brasileiras de Energia), utilizando recursos do programa de P&&D regulado pela ANEEL (Agência Nacional de Energia Elétrica) e executado pela Universidade Federal de Juiz de Fora - UFJF");
  txtPart->Wrap(485);
  bSizer->Add(txtPart, 0, wxALL, 5);

  auto bSizerImages = new wxBoxSizer(wxHORIZONTAL);
  wxIcon aneelIcon;
  aneelIcon.LoadFile("ABOUT_ANEELPD",wxBITMAP_TYPE_ICO_RESOURCE, 220,120);
  wxIcon enteIcon;
  enteIcon.LoadFile("ABOUT_ENTE", wxBITMAP_TYPE_ICO_RESOURCE, 220, 76);
  bSizerImages->Add(new wxStaticBitmap(this, wxID_ANY, aneelIcon), 0, wxALL, 5);
  bSizerImages->Add(new wxStaticBitmap(this, wxID_ANY, enteIcon), 0, wxALL, 5);
  bSizer->Add(bSizerImages, 0, wxALL, 5);

  this->SetSizer(bSizer);
  this->Layout();
  this->Centre(wxBOTH);
}

AboutDialog::~AboutDialog()
{
}