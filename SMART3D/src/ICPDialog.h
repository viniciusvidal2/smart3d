#pragma once
#include <wx/dialog.h>
#include <vtkSmartPointer.h>
#include <thread>

class vtkTransform;
class wxSpinCtrlDouble;
class wxCheckBox;
class wxStaticText;
class Mesh;

class ICPDialog : public wxDialog
{
public:
	ICPDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = "ICP", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(355, 320), long style = wxDEFAULT_DIALOG_STYLE);
	~ICPDialog();

	void setSourceMesh(Mesh* mesh) { meshSource = mesh; };
	void setTargetMesh(Mesh* mesh) { meshTarget = mesh; };
	vtkSmartPointer<vtkTransform> getTransform() const { return T; };

private:
	DECLARE_EVENT_TABLE()

	void OnCkUseMeanDistance(wxCommandEvent& WXUNUSED(event));
	void OnOK(wxCommandEvent& WXUNUSED(event));
	void OnCancel(wxCommandEvent& WXUNUSED(event));
	void OnBtDefault(wxCommandEvent& WXUNUSED(event));
	void OnBtStartThread(wxCommandEvent& WXUNUSED(event));
	void OnBtAbortThread(wxCommandEvent& WXUNUSED(event));
	void executeICP();
	
	wxSpinCtrl* spinMaxNumberOfIterations;
	wxSpinCtrl* spinMaxNumberOfLandmarks;
	wxSpinCtrlDouble* spinMaxMeanDistance;
	wxSpinCtrlDouble* spinClosestPointMaxMeanDistance;
	wxCheckBox* ckUseMeanDistance;
	wxCheckBox* ckStartMatchingCentroids;
	wxCheckBox* ckDelimitByMinimumBoundingBox;
	wxStaticText* statusThread;

	vtkSmartPointer<vtkTransform> T = nullptr;

	Mesh* meshSource = nullptr;
	Mesh* meshTarget = nullptr;
	std::thread* threadICP = nullptr;
};

enum EnumICPDialog
{
	idBtDefault,
	idBtStartThread,
	idBtAbortThread,
	idCkUseMeanDistance
};