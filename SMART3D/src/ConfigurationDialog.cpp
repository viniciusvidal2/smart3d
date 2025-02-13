#include "ConfigurationDialog.h"

#include <fstream>
#include <sstream>

#include <vtksys/SystemTools.hxx>

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/log.h>
#include <wx/stdpaths.h>

wxBEGIN_EVENT_TABLE(ConfigurationDialog, wxDialog)
EVT_BUTTON(wxID_OK, ConfigurationDialog::OnOK)
EVT_BUTTON(idBtDefaultConfigDialog, ConfigurationDialog::OnBtDefault)
wxEND_EVENT_TABLE()

bool ConfigurationDialog::isFirstInstance = true;
//COLMAP
int ConfigurationDialog::sparseQuality = 2;
int ConfigurationDialog::denseQuality = 2;
int ConfigurationDialog::mesher = 1;
bool ConfigurationDialog::useGPU = false;
bool ConfigurationDialog::forceDenseCOLMAP = false;
//MeshRecon
int ConfigurationDialog::boundingBoxType = 0;
int ConfigurationDialog::levelOfDetails = 0;
bool ConfigurationDialog::forceLevelOfDetails = false;
//TexRecon
int ConfigurationDialog::dataTerm = 1;
int ConfigurationDialog::outlierRemoval = 0;
bool ConfigurationDialog::geometricVisibilityTest = true;
bool ConfigurationDialog::globalSeamLeveling = false;
bool ConfigurationDialog::localSeamLeveling = true;
bool ConfigurationDialog::holeFilling = true;
bool ConfigurationDialog::keepUnseenFaces = false;
bool ConfigurationDialog::forceTexReconTexture = true;
//AVTexturing
int ConfigurationDialog::multiBandDownscale = 4;
double ConfigurationDialog::bestScoreThreshold = 0.1;
double ConfigurationDialog::angleHardThreshold = 90.0;
bool ConfigurationDialog::forceVisibleByAllVertices = false;
int ConfigurationDialog::visibilityRemappingMethod = 2;
unsigned int ConfigurationDialog::textureSide = 8192;
unsigned int ConfigurationDialog::padding = 5;
unsigned int ConfigurationDialog::downscale = 2;
bool ConfigurationDialog::fillHoles = false;
bool ConfigurationDialog::useUDIM = true;

ConfigurationDialog::ConfigurationDialog(wxWindow * parent, wxWindowID id, const wxString & title, const wxPoint & pos, const wxSize & size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	if (isFirstInstance)
	{
		loadDefaultConfig();
		isFirstInstance = false;
	}
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* bSizerMesh = new wxBoxSizer(wxHORIZONTAL);

	//COLMAP
	wxStaticBoxSizer* sbSizerCOLMAP = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "COLMAP"), wxVERTICAL);

	wxFlexGridSizer* fgSizerCOLMAP = new wxFlexGridSizer(5, 2, 0, 0);
	fgSizerCOLMAP->SetFlexibleDirection(wxBOTH);
	fgSizerCOLMAP->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	//Sparse quality
	fgSizerCOLMAP->Add(new wxStaticText(sbSizerCOLMAP->GetStaticBox(), wxID_ANY, "Sparse quality"), 0, wxALL, 5);
	wxArrayString choicesQuality;
	choicesQuality.Add("Low"); choicesQuality.Add("Medium"); choicesQuality.Add("High"); choicesQuality.Add("Extreme");
	choiceSparseQuality = new wxChoice(sbSizerCOLMAP->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, choicesQuality);
	choiceSparseQuality->SetSelection(sparseQuality);
	fgSizerCOLMAP->Add(choiceSparseQuality, 0, wxALL, 5);
	//------

	//Dense quality
	fgSizerCOLMAP->Add(new wxStaticText(sbSizerCOLMAP->GetStaticBox(), wxID_ANY, "Dense quality"), 0, wxALL, 5);
	choiceDenseQuality = new wxChoice(sbSizerCOLMAP->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, choicesQuality);
	choiceDenseQuality->SetSelection(denseQuality);
	fgSizerCOLMAP->Add(choiceDenseQuality, 0, wxALL, 5);
	//------

	//Mesher
	fgSizerCOLMAP->Add(new wxStaticText(sbSizerCOLMAP->GetStaticBox(), wxID_ANY, "Mesher"), 0, wxALL, 5);
	wxArrayString choicesMesher;
	choicesMesher.Add("Poisson"); choicesMesher.Add("Delaunay");
	choiceMesher = new wxChoice(sbSizerCOLMAP->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, choicesMesher);
	choiceMesher->SetSelection(mesher);
	fgSizerCOLMAP->Add(choiceMesher, 0, wxALL, 5);
	//------

	//Use GPU
	fgSizerCOLMAP->Add(new wxStaticText(sbSizerCOLMAP->GetStaticBox(), wxID_ANY, "Use GPU"), 0, wxALL, 5);
	ckBUseGPU = new wxCheckBox(sbSizerCOLMAP->GetStaticBox(), wxID_ANY, wxEmptyString);
	ckBUseGPU->SetValue(useGPU);
	fgSizerCOLMAP->Add(ckBUseGPU, 0, wxALL, 5);
	//------

	//Use dense COLMAP
	fgSizerCOLMAP->Add(new wxStaticText(sbSizerCOLMAP->GetStaticBox(), wxID_ANY, "Use dense COLMAP"), 0, wxALL, 5);
	ckBForceDenseCOLMAP = new wxCheckBox(sbSizerCOLMAP->GetStaticBox(), wxID_ANY, wxEmptyString);
	ckBForceDenseCOLMAP->SetValue(forceDenseCOLMAP);
	fgSizerCOLMAP->Add(ckBForceDenseCOLMAP, 0, wxALL, 5);
	//------

	sbSizerCOLMAP->Add(fgSizerCOLMAP, 1, wxEXPAND, 5);

	bSizerMesh->Add(sbSizerCOLMAP, 0, wxEXPAND, 5);

	//MeshRecon
	wxStaticBoxSizer* sbSizerMeshRecon = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "MeshRecon"), wxVERTICAL);
	wxFlexGridSizer* fgSizerMeshRecon = new wxFlexGridSizer(3, 2, 0, 0);
	fgSizerMeshRecon->SetFlexibleDirection(wxBOTH);
	fgSizerMeshRecon->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	fgSizerMeshRecon->Add(new wxStaticText(sbSizerMeshRecon->GetStaticBox(), wxID_ANY, "Bounding box type"), 0, wxALL, 5);
	wxArrayString choicesBoundingBox;
	choicesBoundingBox.Add("0"); choicesBoundingBox.Add("1");
	choiceBoundingBox = new wxChoice(sbSizerMeshRecon->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, choicesBoundingBox);
	choiceBoundingBox->SetSelection(boundingBoxType);
	fgSizerMeshRecon->Add(choiceBoundingBox, 0, wxALL, 5);

	fgSizerMeshRecon->Add(new wxStaticText(sbSizerMeshRecon->GetStaticBox(), wxID_ANY, "Level of details"), 0, wxALL, 5);
	wxArrayString choicesLevelOfDetails;
	choicesLevelOfDetails.Add("High"); choicesLevelOfDetails.Add("Medium"); choicesLevelOfDetails.Add("Low");
	choiceLevelOfDetails = new wxChoice(sbSizerMeshRecon->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, choicesLevelOfDetails);
	choiceLevelOfDetails->SetSelection(levelOfDetails);
	fgSizerMeshRecon->Add(choiceLevelOfDetails, 0, wxALL, 5);

	fgSizerMeshRecon->Add(new wxStaticText(sbSizerMeshRecon->GetStaticBox(), wxID_ANY, "Disable GPU memory check"), 0, wxALL, 5);
	ckBForceLevelOfDetails = new wxCheckBox(sbSizerMeshRecon->GetStaticBox(), wxID_ANY, wxEmptyString);
	ckBForceLevelOfDetails->SetValue(forceLevelOfDetails);
	fgSizerMeshRecon->Add(ckBForceLevelOfDetails, 0, wxALL, 5);

	sbSizerMeshRecon->Add(fgSizerMeshRecon, 1, wxEXPAND, 5);

	bSizerMesh->Add(sbSizerMeshRecon, 0, wxEXPAND, 5);

	wxBoxSizer* bSizerTexture = new wxBoxSizer(wxHORIZONTAL);

	//TexRecon
	wxStaticBoxSizer* sbSizerTexRecon = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "TexRecon"), wxVERTICAL);

	wxFlexGridSizer* fgSizerTexRecon = new wxFlexGridSizer(8, 2, 0, 0);
	fgSizerTexRecon->SetFlexibleDirection(wxBOTH);
	fgSizerTexRecon->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);


	fgSizerTexRecon->Add(new wxStaticText(sbSizerTexRecon->GetStaticBox(), wxID_ANY, "Data Term"), 0, wxALL, 5);
	wxArrayString choicesDataTerm;
	choicesDataTerm.Add("Area"); choicesDataTerm.Add("Gmi");
	choiceDataTerm = new wxChoice(sbSizerTexRecon->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, choicesDataTerm);
	choiceDataTerm->SetSelection(dataTerm);
	fgSizerTexRecon->Add(choiceDataTerm, 0, wxALL, 5);

	fgSizerTexRecon->Add(new wxStaticText(sbSizerTexRecon->GetStaticBox(), wxID_ANY, "Outlier removal"), 0, wxALL, 5);
	wxArrayString choicesOutlierRemoval;
	choicesOutlierRemoval.Add("None"); choicesOutlierRemoval.Add("Gauss damping"); choicesOutlierRemoval.Add("Gauss clamping");
	choiceOutlierRemoval = new wxChoice(sbSizerTexRecon->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, choicesOutlierRemoval);
	choiceOutlierRemoval->SetSelection(outlierRemoval);
	fgSizerTexRecon->Add(choiceOutlierRemoval, 0, wxALL, 5);

	fgSizerTexRecon->Add(new wxStaticText(sbSizerTexRecon->GetStaticBox(), wxID_ANY, "Geometric visibility test"), 0, wxALL, 5);
	ckBGeometricVisibilityTest = new wxCheckBox(sbSizerTexRecon->GetStaticBox(), wxID_ANY, wxEmptyString);
	ckBGeometricVisibilityTest->SetValue(geometricVisibilityTest);
	fgSizerTexRecon->Add(ckBGeometricVisibilityTest, 0, wxALL, 5);

	fgSizerTexRecon->Add(new wxStaticText(sbSizerTexRecon->GetStaticBox(), wxID_ANY, "Global seam leveling"), 0, wxALL, 5);
	ckBGlobalSeamLeveling = new wxCheckBox(sbSizerTexRecon->GetStaticBox(), wxID_ANY, wxEmptyString);
	ckBGlobalSeamLeveling->SetValue(globalSeamLeveling);
	fgSizerTexRecon->Add(ckBGlobalSeamLeveling, 0, wxALL, 5);

	fgSizerTexRecon->Add(new wxStaticText(sbSizerTexRecon->GetStaticBox(), wxID_ANY, "Local seam leveling"), 0, wxALL, 5);
	ckBLocalSeamLeveling = new wxCheckBox(sbSizerTexRecon->GetStaticBox(), wxID_ANY, wxEmptyString);
	ckBLocalSeamLeveling->SetValue(localSeamLeveling);
	fgSizerTexRecon->Add(ckBLocalSeamLeveling, 0, wxALL, 5);

	fgSizerTexRecon->Add(new wxStaticText(sbSizerTexRecon->GetStaticBox(), wxID_ANY, "Hole filling"), 0, wxALL, 5);
	ckBHoleFilling = new wxCheckBox(sbSizerTexRecon->GetStaticBox(), wxID_ANY, wxEmptyString);
	ckBHoleFilling->SetValue(holeFilling);
	fgSizerTexRecon->Add(ckBHoleFilling, 0, wxALL, 5);

	fgSizerTexRecon->Add(new wxStaticText(sbSizerTexRecon->GetStaticBox(), wxID_ANY, "Keep unseen faces"), 0, wxALL, 5);
	ckBKeepUnseenFaces = new wxCheckBox(sbSizerTexRecon->GetStaticBox(), wxID_ANY, wxEmptyString);
	ckBKeepUnseenFaces->SetValue(keepUnseenFaces);
	fgSizerTexRecon->Add(ckBKeepUnseenFaces, 0, wxALL, 5);

	fgSizerTexRecon->Add(new wxStaticText(sbSizerTexRecon->GetStaticBox(), wxID_ANY, "Use TexRecon texture"), 0, wxALL, 5);
	ckBForceTexReconTexture = new wxCheckBox(sbSizerTexRecon->GetStaticBox(), wxID_ANY, wxEmptyString);
	ckBForceTexReconTexture->SetValue(forceTexReconTexture);
	fgSizerTexRecon->Add(ckBForceTexReconTexture, 0, wxALL, 5);


	sbSizerTexRecon->Add(fgSizerTexRecon, 1, wxEXPAND, 5);


	bSizerTexture->Add(sbSizerTexRecon, 0, wxEXPAND, 5);

	//AVTexturing
	wxStaticBoxSizer* sbSizerAVTexturing = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "AVTexturing"), wxVERTICAL);

	wxFlexGridSizer* fgSizerAVTexturing = new wxFlexGridSizer(10, 2, 0, 0);
	fgSizerAVTexturing->SetFlexibleDirection(wxBOTH);
	fgSizerAVTexturing->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);


	fgSizerAVTexturing->Add(new wxStaticText(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, "Multi-band downscale"), 0, wxALL, 5);
	spinMultiBandDownscale = new wxSpinCtrl(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 16384L, 1, 8, multiBandDownscale);
	fgSizerAVTexturing->Add(spinMultiBandDownscale, 0, wxALL, 5);

	fgSizerAVTexturing->Add(new wxStaticText(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, "Best score threshold"), 0, wxALL, 5);
	spinBestScoreThreshold = new wxSpinCtrlDouble(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 16384L, 0, 1.0, bestScoreThreshold);
	fgSizerAVTexturing->Add(spinBestScoreThreshold, 0, wxALL, 5);

	fgSizerAVTexturing->Add(new wxStaticText(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, "Angle hard threshold"), 0, wxALL, 5);
	spinAngleHardThreshold = new wxSpinCtrlDouble(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 16384L, 0, 180.0, angleHardThreshold);
	fgSizerAVTexturing->Add(spinAngleHardThreshold, 0, wxALL, 5);

	fgSizerAVTexturing->Add(new wxStaticText(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, "Force visible by all vertices"), 0, wxALL, 5);
	ckBForceVisibleByAllVertices = new wxCheckBox(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, wxEmptyString);
	ckBForceVisibleByAllVertices->SetValue(forceVisibleByAllVertices);
	fgSizerAVTexturing->Add(ckBForceVisibleByAllVertices, 0, wxALL, 5);

	fgSizerAVTexturing->Add(new wxStaticText(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, "Visibility remapping method"), 0, wxALL, 5);
	wxArrayString choicesVisibilityRemappingMethod;
	choicesVisibilityRemappingMethod.Add("Pull"); choicesVisibilityRemappingMethod.Add("Push"); choicesVisibilityRemappingMethod.Add("Pull-Push");
	choiceVisibilityRemappingMethod = new wxChoice(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, choicesVisibilityRemappingMethod);
	choiceVisibilityRemappingMethod->SetSelection(visibilityRemappingMethod - 1);
	fgSizerAVTexturing->Add(choiceVisibilityRemappingMethod, 0, wxALL, 5);

	fgSizerAVTexturing->Add(new wxStaticText(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, "Texture side"), 0, wxALL, 5);
	spinTextureSide = new wxSpinCtrl(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 16384L, 1024, 16384, textureSide);
	fgSizerAVTexturing->Add(spinTextureSide, 0, wxALL, 5);

	fgSizerAVTexturing->Add(new wxStaticText(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, "Padding"), 0, wxALL, 5);
	spinPadding = new wxSpinCtrl(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 16384L, 0, 100, padding);
	fgSizerAVTexturing->Add(spinPadding, 0, wxALL, 5);

	fgSizerAVTexturing->Add(new wxStaticText(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, "Downscale"), 0, wxALL, 5);
	spinDownscale = new wxSpinCtrl(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 16384L, 1, 8, downscale);
	fgSizerAVTexturing->Add(spinDownscale, 0, wxALL, 5);

	fgSizerAVTexturing->Add(new wxStaticText(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, "Fill holes"), 0, wxALL, 5);
	ckBFillHoles = new wxCheckBox(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, wxEmptyString);
	ckBFillHoles->SetValue(fillHoles);
	fgSizerAVTexturing->Add(ckBFillHoles, 0, wxALL, 5);

	fgSizerAVTexturing->Add(new wxStaticText(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, "Use UDIM"), 0, wxALL, 5);
	ckBUseUDIM = new wxCheckBox(sbSizerAVTexturing->GetStaticBox(), wxID_ANY, wxEmptyString);
	ckBUseUDIM->SetValue(useUDIM);
	fgSizerAVTexturing->Add(ckBUseUDIM, 0, wxALL, 5);

	sbSizerAVTexturing->Add(fgSizerAVTexturing, 1, wxEXPAND, 5);

	bSizerTexture->Add(sbSizerAVTexturing, 0, wxEXPAND, 5);

	//Add mesh and texture box sizers
	bSizer->Add(bSizerMesh, 0, wxEXPAND, 5);
	bSizer->Add(bSizerTexture, 0, wxEXPAND, 5);

	wxBoxSizer* bSizerBts = new wxBoxSizer(wxHORIZONTAL);

	bSizerBts->Add(new wxButton(this, idBtDefaultConfigDialog, "Default"), 0, wxALL, 5);

	bSizerBts->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL, 5);

	bSizerBts->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);

	bSizer->Add(bSizerBts, 0, wxALIGN_RIGHT, 5);
	this->SetSizer(bSizer);
	this->Layout();

	this->Centre(wxBOTH);
}

ConfigurationDialog::~ConfigurationDialog()
{
}

void ConfigurationDialog::loadDefaultConfig()
{
	std::ifstream parametersFile(vtksys::SystemTools::GetFilenamePath(wxStandardPaths::Get().GetExecutablePath().ToStdString()) + "/projetomesh/parameters.txt");
	std::string line = "", parameterName;
	int parameterValue;
	double parameterValueDouble;
	if (parametersFile.is_open())
	{
		while (getline(parametersFile, line, '\n'))
		{
			if (line.size() > 0)
			{
				if (line[0] != '/' && line.find_first_of('=') != std::string::npos && line.find_first_of(';') != std::string::npos)
				{
					parameterName = line.substr(0, line.find_first_of('=') - 1);
					std::string value = line.substr(line.find_first_of('=') + 1, line.find_first_of(';') - (line.find_first_of('=') + 1));
					parameterValue = std::stoi(value);
					parameterValueDouble = std::stod(value);
					if (parameterName == "sparse quality")
					{
						sparseQuality = parameterValue;
					}
					else if (parameterName == "dense quality")
					{
						denseQuality = parameterValue;
					}
					else if (parameterName == "mesher")
					{
						mesher = parameterValue;
					}
					else if (parameterName == "useGPU")
					{
						useGPU = parameterValue;
					}
					else if (parameterName == "forceDenseCOLMAP")
					{
						forceDenseCOLMAP = parameterValue;
					}
					else if (parameterName == "BoundingBox_Type")
					{
						boundingBoxType = parameterValue;
					}
					else if (parameterName == "level of details")
					{
						levelOfDetails = parameterValue;
					}
					else if (parameterName == "forceLevelOfDetails")
					{
						forceLevelOfDetails = parameterValue;
					}
					else if (parameterName == "Data term")
					{
						dataTerm = parameterValue;
					}
					else if (parameterName == "Outlier removal")
					{
						outlierRemoval = parameterValue;
					}
					else if (parameterName == "Geometric visibility test")
					{
						geometricVisibilityTest = parameterValue;
					}
					else if (parameterName == "Global seam leveling")
					{
						globalSeamLeveling = parameterValue;
					}
					else if (parameterName == "Local seam leveling")
					{
						localSeamLeveling = parameterValue;
					}
					else if (parameterName == "Hole filling")
					{
						holeFilling = parameterValue;
					}
					else if (parameterName == "Keep unseen faces")
					{
						keepUnseenFaces = parameterValue;
					}
					else if (parameterName == "forceTexReconTexture")
					{
						forceTexReconTexture = parameterValue;
					}
					else if (parameterName == "multiBandDownscale")
					{
						multiBandDownscale = parameterValue;
					}
					else if (parameterName == "bestScoreThreshold")
					{
						bestScoreThreshold = parameterValueDouble;
					}
					else if (parameterName == "angleHardThreshold")
					{
						angleHardThreshold = parameterValueDouble;
					}
					else if (parameterName == "forceVisibleByAllVertices")
					{
						forceVisibleByAllVertices = parameterValue;
					}
					else if (parameterName == "visibilityRemappingMethod")
					{
						visibilityRemappingMethod = parameterValue;
					}
					else if (parameterName == "textureSide")
					{
						textureSide = parameterValue;
					}
					else if (parameterName == "padding")
					{
						padding = parameterValue;
					}
					else if (parameterName == "downscale")
					{
						downscale = parameterValue;
					}
					else if (parameterName == "fillHoles")
					{
						fillHoles = parameterValue;
					}
					else if (parameterName == "useUDIM")
					{
						useUDIM = parameterValue;
					}
				}
			}
		}
		parametersFile.close();
	}
	else
	{
		wxLogError("Unable to open the default parameters file");
		return;
	}
}

std::string ConfigurationDialog::getParameters()
{
	std::stringstream parameters;
	OptionsAVTexturing options = getOptionsAVTexturing();
	parameters << "------------------------------------------------------\n" <<
		"COLMAP\n" <<
		"Sparse quality " << getSparseQuality() << "\n" <<
		"Dense quality " << getDenseQuality() << "\n" <<
		"Mesher " << getMesher() << "\n" <<
		"Use GPU " << getUseGPU() << "\n" <<
		"Use dense COLMAP " << getForceDenseCOLMAP() << "\n" <<
		"------------------------------------------------------\n" <<
		"MeshRecon\n" <<
		"Bounding box type " << getBoundingBox() << "\n" <<
		"Level of details " << getLevelOfDetails() << "\n" <<
		"Disable GPU memory check " << getForceLevelOfDetails() << "\n" <<
		"------------------------------------------------------\n" <<
		"TexRecon\n" <<
		"Data term " << getDataTerm() << "\n" <<
		"Outlier removal " << getOutlierRemoval() << "\n" <<
		"Geometric visibility test " << geometricVisibilityTest << "\n" <<
		"Global seam leveling " << globalSeamLeveling << "\n" <<
		"Local seam leveling " << localSeamLeveling << "\n" <<
		"Hole filling " << holeFilling << "\n" <<
		"Keep unseen faces " << keepUnseenFaces << "\n" <<
		"Force TexRecon texture " << forceTexReconTexture << "\n" <<
		"------------------------------------------------------\n" <<
		"AVTexturing\n" <<
		"Multi band downscale " << options.multiBandDownscale << "\n" <<
		"Best score threshold " << options.bestScoreThreshold << "\n" <<
		"Angle hard threshold " << options.angleHardThreshold << "\n" <<
		"Visibility remapping method " << options.visibilityRemappingMethod << "\n" <<
		"Texture side " << options.textureSide << "\n" <<
		"Padding " << options.padding << "\n" <<
		"Downscale " << options.downscale << "\n" <<
		"Fill holes " << options.fillHoles << "\n" <<
		"Use UDIM " << options.useUDIM << "\n" <<
		"------------------------------------------------------\n";
	return parameters.str();
}

std::string ConfigurationDialog::getMeshReconParameters()
{
	std::stringstream parameters;
	parameters << boundingBoxType << " " << levelOfDetails;
	return parameters.str();
}

std::string ConfigurationDialog::getTexReconParameters()
{
	std::stringstream parameters;
	parameters << dataTerm << " " << outlierRemoval << " " << geometricVisibilityTest << " " << globalSeamLeveling << " " << localSeamLeveling
		<< " " << holeFilling << " " << keepUnseenFaces;
	return parameters.str();
}

std::string ConfigurationDialog::getSparseQuality()
{
	switch (sparseQuality)
	{
	case 0:
		return "low";
	case 1:
		return "medium";
	case 2:
		return "high";
	case 3:
		return "extreme";
	default:
		return "high";
	}
}

std::string ConfigurationDialog::getDenseQuality()
{
	switch (denseQuality)
	{
	case 0:
		return "low";
	case 1:
		return "medium";
	case 2:
		return "high";
	case 3:
		return "extreme";
	default:
		return "high";
	}
}

std::string ConfigurationDialog::getMesher()
{
	switch (mesher)
	{
	case 0:
		return "poisson";
	case 1:
		return "delaunay";
	default:
		return "delaunay";
	}
}

std::string ConfigurationDialog::getUseGPU()
{
	if (useGPU)
	{
		return "1";
	}
	return "0";
}

bool ConfigurationDialog::getForceDenseCOLMAP()
{
	return forceDenseCOLMAP;
}

std::string ConfigurationDialog::getBoundingBox()
{
	return std::to_string(boundingBoxType);
}

std::string ConfigurationDialog::getLevelOfDetails()
{
	switch (levelOfDetails)
	{
	case 0:
		return "High";
	case 1:
		return "Medium";
	case 2:
		return "Low";
	default:
		return "High";
	}
}

void ConfigurationDialog::setLevelOfDetails(int newLevelOfDetails)
{
	levelOfDetails = newLevelOfDetails;
}

bool ConfigurationDialog::getForceLevelOfDetails()
{
	return forceLevelOfDetails;
}

std::string ConfigurationDialog::getDataTerm()
{
	switch (dataTerm)
	{
	case 0:
		return "Area";
	case 1:
		return "Gmi";
	default:
		return "Gmi";
	}
}

std::string ConfigurationDialog::getOutlierRemoval()
{
	switch (outlierRemoval)
	{
	case 0:
		return "None";
	case 1:
		return "Gauss damping";
	case 2:
		return "Gauss clamping";
	default:
		return "None";
	}
}

bool ConfigurationDialog::getForceTexReconTexture()
{
	return forceTexReconTexture;
}

OptionsAVTexturing ConfigurationDialog::getOptionsAVTexturing()
{
	OptionsAVTexturing options;
	options.multiBandDownscale = multiBandDownscale;
	options.bestScoreThreshold = bestScoreThreshold;
	options.angleHardThreshold = angleHardThreshold;
	options.forceVisibleByAllVertices = forceVisibleByAllVertices;
	options.visibilityRemappingMethod = visibilityRemappingMethod;
	options.textureSide = textureSide;
	options.padding = padding;
	options.downscale = downscale;
	options.fillHoles = fillHoles;
	options.useUDIM = useUDIM;
	return options;
}

void ConfigurationDialog::OnOK(wxCommandEvent & WXUNUSED)
{
	//COLMAP
	sparseQuality = choiceSparseQuality->GetSelection();
	denseQuality = choiceDenseQuality->GetSelection();
	mesher = choiceMesher->GetSelection();
	useGPU = ckBUseGPU->IsChecked();
	forceDenseCOLMAP = ckBForceDenseCOLMAP->IsChecked();
	//MeshRecon
	boundingBoxType = choiceBoundingBox->GetSelection();
	levelOfDetails = choiceLevelOfDetails->GetSelection();
	forceLevelOfDetails = ckBForceLevelOfDetails->IsChecked();
	//TexRecon
	dataTerm = choiceDataTerm->GetSelection();
	outlierRemoval = choiceOutlierRemoval->GetSelection();
	geometricVisibilityTest = ckBGeometricVisibilityTest->IsChecked();
	globalSeamLeveling = ckBGlobalSeamLeveling->IsChecked();
	localSeamLeveling = ckBLocalSeamLeveling->IsChecked();
	holeFilling = ckBHoleFilling->IsChecked();
	keepUnseenFaces = ckBKeepUnseenFaces->IsChecked();
	forceTexReconTexture = ckBForceTexReconTexture->IsChecked();
	//AVTexturing
	multiBandDownscale = spinMultiBandDownscale->GetValue();
	bestScoreThreshold = spinBestScoreThreshold->GetValue();
	angleHardThreshold = spinAngleHardThreshold->GetValue();
	forceVisibleByAllVertices = ckBForceVisibleByAllVertices->IsChecked();
	visibilityRemappingMethod = choiceVisibilityRemappingMethod->GetSelection() + 1;
	textureSide = spinTextureSide->GetValue();
	padding = spinPadding->GetValue();
	downscale = spinDownscale->GetValue();
	fillHoles = ckBFillHoles->IsChecked();
	useUDIM = ckBUseUDIM->IsChecked();
	EndModal(wxID_OK);
}

void ConfigurationDialog::OnBtDefault(wxCommandEvent & WXUNUSED)
{
	loadDefaultConfig();
	//COLMAP
	choiceSparseQuality->SetSelection(sparseQuality);
	choiceDenseQuality->SetSelection(denseQuality);
	choiceMesher->SetSelection(mesher);
	ckBUseGPU->SetValue(useGPU);
	ckBForceDenseCOLMAP->SetValue(forceDenseCOLMAP);
	//MeshRecon
	choiceBoundingBox->SetSelection(boundingBoxType);
	choiceLevelOfDetails->SetSelection(levelOfDetails);
	ckBForceLevelOfDetails->SetValue(forceLevelOfDetails);
	//TexRecon
	choiceDataTerm->SetSelection(dataTerm);
	choiceOutlierRemoval->SetSelection(outlierRemoval);
	ckBGeometricVisibilityTest->SetValue(geometricVisibilityTest);
	ckBGlobalSeamLeveling->SetValue(globalSeamLeveling);
	ckBLocalSeamLeveling->SetValue(localSeamLeveling);
	ckBHoleFilling->SetValue(holeFilling);
	ckBKeepUnseenFaces->SetValue(keepUnseenFaces);
	ckBForceTexReconTexture->SetValue(forceTexReconTexture);
	//AVTexturing
	spinMultiBandDownscale->SetValue(multiBandDownscale);
	spinBestScoreThreshold->SetValue(bestScoreThreshold);
	spinAngleHardThreshold->SetValue(angleHardThreshold);
	ckBForceVisibleByAllVertices->SetValue(forceVisibleByAllVertices);
	choiceVisibilityRemappingMethod->SetSelection(visibilityRemappingMethod - 1);
	spinTextureSide->SetValue(textureSide);
	spinPadding->SetValue(padding);
	spinDownscale->SetValue(downscale);
	ckBFillHoles->SetValue(fillHoles);
	ckBUseUDIM->SetValue(useUDIM);
}
