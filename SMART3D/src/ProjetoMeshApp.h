#pragma once

#ifdef WIN32
#include "wx/msw/winundef.h"
#endif

#include <wx\app.h>

class wxTimer;
class wxTimerEvent;
class FrmPrincipal;

class ProjetoMeshApp : public wxApp 
{
public:
	virtual bool OnInit();
	void OnTimerTimeout(wxTimerEvent& event);
	virtual int OnExit();
private:
	DECLARE_EVENT_TABLE()
	wxTimer* timer;
	FrmPrincipal* frmPrincipal;
	void OnLeftDClick(wxMouseEvent& event);
};

DECLARE_APP(ProjetoMeshApp)

enum
{
	idTimer
};
