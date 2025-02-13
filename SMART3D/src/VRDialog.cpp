#include "VRDialog.h"

#include <vtkNew.h>
#include <vtkCamera.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkExtractSelectedIds.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkOpenVRCamera.h>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/timer.h>
#include <wx/msgdlg.h>
#include <wx/log.h>
#include <wx/thread.h>

#include "InteractorStyleVR.h"
#include "Utils.h"
#include "Mesh.h"


class ThreadVR : public wxThread
{
public:
	ThreadVR(VRDialog *handler)
		: wxThread(wxTHREAD_DETACHED)
	{
		m_pHandler = handler;
	}
	~ThreadVR();
	void setMesh(Mesh* mesh);
	void stopVR();

protected:
	virtual ExitCode Entry();
	VRDialog *m_pHandler;
	Mesh* mesh = NULL;
	vtkNew<InteractorStyleVR> iterStyle;
};


wxBEGIN_EVENT_TABLE(VRDialog, wxDialog)
EVT_BUTTON(idBtStopVR, VRDialog::OnBtStopVR)
EVT_CLOSE(VRDialog::OnClose)
EVT_TIMER(idTimer, VRDialog::OnTimerTimeout)
wxEND_EVENT_TABLE()

VRDialog::VRDialog(wxWindow * parent, Mesh* mesh, wxWindowID id, const wxString & title, const wxPoint & pos, const wxSize & size, long style) : wxDialog(parent, id, title, pos, size, style)
{
  this->SetSizeHints(wxDefaultSize, wxDefaultSize);
  wxBoxSizer* bSizer;
  bSizer = new wxBoxSizer(wxVERTICAL);

  bSizer->Add(new wxStaticText(this, wxID_ANY, "It is not possible to use the visualization area. Please put the VR oculus."), 0, wxALL | wxALIGN_CENTER, 5);
  bSizer->Add(new wxButton(this, idBtStopVR, "Stop VR"), 0, wxALL | wxALIGN_CENTER, 5);

  this->SetSizer(bSizer);
  this->Layout();
  this->Centre(wxBOTH);

  setMesh(mesh);
  DoStartThread();
}

VRDialog::~VRDialog()
{
  meshOriginal = NULL;
  meshCopy = NULL;
}

void VRDialog::setMesh(Mesh * mesh)
{
  this->meshOriginal = mesh;
  meshCopy = new Mesh();
  for (size_t i = 0; i < mesh->actors.size(); i++)
  {
    meshCopy->actors.push_back(Utils::duplicateActor(mesh->actors.at(i)));
  }
  //Calculate the polydata to avoid lag in the VR
  vtkSmartPointer<vtkPolyData> polyData = meshCopy->getPolyData();
  //If it is a point cloud we need to use this filter to make sure the picking will work
  if (polyData->GetPolys()->GetNumberOfCells() == 0)
  {
    vtkSmartPointer<vtkPoints> points = polyData->GetPoints();
	vtkIdType numberOfPoints = points->GetNumberOfPoints();
    vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
    ids->SetNumberOfComponents(1);
    ids->SetNumberOfValues(numberOfPoints);
    // Set values
    for (vtkIdType i = 0; i < numberOfPoints; i++)
    {
      ids->SetValue(i, i);
    }
    vtkSmartPointer<vtkSelectionNode> selectionNode = vtkSmartPointer<vtkSelectionNode>::New();
    selectionNode->SetFieldType(vtkSelectionNode::POINT);
    selectionNode->SetContentType(vtkSelectionNode::INDICES);
    selectionNode->SetSelectionList(ids);

    vtkSmartPointer<vtkSelection> selection = vtkSmartPointer<vtkSelection>::New();
    selection->AddNode(selectionNode);
    
    vtkSmartPointer<vtkExtractSelectedIds> extractSelectedIds = vtkSmartPointer<vtkExtractSelectedIds>::New();
    extractSelectedIds->SetInputData(0, polyData);
    extractSelectedIds->SetInputData(1, selection);
    extractSelectedIds->Update();

    vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
    surfaceFilter->SetInputConnection(extractSelectedIds->GetOutputPort());
    surfaceFilter->Update();

    vtkSmartPointer<vtkPolyData> poly = surfaceFilter->GetOutput();
    meshCopy->updatePoints(poly);
  }
  meshCopy->setCalibration(mesh->getCalibration());
}

void VRDialog::OnClose(wxCloseEvent & event)
{
  if (qtdClosePressed > 0 && timer == NULL)
  {
    wxDialog::EndModal(wxID_CANCEL);
  }
  qtdClosePressed++;
  OnBtStopVR((wxCommandEvent)NULL);
}

void VRDialog::OnTimerTimeout(wxTimerEvent & event)
{
  if (errorVR)
  {
    wxDialog::EndModal(wxID_CANCEL);
  }
  else
  {
    int answer = wxMessageBox("You want to keep the cahnges made in VR?", "Question", wxYES_NO | wxICON_QUESTION, this);
    if (answer == wxYES && meshCopy != NULL)
    {
      meshOriginal->updateActors(meshCopy->actors);
    }
    wxDialog::EndModal(wxID_CANCEL);
  }
}

void VRDialog::OnThreadCompletion(bool error)
{
  errorVR = error;
  timer = new wxTimer(this, idTimer);
  timer->StartOnce(1000);
}

void VRDialog::setCamera(vtkSmartPointer<vtkCamera> camera)
{
  camera2D = camera;
}

void VRDialog::OnBtStopVR(wxCommandEvent & WXUNUSED)
{
  {
    wxCriticalSectionLocker enter(m_pThreadCS);
    if (m_pThread)         // does the thread still exist?
    {
      m_pThread->stopVR();
      if (m_pThread->Delete() != wxTHREAD_NO_ERROR)
        wxLogError("Can't delete the thread!");
    }
  }       // exit from the critical section to give the thread
          // the possibility to enter its destructor
          // (which is guarded with m_pThreadCS critical section!)
  while (1)
  {
    { // was the ~MyThread() function executed?
      wxCriticalSectionLocker enter(m_pThreadCS);
      if (!m_pThread) break;
    }
    // wait for thread completion
    wxThread::This()->Sleep(1);
  }
}




//*****************************************************

void VRDialog::DoStartThread()
{
  m_pThread = new ThreadVR(this);
  m_pThread->setMesh(meshCopy);
  if (m_pThread->Run() != wxTHREAD_NO_ERROR)
  {
    wxLogError("Can't create the thread!");
    delete m_pThread;
    m_pThread = NULL;
  }
}

void ThreadVR::setMesh(Mesh * mesh)
{
  this->mesh = mesh;
}

void ThreadVR::stopVR()
{
  iterStyle->GetInteractor()->ExitCallback();
}

wxThread::ExitCode ThreadVR::Entry()
{
  if (mesh == NULL)
  {
    m_pHandler->OnThreadCompletion(true);
    return (wxThread::ExitCode)1;
  }
  vtkNew<vtkOpenVRRenderer> renderer;
  vtkNew<vtkOpenVRRenderWindow> renderWindow;
  renderWindow->Start();
  if (renderWindow->GetHMD() == NULL)
  {
    wxMessageBox("Please connect the VR to the computer", "Error", wxICON_ERROR);
    m_pHandler->OnThreadCompletion(true);
    return (wxThread::ExitCode)1;
  }
  
  vtkNew<vtkOpenVRRenderWindowInteractor> iren;
  iren->SetInteractorStyle(iterStyle);
  iterStyle->setMesh(mesh);


  renderWindow->AddRenderer(renderer.Get());

  //Add actors
  for (size_t i = 0; i < mesh->actors.size(); i++)
  {
    renderer->AddActor(mesh->actors.at(i));
  }

  iren->SetRenderWindow(renderWindow.Get());
  vtkNew<vtkOpenVRCamera> cam;
  renderer->SetActiveCamera(cam.Get());
  // Without the next line volume rendering in VR does not work
  renderWindow->SetMultiSamples(0);

  // Render
  renderer->ResetCamera();

  vtkNew<vtkCamera> zcam;
  double fp[3];
  m_pHandler->camera2D->GetFocalPoint(fp);
  zcam->SetFocalPoint(fp);
  zcam->SetViewAngle(m_pHandler->camera2D->GetViewAngle());
  zcam->SetViewUp(0.0, 0.0, 1.0);
  double distance = m_pHandler->camera2D->GetDistance();
  zcam->SetPosition(fp[0] + distance, fp[1], fp[2]);
  renderWindow->InitializeViewFromCamera(zcam.Get());
  renderWindow->Render();
  renderer->ResetCamera();
  renderer->ResetCameraClippingRange();

  iren->Start();

  m_pHandler->OnThreadCompletion(false);
  return (wxThread::ExitCode)0;     // success
}

ThreadVR::~ThreadVR()
{
  wxCriticalSectionLocker enter(m_pHandler->m_pThreadCS);
  // the thread is being destroyed; make sure not to leave dangling pointers around
  m_pHandler->m_pThread = NULL;
}


