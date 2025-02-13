#pragma once
#include <wx/dialog.h>
#include "AVTexturing.h"

class wxChoice;
class wxCheckBox;
class wxSpinCtrlDouble;

class ConfigurationDialog : public wxDialog
{
public:
	ConfigurationDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = "Configuration", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(610, 580), long style = wxDEFAULT_DIALOG_STYLE);
	~ConfigurationDialog();

	static void loadDefaultConfig();
	static std::string getParameters();
	static std::string getMeshReconParameters();
	static std::string getTexReconParameters();

	//COLMAP
	//0 - Low 1 - Medium 2 - High 3 - Extreme
	static std::string getSparseQuality();
	//0 - Low 1 - Medium 2 - High 3 - Extreme
	static std::string getDenseQuality();
	//0 - Poisson 1 - Delaunay
	static std::string getMesher();
	static std::string getUseGPU();
	static bool getForceDenseCOLMAP();

	//MeshRecon
	static std::string getBoundingBox();
	//0 - High 1 - Medium 2 - Low
	static std::string getLevelOfDetails();
	static void setLevelOfDetails(int newLevelOfDetails);
	static bool getForceLevelOfDetails();

	//TexRecon
	// 0 - Area 1 - Gmi
	static std::string getDataTerm();
	// 0 - None 1 - Gauss damping 2 - Gauss clamping
	static std::string getOutlierRemoval();
	static bool getForceTexReconTexture();

	//AVTexturing
	static OptionsAVTexturing getOptionsAVTexturing();

private:
	DECLARE_EVENT_TABLE()

	void OnOK(wxCommandEvent& WXUNUSED(event));
	void OnBtDefault(wxCommandEvent& WXUNUSED(event));

	//COLMAP
	wxChoice* choiceSparseQuality;
	wxChoice* choiceDenseQuality;
	wxChoice* choiceMesher;
	wxCheckBox* ckBUseGPU;
	wxCheckBox* ckBForceDenseCOLMAP;

	//MeshRecon
	wxChoice* choiceBoundingBox;
	wxChoice* choiceLevelOfDetails;
	wxCheckBox* ckBForceLevelOfDetails;

	//TexRecon
	wxChoice* choiceDataTerm;
	wxChoice* choiceOutlierRemoval;
	wxCheckBox* ckBGeometricVisibilityTest;
	wxCheckBox* ckBGlobalSeamLeveling;
	wxCheckBox* ckBLocalSeamLeveling;
	wxCheckBox* ckBHoleFilling;
	wxCheckBox* ckBKeepUnseenFaces;
	wxCheckBox* ckBForceTexReconTexture;

	//AVTexturing
	wxSpinCtrl* spinMultiBandDownscale;
	wxSpinCtrlDouble* spinBestScoreThreshold;
	wxSpinCtrlDouble* spinAngleHardThreshold;
	wxCheckBox* ckBForceVisibleByAllVertices;
	wxChoice* choiceVisibilityRemappingMethod;
	wxSpinCtrl* spinTextureSide;
	wxSpinCtrl* spinPadding;
	wxSpinCtrl* spinDownscale;
	wxCheckBox* ckBFillHoles;
	wxCheckBox* ckBUseUDIM;


	//Used to load the default parameters
	static bool isFirstInstance;
	//COLMAP
	static int sparseQuality;
	static int denseQuality;
	static int mesher;
	static bool useGPU;
	static bool forceDenseCOLMAP;
	//MeshRecon
	static int boundingBoxType;
	static int levelOfDetails;
	static bool forceLevelOfDetails;
	//TexRecon
	static int dataTerm;
	static int outlierRemoval;
	static bool geometricVisibilityTest;
	static bool globalSeamLeveling;
	static bool localSeamLeveling;
	static bool holeFilling;
	static bool keepUnseenFaces;
	static bool forceTexReconTexture;
	//AVTexturing
	static int multiBandDownscale;
	static double bestScoreThreshold;
	static double angleHardThreshold;
	static bool forceVisibleByAllVertices;
	static int visibilityRemappingMethod;
	static unsigned int textureSide;
	static unsigned int padding;
	static unsigned int downscale;
	static bool fillHoles;
	static bool useUDIM;

};
enum EnumConfigDialog
{
	idBtDefaultConfigDialog
};