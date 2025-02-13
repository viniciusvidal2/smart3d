#pragma once
#include <wx\dialog.h>

class AboutDialog : public wxDialog
{
public:
  AboutDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = "About",
	  const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(555,400), long style = wxDEFAULT_DIALOG_STYLE);
  ~AboutDialog();
};