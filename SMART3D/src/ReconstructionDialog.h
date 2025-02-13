#pragma once
#include <wx\dialog.h>

class wxFileDirPickerEvent;
struct MeshAux;
struct CameraParameters;
class Camera;
class wxCheckBox;
class wxStaticText;
class wxDirPickerCtrl;
class wxFilePickerCtrl;

struct BatchItem
{
	bool generateMesh = false;
	bool generateTexture = false;
	wxString imagePath = "";
	wxString meshPath = "";
	wxString nvmPath = "";
	wxString outputPath = "";
};

class ReconstructionDialog : public wxDialog
{
public:
  ReconstructionDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = "Reconstruction/Texturization", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
  ~ReconstructionDialog();

  wxString getOutputPath();

  //Method used to texturize a mesh without opening the dialog, it will use the last set parameters in the config dialog
  void texturizeMesh(std::string meshPath, std::string nvmPath, std::string outputPath);

  bool runAVTexturization(std::string meshPath, std::string nvmPath, std::string outputPath);
  
private:
  DECLARE_EVENT_TABLE()

  void OnOK(wxCommandEvent& WXUNUSED(event));
  void OnBtConfig(wxCommandEvent& WXUNUSED(event));
  void OnBtBatchAdd(wxCommandEvent& WXUNUSED(event));
  void OnBtBatchStart(wxCommandEvent& WXUNUSED(event));
  void OnCheckBoxs(wxCommandEvent& WXUNUSED(event));
  void OnDirPickerImages(wxFileDirPickerEvent& WXUNUSED(event));
  void OnDirPickerNVM(wxFileDirPickerEvent& WXUNUSED(event));
  void OnDirPickerOutput(wxFileDirPickerEvent& WXUNUSED(event));

  //Mesh
  //True - OK
  //False - wrong
  bool testImageFolder(wxString imagePath);
  int widthDefault = -1;
  int heightDefault = -1;

  //Batch reconstruction
  std::vector<BatchItem> batchItems;
  //Disable the EndModal in the OnOk method, this is used in the batch reconstruction
  bool disableEndModal = false;

  //Texturization
  MeshAux* loadMeshToTexturize(std::string meshPath);
  void createPointVisibilityData(MeshAux* mesh, CameraParameters* cameraParameters, std::vector<Camera*> cameras);

  wxCheckBox* cbGenerateMesh;
  wxCheckBox* cbTexturize;

  //Mesh
  wxStaticText* textImages;
  wxDirPickerCtrl* pickerImages;
  wxStaticText* textOutputMesh;
  wxFilePickerCtrl* pickerOutputMesh;
  
  //Texture
  wxStaticText* textMesh;
  wxFilePickerCtrl* pickerMesh;
  wxStaticText* textNVM;
  wxFilePickerCtrl* pickerNVM;
  wxStaticText* textOutputTexture;
  wxFilePickerCtrl* pickerOutputTexture;

  //Batch reconstruction
  wxStaticText* textQueueSize;

};

enum {
  idCBMesh,
  idCBTexture,
  idBtConfig,
  idBtBatchAdd,
  idBtBatchStart,
  idDirPickerImages,
  idDirPickerNVM,
  idDirPickerOutput
};
