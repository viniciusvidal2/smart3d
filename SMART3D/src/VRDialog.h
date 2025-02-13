#pragma once
#include <vtkSmartPointer.h>
#include <wx/dialog.h>

class vtkCamera;
class wxTimer;
class wxTimerEvent;
class InteractorStyleVR;
class Mesh;
class ThreadVR;

class VRDialog : public wxDialog
{
public:
  VRDialog(wxWindow* parent, Mesh* mesh, wxWindowID id = wxID_ANY, const wxString& title = "Virtual reality", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(400, 100), long style = wxDEFAULT_DIALOG_STYLE);
  ~VRDialog();

  void setCamera(vtkSmartPointer<vtkCamera> camera);

  //******************
  ThreadVR* m_pThread;
  wxCriticalSection m_pThreadCS;
  void DoStartThread();
  void OnThreadCompletion(bool error);
  vtkSmartPointer<vtkCamera> camera2D;

private:
  DECLARE_EVENT_TABLE()

  void setMesh(Mesh* mesh);
  void OnClose(wxCloseEvent& event);

  wxTimer* timer = NULL;
  void OnTimerTimeout(wxTimerEvent& event);

  enum {
    idBtStopVR,
    idTimer
  };

  
  void OnBtStopVR(wxCommandEvent& WXUNUSED(event));

  Mesh* meshOriginal = NULL;
  Mesh* meshCopy = NULL;
  int qtdClosePressed = 0;
  bool errorVR = false;

};