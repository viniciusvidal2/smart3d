#pragma once
#include <vtkSmartPointer.h>

class vtkRenderWindow;
class wxString;

class SnapshotTool
{
public:
	SnapshotTool() {};
	~SnapshotTool() {};

	void takeSnapshot(vtkSmartPointer<vtkRenderWindow> renderWindow, bool ShowDialog = false);

private:
	//Counter snapshot
	static int cont;
	//Magnification
	static int maginification;
	//Transparency
	static bool getAlpha;
	//Path used to save the snapshot
	static wxString path;

};