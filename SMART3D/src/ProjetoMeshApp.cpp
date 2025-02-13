#include "ProjetoMeshApp.h"

#include <vtksys/SystemTools.hxx>

#include <wx/fs_zip.h>
#include <wx/timer.h>
#include <wx/stdpaths.h>
#include <wx/cshelp.h>
#include <wx/html/helpctrl.h>

#include "FrmPrincipal.h"

IMPLEMENT_APP(ProjetoMeshApp)

BEGIN_EVENT_TABLE(ProjetoMeshApp, wxApp)
EVT_TIMER(idTimer, ProjetoMeshApp::OnTimerTimeout)
EVT_LEFT_DCLICK(ProjetoMeshApp::OnLeftDClick)
END_EVENT_TABLE()

bool ProjetoMeshApp::OnInit() {
	wxInitAllImageHandlers();

	//Add the handler to open zip
	wxFileSystem::AddHandler(new wxZipFSHandler);
	wxHelpControllerHelpProvider* helpProvider = new wxHelpControllerHelpProvider();
	wxHelpProvider::Set(helpProvider);

	wxString title("SM.A.R.T. 3D");
	frmPrincipal = new FrmPrincipal(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600));

	std::string executionPath = vtksys::SystemTools::GetFilenamePath(wxStandardPaths::Get().GetExecutablePath().ToStdString());
	if (executionPath == "")
	{
		executionPath = ".";
	}
	helpProvider->SetHelpController(frmPrincipal->getHelpController());
	if (!frmPrincipal->getHelpController()->AddBook(wxFileName(executionPath + "/Help.zip")))
	{
		wxLogError("Cannot initialize the Help system.");
	}

	frmPrincipal->Show(true);
	frmPrincipal->DragAcceptFiles(true);
	this->SetTopWindow(frmPrincipal);
	//We need to use a timer to give some time to the VTK elements load//bool result = vtk_panel->IsShownOnScreen();
	timer = new wxTimer(this, idTimer);
	timer->StartOnce(1);
	return true;
}
void ProjetoMeshApp::OnTimerTimeout(wxTimerEvent& event)
{
	if (wxGetApp().argc >= 2)
	{
		wxArrayString paths;
		paths.push_back(wxGetApp().argv[1]);
		frmPrincipal->loadMesh(paths);
	}
}
int ProjetoMeshApp::OnExit() {
	return 0;
}

void ProjetoMeshApp::OnLeftDClick(wxMouseEvent & event)
{
	frmPrincipal->OnLeftDClick(event);
	event.Skip();
}
