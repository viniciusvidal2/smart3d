#include "ReconstructionDialog.h"

#include <gl\GL.h>

#include <vtkMatrix4x4.h>
#include <vtkPLYReader.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtksys/SystemTools.hxx>

#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/bmpbuttn.h>
#include <wx/filepicker.h>
#include <wx/dir.h>
#include <wx/sizer.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/progdlg.h>

#include "HelperCOLMAP.h"
#include "HelperMeshRecon.h"
#include <claudette/collision_model_3d.h>
#include <claudette/ray_collision_test.h>
#include "ReconstructionLog.h"
#include "ConfigurationDialog.h"
#include "Utils.h"
#include "ImageIO.h"
#include "Camera.h"

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

wxBEGIN_EVENT_TABLE(ReconstructionDialog, wxDialog)
EVT_BUTTON(wxID_OK, ReconstructionDialog::OnOK)
EVT_BUTTON(idBtConfig, ReconstructionDialog::OnBtConfig)
EVT_BUTTON(idBtBatchAdd, ReconstructionDialog::OnBtBatchAdd)
EVT_BUTTON(idBtBatchStart, ReconstructionDialog::OnBtBatchStart)
EVT_CHECKBOX(idCBMesh, ReconstructionDialog::OnCheckBoxs)
EVT_CHECKBOX(idCBTexture, ReconstructionDialog::OnCheckBoxs)
EVT_DIRPICKER_CHANGED(idDirPickerImages, ReconstructionDialog::OnDirPickerImages)
EVT_FILEPICKER_CHANGED(idDirPickerNVM, ReconstructionDialog::OnDirPickerNVM)
EVT_FILEPICKER_CHANGED(idDirPickerOutput, ReconstructionDialog::OnDirPickerOutput)
wxEND_EVENT_TABLE()


ReconstructionDialog::ReconstructionDialog(wxWindow * parent, wxWindowID id, const wxString & title, const wxPoint & pos, const wxSize & size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	auto bSizer = new wxBoxSizer(wxVERTICAL);

	cbGenerateMesh = new wxCheckBox(this, idCBMesh, "Generate mesh");
	bSizer->Add(cbGenerateMesh, 0, wxALL, 5);
	cbTexturize = new wxCheckBox(this, idCBTexture, "Generate texture");
	bSizer->Add(cbTexturize, 0, wxALL, 5);

	//Mesh
	wxStaticBoxSizer* sbSizerMesh;
	sbSizerMesh = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Mesh"), wxVERTICAL);

	textImages = new wxStaticText(sbSizerMesh->GetStaticBox(), wxID_ANY, "Select the image folder:");
	sbSizerMesh->Add(textImages, 0, wxALL, 5);
	pickerImages = new wxDirPickerCtrl(sbSizerMesh->GetStaticBox(), idDirPickerImages, wxEmptyString, "Select the image folder");
	sbSizerMesh->Add(pickerImages, 0, wxALL, 5);

	textOutputMesh = new wxStaticText(sbSizerMesh->GetStaticBox(), wxID_ANY, "Select the output file:");
	sbSizerMesh->Add(textOutputMesh, 0, wxALL, 5);
	pickerOutputMesh = new wxFilePickerCtrl(sbSizerMesh->GetStaticBox(), idDirPickerOutput, wxEmptyString, "Select the output file", "PLY files(*.ply) | *.ply", wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL | wxFLP_SAVE | wxFLP_OVERWRITE_PROMPT);
	sbSizerMesh->Add(pickerOutputMesh, 0, wxALL, 5);

	bSizer->Add(sbSizerMesh, 0, wxALL, 5);

	//Texture
	wxStaticBoxSizer* sbSizerTexture;
	sbSizerTexture = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Texture"), wxVERTICAL);

	textMesh = new wxStaticText(sbSizerTexture->GetStaticBox(), wxID_ANY, "Select the mesh to be texturized:");
	sbSizerTexture->Add(textMesh, 0, wxALL, 5);
	pickerMesh = new wxFilePickerCtrl(sbSizerTexture->GetStaticBox(), wxID_ANY, wxEmptyString, "Select the mesh to be texturized", "PLY files (*.ply)|*.ply");
	sbSizerTexture->Add(pickerMesh, 0, wxALL, 5);

	textNVM = new wxStaticText(sbSizerTexture->GetStaticBox(), wxID_ANY, "Select the NVM file:");
	sbSizerTexture->Add(textNVM, 0, wxALL, 5);
	pickerNVM = new wxFilePickerCtrl(sbSizerTexture->GetStaticBox(), idDirPickerNVM, wxEmptyString, "Select the NVM file", "NVM files (*.nvm)|*.nvm");
	sbSizerTexture->Add(pickerNVM, 0, wxALL, 5);

	textOutputTexture = new wxStaticText(sbSizerTexture->GetStaticBox(), wxID_ANY, "Select the output file:");
	sbSizerTexture->Add(textOutputTexture, 0, wxALL, 5);
	pickerOutputTexture = new wxFilePickerCtrl(sbSizerTexture->GetStaticBox(), idDirPickerOutput, wxEmptyString, "Select the output file", "OBJ files(*.obj) | *.obj", wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL | wxFLP_SAVE | wxFLP_OVERWRITE_PROMPT);
	sbSizerTexture->Add(pickerOutputTexture, 0, wxALL, 5);

	bSizer->Add(sbSizerTexture, 0, wxALL, 5);

	//Batch reconstruction
	wxStaticBoxSizer* sbSizerBatch = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Batch reconstruction"), wxVERTICAL);

	sbSizerBatch->Add(new wxStaticText(sbSizerBatch->GetStaticBox(), wxID_ANY, "Add a reconstruction in the queue:"), 0, wxALL, 5);

	wxBoxSizer* bSizerBtsBatch = new wxBoxSizer(wxHORIZONTAL);
	bSizerBtsBatch->Add(new wxButton(sbSizerBatch->GetStaticBox(), idBtBatchAdd, "Add"), 0, wxALL | wxALIGN_CENTER, 5);
	bSizerBtsBatch->Add(new wxButton(sbSizerBatch->GetStaticBox(), idBtBatchStart, "Start"), 0, wxALL | wxALIGN_CENTER, 5);
	sbSizerBatch->Add(bSizerBtsBatch, 0, wxALL | wxALIGN_CENTER, 5);

	textQueueSize = new wxStaticText(sbSizerBatch->GetStaticBox(), wxID_ANY, "Queue size: 0");
	sbSizerBatch->Add(textQueueSize, 0, wxALL, 5);

	bSizer->Add(sbSizerBatch, 0, wxALL, 5);

	//Bts
	wxBoxSizer* bSizerBts = new wxBoxSizer(wxHORIZONTAL);

	bSizerBts->Add(new wxBitmapButton(this, idBtConfig, wxICON(ICON_CONFIG)), 0, wxALL | wxALIGN_CENTER, 5);

	bSizerBts->Add(new wxButton(this, wxID_OK, "OK"), 0, wxALL | wxALIGN_CENTER, 5);

	bSizerBts->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL | wxALIGN_CENTER, 5);

	bSizer->Add(bSizerBts, 0, wxALIGN_RIGHT, 5);

	OnCheckBoxs((wxCommandEvent)NULL);

	this->SetSizer(bSizer);
	this->Layout();
	this->Fit();
	this->Centre(wxBOTH);
}

ReconstructionDialog::~ReconstructionDialog()
{
}

wxString ReconstructionDialog::getOutputPath()
{
	wxString path;
	if (cbTexturize->IsChecked())
	{
		path = pickerOutputTexture->GetPath();
	}
	else
	{
		path = pickerOutputMesh->GetPath();
	}
	std::replace(path.begin(), path.end(), '\\', '/');
	std::size_t found = path.find_last_of(".");
	path = path.substr(0, found);
	if (cbTexturize->IsChecked())
	{
		path += "_textured.obj";
	}
	else
	{
		path += "_refine.ply";
	}
	return path;
}

void ReconstructionDialog::texturizeMesh(std::string meshPath, std::string nvmPath, std::string outputPath)
{
	cbTexturize->SetValue(true);
	pickerMesh->SetPath(meshPath);
	pickerNVM->SetPath(nvmPath);
	pickerOutputTexture->SetPath(outputPath);
	OnOK((wxCommandEvent)NULL);
}

void ReconstructionDialog::OnOK(wxCommandEvent & WXUNUSED)
{
	//Log
	ReconstructionLog* log = nullptr;
	if (cbGenerateMesh->IsChecked())
	{
		if (!cbTexturize->IsChecked())
		{
			if (pickerImages->GetPath() == "" || pickerOutputMesh->GetPath() == "")
			{
				wxLogError("Please fulfill all the fields correctly.");
				return;
			}
			//Log
			std::string path = pickerOutputMesh->GetPath();
			log = new ReconstructionLog(path.substr(0, path.size() - 4) + "_log.txt");
			log->write("Generate mesh - Yes");
			log->write("Generate texture - No");
		}
		else
		{
			if (pickerImages->GetPath() == "" || pickerOutputTexture->GetPath() == "")
			{
				wxLogError("Please fulfill all the fields correctly.");
				return;
			}
			//Log
			std::string path = pickerOutputTexture->GetPath();
			log = new ReconstructionLog(path.substr(0, path.size() - 4) + "_log.txt");
			log->write("Generate mesh - Yes");
			log->write("Generate texture - Yes");
		}
	}
	else if (cbTexturize->IsChecked())
	{
		if (pickerNVM->GetPath() == "" || pickerMesh->GetPath() == "" || pickerOutputTexture->GetPath() == "")
		{
			wxLogError("Please fulfill all the fields correctly.");
			return;
		}
		//Log
		std::string path = pickerOutputTexture->GetPath();
		log = new ReconstructionLog(path.substr(0, path.size() - 4) + "_log.txt");
		log->write("Generate mesh - No");
		log->write("Generate texture - Yes");
	}
	else
	{
		wxLogError("Please select at least one checkbox.");
		return;
	}
	//Log
	log->addSeparator();
	//Mesh
	std::string nvmPath;
	std::string meshPath;
	bool useDenseCOLMAP = ConfigurationDialog::getForceDenseCOLMAP();
	if (cbGenerateMesh->IsChecked())
	{
		if (!cbTexturize->IsChecked())
		{
			nvmPath = pickerOutputMesh->GetPath();
		}
		else
		{
			nvmPath = pickerOutputTexture->GetPath();
		}
		//COLMAP
		nvmPath = nvmPath.substr(0, nvmPath.size() - 4) + ".nvm";
		log->write("Started sparse reconstruction", true, true);
		if (!HelperCOLMAP::executeSparse(pickerImages->GetPath().ToStdString(), nvmPath))
		{
			log->write("Error during sparse reconstruction", true, true);
			delete log;
			return;
		}
		log->write("Finished sparse reconstruction", true, true);
		log->addSeparator();
		//Do the memory check for MeshRecon
		if (!useDenseCOLMAP)
		{
			int numberOfcameras = Utils::getNumberOfCamerasNVM(nvmPath);
			if (numberOfcameras > 200)
			{
				wxLogWarning("Too many cameras to MeshRecon, using COLMAP", "Warning");
				log->write("Too many cameras to MeshRecon, using COLMAP");
				useDenseCOLMAP = true;
			}
			else //Memory check
			{
				GLint nCurAvailMemoryInKB = 0;
				glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, &nCurAvailMemoryInKB);
				double memoryNeeded = widthDefault * heightDefault * numberOfcameras / 1000;
				int levelOfDetailsAllowed = -1;
				std::string levelOfDetailsAllowedString = "";
				if (nCurAvailMemoryInKB / 1024 > memoryNeeded * 0.025)
				{
					levelOfDetailsAllowed = 0;
					levelOfDetailsAllowedString = "high";
				}
				else if (nCurAvailMemoryInKB / 1024 > memoryNeeded * 0.008)
				{
					levelOfDetailsAllowed = 1;
					levelOfDetailsAllowedString = "medium";
				}
				else if (nCurAvailMemoryInKB / 1024 > memoryNeeded * 0.003)
				{
					levelOfDetailsAllowed = 2;
					levelOfDetailsAllowedString = "low";
				}
				if (std::atoi(ConfigurationDialog::getLevelOfDetails().c_str()) < levelOfDetailsAllowed && !ConfigurationDialog::getForceLevelOfDetails())
				{
					if (levelOfDetailsAllowed >= 0)
					{
						wxLogWarning("Not enough RAM to use this level of details, lowering the settings to: %s", levelOfDetailsAllowedString);
						log->write("Not enough RAM to use this level of details, lowering the settings to: " + levelOfDetailsAllowedString);
						ConfigurationDialog::setLevelOfDetails(levelOfDetailsAllowed);
					}
					else
					{
						wxLogWarning("Not enough RAM to use MeshRecon, using COLMAP", "Warning");
						log->write("Not enough RAM to use MeshRecon, using COLMAP");
						useDenseCOLMAP = true;
					}
				}
			}
		}
		if (useDenseCOLMAP)//COLMAP
		{
			log->write("Started COLMAP dense reconstruction", true, true);
			if (!HelperCOLMAP::executeDense(pickerImages->GetPath().ToStdString(), nvmPath))
			{
				log->write("Error during COLMAP dense reconstruction", true, true);
				delete log;
				return;
			}
			log->write("Finished COLMAP dense reconstruction", true, true);
			log->addSeparator();
			//Move the file!
			std::string originalPLY = vtksys::SystemTools::GetFilenamePath(nvmPath) + "/dense/0/meshed-" + ConfigurationDialog::getMesher() + ".ply";
			if (!Utils::exists(originalPLY))
			{
				wxLogError("No mesh generated from COLMAP");
				log->write("No mesh generated from COLMAP", true, true);
				delete log;
				return;
			}
			std::wstring stemp = Utils::s2ws(originalPLY);
			LPCWSTR original = stemp.c_str();

			std::string newPLY = nvmPath.substr(0, nvmPath.size() - 4) + "_refine.ply";
			std::wstring stemp2 = Utils::s2ws(newPLY);
			LPCWSTR newPath = stemp2.c_str();

			if (!MoveFile(original, newPath))
			{
				wxLogError("MoveFile failed (%d)", GetLastError());
				log->write("MoveFile failed", true, true);
				delete log;
				return;
			}
		}
		else//MeshRecon
		{
			log->write("Started NVM2SFM", true, true);
			std::string sfmPath = nvmPath.substr(0, nvmPath.size() - 4) + ".sfm";
			if (!HelperMeshRecon::executeNVM2SFM(nvmPath, sfmPath, ConfigurationDialog::getBoundingBox()))
			{
				log->write("Error during NVM2SFM", true, true);
				delete log;
				return;
			}
			log->write("Finished NVM2SFM", true, true);
			log->addSeparator();
			log->write("Started MeshRecon", true, true);
			if (!HelperMeshRecon::executeMeshRecon(sfmPath, sfmPath, ConfigurationDialog::getLevelOfDetails()))
			{
				log->write("Error during MeshRecon", true, true);
				delete log;
				return;
			}
			log->write("Finished MeshRecon", true, true);
			log->addSeparator();
		}
		meshPath = nvmPath.substr(0, nvmPath.size() - 4) + "_refine.ply";
		if (!cbTexturize->IsChecked())
		{
			if (!disableEndModal)
			{
				delete log;
				EndModal(wxID_OK);
			}
		}
	}
	if (cbGenerateMesh->IsChecked() && cbTexturize->IsChecked())
	{
		//This call is here because it does the nvm checking
		//TexRecon
		if (ConfigurationDialog::getForceTexReconTexture())
		{
			log->write("Started TexRecon", true, true);
			std::string texReconTextureParameters = "2 " + Utils::preparePath(meshPath) + " " +
				Utils::preparePath(nvmPath) + " " +
				Utils::preparePath(pickerOutputTexture->GetPath().ToStdString()) +
				" " +
				ConfigurationDialog::getTexReconParameters();
			std::string texReconParams = Utils::preparePath(Utils::getExecutionPath() + "/projetomesh/MeshRecon_plus_TexRecon.exe") + " martintalationpathNotUsed " + texReconTextureParameters;
			if (!Utils::startProcess(texReconParams))
			{
				wxLogError("Error with TexRecon");
				log->write("Error during TexRecon", true, true);
				delete log;
				return;
			}
			log->write("Finished TexRecon", true, true);
		}
		else//AVTexturing
		{
			log->write("Started AVTexturing", true, true);
			std::string ouputMeshPath = pickerOutputTexture->GetPath().ToStdString();
			ouputMeshPath = ouputMeshPath.substr(0, ouputMeshPath.size() - 4) + "_textured.obj";
			if (!runAVTexturization(meshPath, nvmPath, ouputMeshPath))
			{
				wxLogError("Error with AVTexturing");
				log->write("Error during AVTexturing", true, true);
				delete log;
				return;
			}
			log->write("Finished AVTexturing", true, true);
		}
		
		log->addSeparator();
		if (!disableEndModal)
		{
			delete log;
			EndModal(wxID_OK);
		}
	}
	else if (cbTexturize->IsChecked())
	{
		nvmPath = pickerNVM->GetPath().ToStdString();
		meshPath = pickerMesh->GetPath().ToStdString();
		//TexRecon
		if (ConfigurationDialog::getForceTexReconTexture())
		{
			log->write("Started TexRecon", true, true);
			std::string texReconTextureParameters = "2 " + Utils::preparePath(meshPath) + " " +
				Utils::preparePath(nvmPath) + " " +
				Utils::preparePath(pickerOutputTexture->GetPath().ToStdString()) +
				" " +
				ConfigurationDialog::getTexReconParameters();
			std::string texReconParams = Utils::preparePath(Utils::getExecutionPath() + "/projetomesh/MeshRecon_plus_TexRecon.exe") + " martintalationpathNotUsed " + texReconTextureParameters;
			if (!Utils::startProcess(texReconParams))
			{
				wxLogError("Error with TexRecon");
				log->write("Error during TexRecon", true, true);
				delete log;
				return;
			}
			log->write("Finished TexRecon", true, true);
			log->addSeparator();
			meshPath = pickerOutputTexture->GetPath().ToStdString();
			meshPath = meshPath.substr(0, meshPath.size() - 4) + "_textured.obj";
			if (!Utils::exists(meshPath))
			{
				wxLogError("No textured mesh generated with TexRecon");
				log->write("No textured mesh generated with TexRecon", true, true);
				delete log;
				return;
			}
		}
		else//AVTexturing
		{
			log->write("Started AVTexturing", true, true);
			std::string ouputMeshPath = pickerOutputTexture->GetPath().ToStdString();
			ouputMeshPath = ouputMeshPath.substr(0, ouputMeshPath.size() - 4) + "_textured.obj";
			if (!runAVTexturization(meshPath, nvmPath, ouputMeshPath))
			{
				wxLogError("Error with AVTexturing");
				log->write("Error during AVTexturing", true, true);
				delete log;
				return;
			}
			log->write("Finished AVTexturing", true, true);
		}
		if (!disableEndModal)
		{
			delete log;
			EndModal(wxID_OK);
		}
	}
	if (disableEndModal)
	{
		delete log;
	}
}

void ReconstructionDialog::OnBtConfig(wxCommandEvent & WXUNUSED)
{
	ConfigurationDialog* config = new ConfigurationDialog(this);
	config->ShowModal();
	delete config;
}

void ReconstructionDialog::OnBtBatchAdd(wxCommandEvent & WXUNUSED)
{
	if (cbGenerateMesh->IsChecked())
	{
		if (!cbTexturize->IsChecked())
		{
			if (pickerImages->GetPath() == "" || pickerOutputMesh->GetPath() == "")
			{
				wxLogError("Please fulfill all the fields correctly.");
				return;
			}
		}
		else
		{
			if (pickerImages->GetPath() == "" || pickerOutputTexture->GetPath() == "")
			{
				wxLogError("Please fulfill all the fields correctly.");
				return;
			}
		}
	}
	else if (cbTexturize->IsChecked())
	{
		if (pickerNVM->GetPath() == "" || pickerMesh->GetPath() == "" || pickerOutputTexture->GetPath() == "")
		{
			wxLogError("Please fulfill all the fields correctly.");
			return;
		}
	}
	else
	{
		wxLogError("Please select at least one checkbox.");
		return;
	}
	BatchItem b;
	b.generateMesh = cbGenerateMesh->GetValue();
	b.generateTexture = cbTexturize->GetValue();
	b.imagePath = pickerImages->GetPath();
	b.meshPath = pickerMesh->GetPath();
	b.nvmPath = pickerNVM->GetPath();
	if (b.generateTexture)
	{
		b.outputPath = pickerOutputTexture->GetPath();
	}
	else
	{
		b.outputPath = pickerOutputMesh->GetPath();
	}
	batchItems.push_back(b);
	textQueueSize->SetLabelText("Queue size: " + std::to_string(batchItems.size()));
	cbGenerateMesh->SetValue(false);
	cbTexturize->SetValue(false);
	pickerImages->SetPath("");
	pickerMesh->SetPath("");
	pickerNVM->SetPath("");
	pickerOutputTexture->SetPath("");
	pickerOutputMesh->SetPath("");
	OnCheckBoxs((wxCommandEvent)NULL);
}

void ReconstructionDialog::OnBtBatchStart(wxCommandEvent & WXUNUSED)
{
	disableEndModal = true;
	wxProgressDialog* progDialog = new wxProgressDialog("Reconstructions", "Reconstruction 0", batchItems.size(), this, wxPD_SMOOTH | wxPD_AUTO_HIDE | wxPD_APP_MODAL);
	int i = 0;
	wxBeginBusyCursor();
	for (auto b : batchItems)
	{
		progDialog->Update(i, "Reconstruction " + std::to_string(i));
		i++;
		cbGenerateMesh->SetValue(b.generateMesh);
		cbTexturize->SetValue(b.generateTexture);
		pickerImages->SetPath(b.imagePath);
		if (b.generateTexture)
		{
			pickerMesh->SetPath(b.meshPath);
			pickerNVM->SetPath(b.nvmPath);
			pickerOutputTexture->SetPath(b.outputPath);
		}
		else
		{
			pickerOutputMesh->SetPath(b.outputPath);
		}
		OnOK((wxCommandEvent)NULL);
	}
	//Clear the fields
	cbGenerateMesh->SetValue(false);
	cbTexturize->SetValue(false);
	pickerImages->SetPath("");
	pickerMesh->SetPath("");
	pickerNVM->SetPath("");
	pickerOutputTexture->SetPath("");
	pickerOutputMesh->SetPath("");
	OnCheckBoxs((wxCommandEvent)NULL);
	wxEndBusyCursor();
	delete progDialog;
	disableEndModal = false;
}

void ReconstructionDialog::OnCheckBoxs(wxCommandEvent & WXUNUSED)
{
	if (cbGenerateMesh->IsChecked() && cbTexturize->IsChecked())
	{
		textImages->Enable();
		pickerImages->Enable();
		textOutputMesh->Enable(false);
		pickerOutputMesh->Enable(false);

		textMesh->Enable(false);
		pickerMesh->Enable(false);
		textNVM->Enable(false);
		pickerNVM->Enable(false);
		textOutputTexture->Enable();
		pickerOutputTexture->Enable();
	}
	else
	{
		textImages->Enable(cbGenerateMesh->IsChecked());
		pickerImages->Enable(cbGenerateMesh->IsChecked());
		textOutputMesh->Enable(cbGenerateMesh->IsChecked());
		pickerOutputMesh->Enable(cbGenerateMesh->IsChecked());

		textMesh->Enable(cbTexturize->IsChecked());
		pickerMesh->Enable(cbTexturize->IsChecked());
		textNVM->Enable(cbTexturize->IsChecked());
		pickerNVM->Enable(cbTexturize->IsChecked());
		textOutputTexture->Enable(cbTexturize->IsChecked());
		pickerOutputTexture->Enable(cbTexturize->IsChecked());
	}
}

void ReconstructionDialog::OnDirPickerImages(wxFileDirPickerEvent & WXUNUSED)
{
	wxBeginBusyCursor();
	if (!testImageFolder(pickerImages->GetPath()))
	{
		wxEndBusyCursor();
		pickerImages->SetPath("");
		return;
	}
	wxEndBusyCursor();
	//We need to go back a folder to avoid contaminating the image folder
	wxString reconsPath = pickerImages->GetPath();
	reconsPath = reconsPath.substr(0, reconsPath.find_last_of("\\")) + "\\Reconstruction";
	if (pickerOutputTexture->IsEnabled() && pickerOutputTexture->GetPath() == "" || pickerOutputMesh->IsEnabled() && pickerOutputMesh->GetPath() == "")
	{
		wxString reconsPathTemp = reconsPath;
		int folderNum = 0;
		while (_mkdir(reconsPathTemp))
		{
			folderNum++;
			reconsPathTemp = reconsPath;
			reconsPathTemp << "_" << folderNum;
			if (folderNum == 100)//If something go really wrong
			{
				return;
			}
		}
		if (pickerOutputTexture->IsEnabled())
		{
			pickerOutputTexture->SetPath(reconsPathTemp + "\\mesh.obj");
		}
		else if (pickerOutputMesh->IsEnabled())
		{
			pickerOutputMesh->SetPath(reconsPathTemp + "\\mesh.ply");
		}
	}
}

void ReconstructionDialog::OnDirPickerNVM(wxFileDirPickerEvent & WXUNUSED)
{
	wxBeginBusyCursor();
	std::string nvmPath = pickerNVM->GetPath().ToStdString();
	std::vector<std::string> imagePaths;
	if (ImageIO::getCamerasFileImagePaths(nvmPath, imagePaths))
	{
		if (!ImageIO::getImagePathsExist(imagePaths))
		{
			std::string newImageDir;
			wxMessageBox("Please select the correct image folder", "Error", wxICON_ERROR);
			wxDirDialog* imageDialog = new wxDirDialog(this, "Choose the image folder", "", wxDD_DEFAULT_STYLE);
			//Just stop when all the images are in the selected folder
			bool insist = 0;
			do
			{
				if (insist)
				{
					wxMessageBox("The folder still wrong, please select the correct image folder", "Warning", wxICON_WARNING);
				}
				insist = 1;
				if (imageDialog->ShowModal() == wxID_OK)
				{
					newImageDir = imageDialog->GetPath();
				}
				else
				{
					wxLogError("NVM with wrong image paths");
					pickerNVM->SetPath("");
					delete imageDialog;
					return;
				}

			} while (!ImageIO::getImagePathsExist(imagePaths, newImageDir));
			delete imageDialog;
			ImageIO::replaceCamerasFileImageDir(nvmPath, newImageDir);
		}
	}
	else
	{
		wxLogError("Error while loading this NVM file");
		pickerNVM->SetPath("");
	}
	wxEndBusyCursor();
}

void ReconstructionDialog::OnDirPickerOutput(wxFileDirPickerEvent & WXUNUSED)
{
	if (cbTexturize->IsChecked())
	{
		if (pickerOutputTexture->GetPath().find(' ', 0) != wxNOT_FOUND)
		{
			wxLogError("Do not select a folder/filename with spaces in the name");
			pickerOutputTexture->SetPath("");
		}
	}
	else
	{
		if (pickerOutputMesh->GetPath().find(' ', 0) != wxNOT_FOUND)
		{
			wxLogError("Do not select a folder/filename with spaces in the name");
			pickerOutputMesh->SetPath("");
		}
	}

}

bool ReconstructionDialog::testImageFolder(wxString imagePath)
{
	//COLMAP does not accept spaces or special characters in the folder path
	if (imagePath.find(' ', 0) != wxNOT_FOUND)
	{
		wxLogError("Do not select a folder with spaces in the name");
		return false;
	}
	if (imagePath.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_\\/:-()") != std::string::npos)
	{
		wxLogError("Do not select a folder with special characters in the name");
		return false;
	}
	wxDir* directory = new wxDir(imagePath);
	if (!directory->IsOpened())
	{
		wxLogError("Could not open the folder");
		return false;
	}
	wxArrayString* files = new wxArrayString();
	directory->GetAllFiles(imagePath, files, wxEmptyString, wxDIR_FILES);
	if (files->size() == 0)
	{
		wxLogError("There is no image in the folder");
		return false;
	}
	wxString extension;
	int qtdImages = 0;
	int width, height;
	for (int i = 0; i < files->size(); i++)
	{
		extension = files->Item(i).SubString(files->Item(i).size() - 4, files->Item(i).size());
		if (extension == ".jpg" || extension == ".JPG")
		{
			if (ImageIO::getImageSize(files->Item(i).ToStdString(), width, height))
			{
				if (qtdImages == 0)
				{
					heightDefault = height;
					widthDefault = width;
				}
				if (height != heightDefault || width != widthDefault)
				{
					wxLogError("The images must have the same resolution");
					return false;
				}
				qtdImages++;
			}
			else
			{
				wxLogError("Error on load the image");
				return false;
			}
		}
	}
	if (qtdImages <= 1)
	{
		wxLogError("You need at least two .jpg images");
		return false;
	}
	return true;
}

//AVTexturing
MeshAux * ReconstructionDialog::loadMeshToTexturize(std::string meshPath)
{
	MeshAux* mesh = nullptr;
	vtkNew<vtkPLYReader> plyReader;
	plyReader->SetFileName(meshPath.c_str());
	plyReader->Update();
	vtkSmartPointer<vtkPolyData> polyData = plyReader->GetOutput();
	if (polyData)
	{
		mesh = new MeshAux();
		vtkSmartPointer<vtkPoints> points = polyData->GetPoints();
		vtkSmartPointer<vtkFloatArray> normals = nullptr;
		if (polyData->GetPointData()->GetNormals())
		{
			normals = vtkFloatArray::SafeDownCast(polyData->GetPointData()->GetNormals());
		}
		size_t numberOfPoints = points->GetNumberOfPoints();
		mesh->points.resize(numberOfPoints);
		#pragma omp parallel for
		for (int i = 0; i < numberOfPoints; i++)
		{
			VertexAux* v = new VertexAux();
			double point[3], normal[3];
			points->GetPoint(i, point);
			if (normals)
			{
				normals->GetTuple(i, normal);
			}
			for (size_t j = 0; j < 3; j++)
			{
				v->p[j] = point[j];
				if (normals)
				{
					v->normal[j] = normal[j];
				}
			}
			v->vertex_index = i;
			mesh->points[i] = v;
		}
		vtkSmartPointer<vtkCellArray> faces = polyData->GetPolys();
		if (faces)
		{
			size_t numberOfFaces = faces->GetNumberOfCells();
			mesh->polygons.resize(numberOfFaces);
			for (size_t i = 0; i < numberOfFaces; i++)
			{
				vtkNew<vtkIdList> idxList;
				faces->GetNextCell(idxList);
				FaceAux* face = new FaceAux();
				for (int j = 0; j < idxList->GetNumberOfIds(); j++)
				{
					face->vertices.push_back(mesh->points[idxList->GetId(j)]);
				}
				mesh->polygons[i] = face;
			}
		}
	}
	return mesh;
}
void ReconstructionDialog::createPointVisibilityData(MeshAux* mesh, CameraParameters* cameraParameters, std::vector<Camera*> cameras)
{
	if (!mesh)
	{
		return;
	}
	//Creating collision model
	Claudette::CollisionModel3D* collisionModel = new Claudette::CollisionModel3D();
	for (auto face : mesh->polygons)
	{
		if (face->vertices.size() != 3)
		{
			continue;
		}
		collisionModel->addTriangle(face->vertices.at(0)->p, face->vertices.at(1)->p, face->vertices.at(2)->p);
	}
	collisionModel->finalize();
	//Create the landmarks
	int sizeLandmarks = mesh->points.size();
	cameraParameters->landmarks.resize(sizeLandmarks);
	vtkNew<vtkPoints> pointsMesh;
	pointsMesh->SetNumberOfPoints(sizeLandmarks);
#pragma omp parallel for
	for (int i = 0; i < sizeLandmarks; i++)
	{
		LandmarkAux* landmark = new LandmarkAux();
		for (size_t j = 0; j < 3; j++)
		{
			landmark->X[j] = mesh->points[i]->p[j];
		}
		cameraParameters->landmarks[i] = landmark;
		pointsMesh->InsertPoint(i, landmark->X);
	}
	//Create a vtkPolyData to select the points using the frustum
	vtkNew<vtkPolyData> meshPolyData;
	meshPolyData->SetPoints(pointsMesh);
	for (size_t i = 0; i < cameras.size(); i++)
	{
		Camera* cam = cameras[i];
		double* cameraOrigin = cam->cameraPoints[0];

		double bounds[6];
		meshPolyData->GetBounds(bounds);
		
		double maxDistance = -1;
		//Z
		for (size_t i = 4; i < 6; i++)
		{
			//Y
			for (size_t j = 2; j < 4; j++)
			{
				//X
				for (size_t k = 0; k < 2; k++)
				{
					double boundPoint[3];
					boundPoint[0] = bounds[k];
					boundPoint[1] = bounds[j];
					boundPoint[2] = bounds[i];
					double dist = vtkMath::Distance2BetweenPoints(cameraOrigin, boundPoint);
					if (maxDistance < dist)
					{
						maxDistance = dist;
					}
				}
			}
		}
		maxDistance = std::sqrt(maxDistance);

		vtkNew<vtkSelectEnclosedPoints> selectEnclosedPoints;
		selectEnclosedPoints->SetSurfaceData(cam->getClosedFrustum(maxDistance));
		selectEnclosedPoints->SetInputData(meshPolyData);
		selectEnclosedPoints->SetTolerance(.00000001);
		selectEnclosedPoints->Update();
		#pragma omp parallel for
		for (int idxLand = 0; idxLand < sizeLandmarks; idxLand++)
		{
			if (selectEnclosedPoints->IsInside(idxLand))
			{
				double* landPoint = cameraParameters->landmarks[idxLand]->X;
				Claudette::RayCollisionTest rayTest;
				rayTest.setRayOrigin(cameraOrigin[0], cameraOrigin[1], cameraOrigin[2]);
				rayTest.setRaySearch(Claudette::RayCollisionTest::SearchClosestTriangle);
				double norm = 0;
				double* direction = new double[3];
				for (unsigned int i = 0; i < 3; i++)
				{
					direction[i] = landPoint[i] - cameraOrigin[i];
					norm += direction[i] * direction[i];
				}
				norm = sqrt(norm);
				for (unsigned int i = 0; i < 3; i++)
				{
					direction[i] /= norm;
				}
				rayTest.setRayDirection(direction[0], direction[1], direction[2]);
				if (collisionModel->rayCollision(&rayTest))
				{
					const float* collisionPoint = rayTest.point();
					if (pow(collisionPoint[0] - landPoint[0], 2) + pow(collisionPoint[1] - landPoint[1], 2) + pow(collisionPoint[2] - landPoint[2], 2) < 0.000001)//0.001^2
					{
						cameraParameters->landmarks[idxLand]->observations.push_back(i);
					}
				}
				delete direction;
			}
		}
	}
}
bool ReconstructionDialog::runAVTexturization(std::string meshPath, std::string nvmPath, std::string outputPath)
{
	wxBeginBusyCursor();
	MeshAux* mesh = loadMeshToTexturize(meshPath);
	std::vector<Camera*> cameras;
	if (!ImageIO::loadCameraParameters(nvmPath, cameras))
	{
		wxLogError(wxString("Error loading " + Utils::getFileName(nvmPath)));
		return 0;
	}
	CameraParameters* cameraParameters = new CameraParameters();
	createPointVisibilityData(mesh, cameraParameters, cameras);
	for (size_t i = 0; i < cameras.size(); i++)
	{
		//View
		ViewAux* view = new ViewAux();
		view->viewId = i;
		view->poseId = i;
		view->intrinsicId = i;
		view->path = cameras[i]->filePath;
		view->width = cameras[i]->width;
		view->height = cameras[i]->height;
		cameraParameters->views.emplace(view->viewId, view);
		//Intrinsic
		IntrinsicAux* intrinsic = new IntrinsicAux();
		intrinsic->intrinsicId = i;
		intrinsic->width = cameras[i]->width;
		intrinsic->height = cameras[i]->height;
		intrinsic->pxFocalLength = cameras[i]->getFocalX();
		intrinsic->principalPoint[0] = intrinsic->width / 2.f;
		intrinsic->principalPoint[1] = intrinsic->height / 2.f;
		intrinsic->distortionParams.push_back(0);
		intrinsic->distortionParams.push_back(0);
		intrinsic->distortionParams.push_back(0);
		cameraParameters->instrinsics.emplace(intrinsic->intrinsicId, intrinsic);
		//Pose
		PoseAux* pose = new PoseAux();
		pose->poseId = i;
		vtkSmartPointer<vtkMatrix4x4> matrixRt = cameras[i]->getMatrixRt();
		for (size_t j = 0; j < 3; j++)
		{
			for (size_t k = 0; k < 3; k++)
			{
				pose->rotation[j * 3 + k] = matrixRt->GetElement(j, k);
			}
			pose->center[j] = cameras[i]->cameraPoints[0][j];
		}
		cameraParameters->poses.emplace(pose->poseId, pose);
	}
	AVTexturing* AVText = new AVTexturing();
	OptionsAVTexturing* options = new OptionsAVTexturing();
	AVText->applyTexture(mesh, cameraParameters, options, outputPath);
	delete mesh, cameraParameters, options, AVText;
	wxEndBusyCursor();
	if (!Utils::exists(outputPath))
	{
		return 0;
	}
	return 1;
}
