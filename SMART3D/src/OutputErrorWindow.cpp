#include "OutputErrorWindow.h"
#include <vtkObjectFactory.h>

#include <wx/log.h>

vtkStandardNewMacro(OutputErrorWindow);

bool OutputErrorWindow::error = false;
bool OutputErrorWindow::warning = false;
bool OutputErrorWindow::suppressMessages = false;

OutputErrorWindow::OutputErrorWindow()
{

}
OutputErrorWindow::~OutputErrorWindow()
{
}
void OutputErrorWindow::DisplayText(const char* text)
{
	if (!text)
	{
		return;
	}
	if (!suppressMessages)
	{
		wxLogMessage(text);
	}
}

void OutputErrorWindow::DisplayErrorText(const char * text)
{
	if (!text)
	{
		return;
	}
	error = 1;
	if (!suppressMessages)
	{
		wxLogError(text);
	}
}

void OutputErrorWindow::DisplayWarningText(const char * text)
{
	if (!text)
	{
		return;
	}
	warning = 1;
	if (!suppressMessages)
	{
		wxLogWarning(text);
	}
}

void OutputErrorWindow::DisplayGenericWarningText(const char * text)
{
	if (!text)
	{
		return;
	}
	warning = 1;
	if (!suppressMessages)
	{
		wxLogWarning(text);
	}
}

bool OutputErrorWindow::checkOutputErrorWindow()
{
	if (error)
	{
		clearFlags();
		return 0;
	}
	else if (warning)
	{
		clearFlags();
	}
	return 1;
}
