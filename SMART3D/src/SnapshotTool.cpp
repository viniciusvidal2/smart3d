#include "SnapshotTool.h"

#include <vtkRenderWindow.h>

#include "SnapshotDialog.h"
#include "Utils.h"

int SnapshotTool::cont = 0;
//Magnification
int SnapshotTool::maginification = -1;
//Transparency
bool SnapshotTool::getAlpha = false;
//Path used to save the snapshot
wxString SnapshotTool::path = "";

void SnapshotTool::takeSnapshot(vtkSmartPointer<vtkRenderWindow> renderWindow, bool ShowDialog)
{
  if (ShowDialog || maginification == -1)
  {
    SnapshotDialog* snapDialog = new SnapshotDialog(NULL, wxID_ANY, "Snapshot", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, renderWindow->GetSize());
    if (snapDialog->ShowModal() == wxID_CANCEL)
    {
      cont = 0;
      maginification = -1;
      path = "";
      delete snapDialog;
      return;
    }
    if (snapDialog->getPath() != path)
    {
      cont = -1;
    }
    maginification = snapDialog->getMagnification();
    getAlpha = snapDialog->getAlpha();
    path = snapDialog->getPath();
    delete snapDialog;
  }
  cont++;
  wxString newPath = path;
  newPath.erase(newPath.size() - 4, 4);
  newPath << "_" << cont << ".png";
  Utils::takeSnapshot(newPath.ToStdString(), maginification, getAlpha, renderWindow);
}
