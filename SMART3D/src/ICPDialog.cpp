#include "ICPDialog.h"

#include <sstream>

#include <vtkTransform.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>

#include "ICP.h"
#include "Mesh.h"

wxBEGIN_EVENT_TABLE(ICPDialog, wxDialog)
	EVT_CHECKBOX(idCkUseMeanDistance, ICPDialog::OnCkUseMeanDistance)
	EVT_BUTTON(wxID_OK, ICPDialog::OnOK)
	EVT_BUTTON(wxID_CANCEL, ICPDialog::OnCancel)
	EVT_BUTTON(idBtDefault, ICPDialog::OnBtDefault)
	EVT_BUTTON(idBtStartThread, ICPDialog::OnBtStartThread)
	EVT_BUTTON(idBtAbortThread, ICPDialog::OnBtAbortThread)
wxEND_EVENT_TABLE()

ICPDialog::ICPDialog(wxWindow * parent, wxWindowID id, const wxString & title, const wxPoint & pos, const wxSize & size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	//Used to get the default options
	ICP icp;

	wxBoxSizer* bSizer = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* fgSizer = new wxFlexGridSizer(7, 2, 0, 0);
	fgSizer->SetFlexibleDirection(wxBOTH);
	fgSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Maximum number of iterations"), 0, wxALL, 5);
	spinMaxNumberOfIterations = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 16384L, 1, 1000000, icp.getMaxNumberOfIterations());
	fgSizer->Add(spinMaxNumberOfIterations, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Maximum number of landmarks"), 0, wxALL, 5);
	spinMaxNumberOfLandmarks = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 16384L, 4, 1000000, icp.getMaxNumberOfLandmarks());
	fgSizer->Add(spinMaxNumberOfLandmarks, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Maximum mean distance"), 0, wxALL, 5);
	spinMaxMeanDistance = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 16384L, 0.0000001, 100.0, icp.getMaxMeanDistance());
	spinMaxMeanDistance->Enable(icp.getUseMeanDistance());
	fgSizer->Add(spinMaxMeanDistance, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Maximum closest point distance"), 0, wxALL, 5);
	spinClosestPointMaxMeanDistance = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 16384L, 0.0000001, 100.0, icp.getClosestPointMaxDistance());
	fgSizer->Add(spinClosestPointMaxMeanDistance, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Use mean distance"), 0, wxALL, 5);
	ckUseMeanDistance = new wxCheckBox(this, idCkUseMeanDistance, wxEmptyString);
	ckUseMeanDistance->SetValue(icp.getUseMeanDistance());
	fgSizer->Add(ckUseMeanDistance, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Start matching centroids"), 0, wxALL, 5);
	ckStartMatchingCentroids = new wxCheckBox(this, wxID_ANY, wxEmptyString);
	ckStartMatchingCentroids->SetValue(icp.getStartByMatchingCentroids());
	fgSizer->Add(ckStartMatchingCentroids, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Delimit by minimum bouding box"), 0, wxALL, 5);
	ckDelimitByMinimumBoundingBox = new wxCheckBox(this, wxID_ANY, wxEmptyString);
	fgSizer->Add(ckDelimitByMinimumBoundingBox, 0, wxALL, 5);
	
	bSizer->Add(fgSizer, 0, wxEXPAND, 5);

	wxBoxSizer* bSizerBtsThread = new wxBoxSizer(wxHORIZONTAL);

	statusThread = new wxStaticText(this, wxID_ANY, "Thread: stopped");

	bSizerBtsThread->Add(statusThread, 0, wxALL, 5);

	bSizerBtsThread->Add(new wxButton(this, idBtStartThread, "Start ICP"), 0, wxALL, 5);

	bSizerBtsThread->Add(new wxButton(this, idBtAbortThread, "Abort ICP"), 0, wxALL, 5);

	bSizer->Add(bSizerBtsThread, 0, wxALIGN_RIGHT, 5);

	wxBoxSizer* bSizerBts = new wxBoxSizer(wxHORIZONTAL);

	bSizerBts->Add(new wxButton(this, idBtDefault, "Default"), 0, wxALL, 5);

	bSizerBts->Add(new wxButton(this, wxID_OK, "Apply transform"), 0, wxALL, 5);

	bSizerBts->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);

	bSizer->Add(bSizerBts, 0, wxALIGN_RIGHT, 5);
	
	this->SetSizer(bSizer);
	this->Layout();

	this->Centre(wxBOTH);
}

ICPDialog::~ICPDialog()
{
	meshSource = nullptr;
	meshTarget = nullptr;
	if (threadICP)
	{
		delete threadICP;
		threadICP = nullptr;
	}
}

void ICPDialog::OnCkUseMeanDistance(wxCommandEvent & WXUNUSED)
{
	spinMaxMeanDistance->Enable(ckUseMeanDistance->GetValue());
}

void ICPDialog::OnOK(wxCommandEvent & WXUNUSED)
{
	if (!T)
	{
		wxMessageBox("You need to start the ICP first.", "Error", wxICON_ERROR);
		return;
	}
	OnBtAbortThread((wxCommandEvent)NULL);
	EndModal(wxID_OK);
}

void ICPDialog::OnCancel(wxCommandEvent & WXUNUSED)
{
	OnBtAbortThread((wxCommandEvent) NULL);
	EndModal(wxID_CANCEL);
}

void ICPDialog::OnBtDefault(wxCommandEvent & WXUNUSED)
{
	ICP icp;
	spinMaxNumberOfIterations->SetValue(icp.getMaxNumberOfIterations());
	spinMaxNumberOfLandmarks->SetValue(icp.getMaxNumberOfLandmarks());
	spinMaxMeanDistance->SetValue(icp.getMaxMeanDistance());
	spinClosestPointMaxMeanDistance->SetValue(icp.getClosestPointMaxDistance());
	ckUseMeanDistance->SetValue(icp.getUseMeanDistance());
	ckStartMatchingCentroids->SetValue(icp.getStartByMatchingCentroids());
}

void ICPDialog::OnBtStartThread(wxCommandEvent & WXUNUSED)
{
	if (!threadICP)
	{
		threadICP = new std::thread(&ICPDialog::executeICP, this);
	}
	statusThread->SetLabel("Thread: running");
}

void ICPDialog::OnBtAbortThread(wxCommandEvent & WXUNUSED)
{
	if (!threadICP)
	{
		return;
	}
	DWORD exitCode;
	DWORD result = GetExitCodeThread(threadICP->native_handle(), &exitCode);
	if (!result)
	{
		wxMessageBox(wxString("Failed to get exit code: failure resson " + GetLastError()));
		return;
	}
	if (exitCode == STILL_ACTIVE)
	{
		result = SuspendThread(threadICP->native_handle());
		if (result == -1)
		{
			wxMessageBox(wxString("Failed to suspend thread: failure resson " + GetLastError()));
		}
	}
	result = TerminateThread(threadICP->native_handle(), 1);
	if (!result)
	{
		wxMessageBox(wxString("Failed to terminate thread: failure resson " + GetLastError()));
	}
	statusThread->SetLabel("Thread: stopped");
	threadICP->detach();
	delete threadICP;
	threadICP = nullptr;
}

void ICPDialog::executeICP()
{
	ICP icp;
	//Parameters
	icp.setMaxNumberOfIterations(spinMaxNumberOfIterations->GetValue());
	icp.setMaxNumberOfLandmarks(spinMaxNumberOfLandmarks->GetValue());
	icp.setMaxMeanDistance(spinMaxMeanDistance->GetValue());
	icp.setClosestPointMaxDistance(spinClosestPointMaxMeanDistance->GetValue());
	icp.setStartByMatchingCentroids(ckStartMatchingCentroids->GetValue());
	icp.setUseMeanDistance(ckUseMeanDistance->GetValue());
	//Inputs
	icp.setSourcePoints(meshSource->getPolyData()->GetPoints());
	icp.setTargetPoints(meshTarget->getPolyData()->GetPoints());
	//Process
	if (ckDelimitByMinimumBoundingBox->GetValue())
	{
		icp.delimitByMinimumBoudingBox();
	}
	icp.executeICP();
	if (!T)
	{
		T = vtkSmartPointer<vtkTransform>::New();
	}
	T = icp.getTransform();
	statusThread->SetLabel("Thread: stopped");
	if(ckUseMeanDistance->GetValue())
	{
		wxMessageBox("ICP finished, mean distance: " + std::to_string(icp.getMeanDistance()) + ".", "Information", wxICON_INFORMATION);
	}
	std::stringstream s;
	s << "Transformation matrix:\n";
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			s << std::fixed << std::setprecision(7) << T->GetMatrix()->GetElement(i, j) << " ";
		}
		s << "\n";
	}
	wxMessageBox(s.str(), "Information", wxICON_INFORMATION);
}
