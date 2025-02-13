#pragma once
#include <wx\dialog.h>
#include <vtkSmartPointer.h>

class vtkTransform;
class wxSpinCtrlDouble;

class TransformDialog : public wxDialog
{
public:
	TransformDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = "Transform", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(300, 220), long style = wxDEFAULT_DIALOG_STYLE);
	~TransformDialog();

	void OnOK(wxCommandEvent& WXUNUSED(event));
	void OnEnter(wxKeyEvent& event);
	void OnAlignWithTool(wxCommandEvent& WXUNUSED(event));

	vtkSmartPointer<vtkTransform> transform = nullptr;

	enum {
		idAlignWithTool
	};

private:
	DECLARE_EVENT_TABLE()

	wxSpinCtrlDouble* spinRotationX;
	wxSpinCtrlDouble* spinRotationY;
	wxSpinCtrlDouble* spinRotationZ;
	wxSpinCtrlDouble* spinTranslationX;
	wxSpinCtrlDouble* spinTranslationY;
	wxSpinCtrlDouble* spinTranslationZ;
	wxButton* btOK;
};