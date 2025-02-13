#pragma once
#include <wx/dialog.h>
#include <thread>

class wxSpinCtrlDouble;
class wxFilePickerCtrl;
class wxStaticText;

class M3C2Dialog : public wxDialog
{
public:
	M3C2Dialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = "M3C2",
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(420, 480), long style = wxDEFAULT_DIALOG_STYLE);
	~M3C2Dialog();

private:
	DECLARE_EVENT_TABLE()

	void OnOK(wxCommandEvent& WXUNUSED(event));
	void OnCancel(wxCommandEvent& WXUNUSED(event));
	void OnBtDefault(wxCommandEvent& WXUNUSED(event));
	void OnBtStartThread(wxCommandEvent& WXUNUSED(event));
	void OnBtAbortThread(wxCommandEvent& WXUNUSED(event));
	void executeM3C2();
	
	wxSpinCtrlDouble* spinNormalScale;
	wxSpinCtrlDouble* spinCylinderBase;
	wxSpinCtrlDouble* spinCylinderLength;
	wxFilePickerCtrl* fpP1;
	wxFilePickerCtrl* fpP2;
	wxFilePickerCtrl* fpResult;
	wxFilePickerCtrl* fpCores;
	wxFilePickerCtrl* fpExtpts;
	wxFilePickerCtrl* fpP1Reduced;
	wxFilePickerCtrl* fpP2Reduced;
	wxStaticText* statusThread;

	std::thread* thread;
};