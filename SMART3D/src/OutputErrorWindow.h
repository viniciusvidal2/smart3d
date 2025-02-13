#pragma once
#include <vtkOutputWindow.h>

class OutputErrorWindow : public vtkOutputWindow
{
public:
	vtkTypeMacro(OutputErrorWindow, vtkOutputWindow);

	static OutputErrorWindow * New();
	virtual void DisplayText(const char*);
	virtual void DisplayErrorText(const char*);
	virtual void DisplayWarningText(const char*);
	virtual void DisplayGenericWarningText(const char*);
	//Test if there is some error or warning message 1 - OK 0 - ERROR
	static bool checkOutputErrorWindow();
	static bool error;
	static bool warning;
	static bool suppressMessages;
	/*
	Clear error and warning flags
	*/
	static void clearFlags()
	{
		error = 0;
		warning = 0;
	};
	//Change to show or not the logs
	static void setSuppressMessages(bool suppress) { suppressMessages = suppress; };

protected:
	OutputErrorWindow();
	virtual ~OutputErrorWindow();
private:
	OutputErrorWindow(const OutputErrorWindow &);  // Not implemented.
	void operator=(const OutputErrorWindow &);  // Not implemented.
};