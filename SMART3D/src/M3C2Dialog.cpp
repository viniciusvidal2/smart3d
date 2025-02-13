#include "M3C2Dialog.h"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/spinctrl.h>
#include <wx/filepicker.h>
#include <wx/msgdlg.h>
#include <wx/log.h>

#include "MeshIO.h"
#include "Utils.h"
#include "m3c2.h"


enum EnumM3C2Dialog
{
	idBtDefault,
	idBtStartThread,
	idBtAbortThread,
	idCkUseMeanDistance
};

wxBEGIN_EVENT_TABLE(M3C2Dialog, wxDialog)
EVT_BUTTON(wxID_OK, M3C2Dialog::OnOK)
EVT_BUTTON(wxID_CANCEL, M3C2Dialog::OnCancel)
EVT_BUTTON(idBtDefault, M3C2Dialog::OnBtDefault)
EVT_BUTTON(idBtStartThread, M3C2Dialog::OnBtStartThread)
EVT_BUTTON(idBtAbortThread, M3C2Dialog::OnBtAbortThread)
wxEND_EVENT_TABLE()

M3C2Dialog::M3C2Dialog(wxWindow * parent, wxWindowID id, const wxString & title,
	const wxPoint & pos, const wxSize & size, long style) : wxDialog(parent, id, title, pos, size, style), thread(nullptr)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* fgSizer = new wxFlexGridSizer(10, 2, 0, 0);
	fgSizer->SetFlexibleDirection(wxBOTH);
	fgSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Normal scale"), 0, wxALL, 5);
	spinNormalScale = new wxSpinCtrlDouble(this);
	spinNormalScale->SetValue(0.4);
	fgSizer->Add(spinNormalScale, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Cylinder base (Optional)"), 0, wxALL, 5);
	spinCylinderBase = new wxSpinCtrlDouble(this);
	spinCylinderBase->SetValue(0.04);
	fgSizer->Add(spinCylinderBase, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Cylinder length (Optional)"), 0, wxALL, 5);
	spinCylinderLength = new wxSpinCtrlDouble(this);
	spinCylinderLength->SetValue(0.8);
	fgSizer->Add(spinCylinderLength, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Point cloud 1"), 0, wxALL, 5);
	fpP1 = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString,
		"Select the point cloud 1 file", "PLY files(*.ply) | *.ply",
		wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL | wxFLP_OPEN);
	fgSizer->Add(fpP1, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Point cloud 2"), 0, wxALL, 5);
	fpP2 = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString,
		"Select the point cloud 2 file", "PLY files(*.ply) | *.ply",
		wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL | wxFLP_OPEN);
	fgSizer->Add(fpP2, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Result"), 0, wxALL, 5);
	fpResult = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString,
		"Select the result file", "PLY files(*.ply) | *.ply",
		wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL | wxFLP_SAVE | wxFLP_OVERWRITE_PROMPT);
	fgSizer->Add(fpResult, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Point cloud cores (Optional)"), 0, wxALL, 5);
	fpCores = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString,
		"Select the cores point cloud file", "PLY files(*.ply) | *.ply",
		wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL | wxFLP_OPEN);
	fgSizer->Add(fpCores, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Point cloud exterior (Optional)"), 0, wxALL, 5);
	fpExtpts = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString,
		"Select the exterior point cloud file", "PLY files(*.ply) | *.ply",
		wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL | wxFLP_OPEN);
	fgSizer->Add(fpExtpts, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Point cloud 1 reduced (Optional)"), 0, wxALL, 5);
	fpP1Reduced = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString,
		"Select the reduced point cloud 1 file", "PLY files(*.ply) | *.ply",
		wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL | wxFLP_OPEN);
	fgSizer->Add(fpP1Reduced, 0, wxALL, 5);

	fgSizer->Add(new wxStaticText(this, wxID_ANY, "Point cloud 2 reduced (Optional)"), 0, wxALL, 5);
	fpP2Reduced = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString,
		"Select the reduced point cloud 2 file", "PLY files(*.ply) | *.ply",
		wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL | wxFLP_OPEN);
	fgSizer->Add(fpP2Reduced, 0, wxALL, 5);
	
	bSizer->Add(fgSizer, 0, wxEXPAND, 5);

	wxBoxSizer* bSizerBtsThread = new wxBoxSizer(wxHORIZONTAL);

	statusThread = new wxStaticText(this, wxID_ANY, "Thread: stopped");

	bSizerBtsThread->Add(statusThread, 0, wxALL, 5);

	bSizerBtsThread->Add(new wxButton(this, idBtStartThread, "Start M3C2"), 0, wxALL, 5);

	bSizerBtsThread->Add(new wxButton(this, idBtAbortThread, "Abort M3C2"), 0, wxALL, 5);

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

M3C2Dialog::~M3C2Dialog()
{
	if (thread)
	{
		delete thread;
		thread = nullptr;
	}
}

void M3C2Dialog::OnOK(wxCommandEvent & WXUNUSED)
{
	OnBtAbortThread((wxCommandEvent)NULL);
	EndModal(wxID_OK);
}

void M3C2Dialog::OnCancel(wxCommandEvent & WXUNUSED)
{
	OnBtAbortThread((wxCommandEvent) NULL);
	EndModal(wxID_CANCEL);
}

void M3C2Dialog::OnBtDefault(wxCommandEvent & WXUNUSED)
{
	spinNormalScale->SetValue(0.4);
	spinCylinderBase->SetValue(0.04);
	spinCylinderLength->SetValue(0.8);
}

void createCloud(const std::string& filename, M3C2::Cloud& cloud)
{
	vtkSmartPointer<vtkPolyData> polyData;
	MeshIO::getPolyDataFromFile(filename, polyData);
	vtkSmartPointer<vtkPoints> points = polyData->GetPoints();
	const int numberOfPoints = points->GetNumberOfPoints();
	cloud.points.reserve(numberOfPoints);
#pragma omp for
	for (int i = 0; i < numberOfPoints; i++)
	{
		double pAux[3];
		points->GetPoint(i, pAux);
		cloud.points.emplace_back(pAux[0], pAux[1], pAux[2]);
	}
}

bool saveCloud(const std::string& filename, const M3C2::Cloud& cloud)
{
	auto numberOfElements = cloud.points.size();
	auto numberOfElements1 = cloud.normals.size();
	auto numberOfElements2 = cloud.diff.size();
	if (numberOfElements == 0)
	{
		return 0;
	}
	if (numberOfElements != numberOfElements1 ||
		numberOfElements != numberOfElements2)
	{
		return 0;
	}
	//Write the ply file
	std::ofstream outputFile(filename, std::ios::binary);
	if (!outputFile.is_open())
	{
		return 0;
	}
	outputFile << "ply\n";
	outputFile << "format binary_little_endian 1.0\n";
	outputFile << "element vertex " << numberOfElements << "\n";
	outputFile << "property float x\n";
	outputFile << "property float y\n";
	outputFile << "property float z\n";
	outputFile << "property float nx\n";
	outputFile << "property float ny\n";
	outputFile << "property float nz\n";
	outputFile << "property float diff\n";
	outputFile << "end_header\n";
	if (outputFile.is_open())
	{
		for (size_t i = 0; i < numberOfElements; i++)
		{
			Utils::writeBin<float>(outputFile, static_cast<float>(cloud.points[i].x));
			Utils::writeBin<float>(outputFile, static_cast<float>(cloud.points[i].y));
			Utils::writeBin<float>(outputFile, static_cast<float>(cloud.points[i].z));
			//Normals
			Utils::writeBin<float>(outputFile, static_cast<float>(cloud.normals[i].x));
			Utils::writeBin<float>(outputFile, static_cast<float>(cloud.normals[i].y));
			Utils::writeBin<float>(outputFile, static_cast<float>(cloud.normals[i].z));
			//Diff
			Utils::writeBin<float>(outputFile, static_cast<float>(cloud.diff[i]));
		}
	}
	outputFile.close();
	return 1;
}

void M3C2Dialog::executeM3C2()
{
	M3C2::Cloud p1, p2, cores, extpts, p1Reduced, p2Reduced;
	createCloud(fpP1->GetPath().ToStdString(), p1);
	createCloud(fpP2->GetPath().ToStdString(), p2);
	if (fpCores->GetPath() != "")
	{
		createCloud(fpCores->GetPath().ToStdString(), cores);
	}
	else
	{
		cores = p1;
	}
	if (fpExtpts->GetPath() != "")
	{
		createCloud(fpExtpts->GetPath().ToStdString(), extpts);
	}
	else
	{
		//implemt the Z shift
		extpts.points.reserve(p1.points.size());
		for (const auto& p : p1.points)
		{
			extpts.points.emplace_back(p.x, p.y, p.z + 1);
		}
	}
	M3C2::Options opt;
	opt.scales.emplace_back(spinNormalScale->GetValue());
	opt.cylinder_base = spinCylinderBase->GetValue();
	opt.cylinder_length = spinCylinderLength->GetValue();
	opt.result_filename = fpResult->GetPath();
	M3C2::Cloud result;
	M3C2::M3C2 m3c2;
	m3c2.compute(p1, p1Reduced, p2, p2Reduced, cores, extpts, result, opt);
	if (!saveCloud(opt.result_filename, result))
	{
		wxMessageBox("Could not save the cloud", "Error", wxICON_ERROR);
	}
	statusThread->SetLabel("Thread: stopped");
}

void M3C2Dialog::OnBtStartThread(wxCommandEvent & WXUNUSED)
{
	if (fpP1->GetPath() == "" || fpP2->GetPath() == "" || fpResult->GetPath() == "")
	{
		wxLogError("Please fulfill all the fields correctly.");
		return;
	}
	if (!thread)
	{
		thread = new std::thread(&M3C2Dialog::executeM3C2, this);
	}
	statusThread->SetLabel("Thread: running");
}

void M3C2Dialog::OnBtAbortThread(wxCommandEvent & WXUNUSED)
{
	if (!thread)
	{
		return;
	}
	DWORD exitCode;
	DWORD result = GetExitCodeThread(thread->native_handle(), &exitCode);
	if (!result)
	{
		wxMessageBox(wxString("Failed to get exit code: failure resson " + GetLastError()));
		return;
	}
	if (exitCode == STILL_ACTIVE)
	{
		result = SuspendThread(thread->native_handle());
		if (result == -1)
		{
			wxMessageBox(wxString("Failed to suspend thread: failure resson " + GetLastError()));
		}
	}
	result = TerminateThread(thread->native_handle(), 1);
	if (!result)
	{
		wxMessageBox(wxString("Failed to terminate thread: failure resson " + GetLastError()));
	}
	statusThread->SetLabel("Thread: stopped");
	thread->detach();
	delete thread;
	thread = nullptr;
}
