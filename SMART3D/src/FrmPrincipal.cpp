#include "FrmPrincipal.h"

#include <sstream>
//VTK
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkOutputWindow.h>
#include <vtkAxesActor.h>
#include <vtkLandmarkTransform.h>
#include <vtkTransform.h>
#include <vtkUnsignedCharArray.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPLYWriter.h>
#include <vtkPLYReader.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkMapper.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkCleanPolyData.h>
#include <vtkFeatureEdges.h>
#include <vtkAssemblyPath.h>
#include <vtkPropPicker.h>
#include <vtkNew.h>
//WX
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/splitter.h>
#include <wx/html/helpctrl.h>
#include <wx/dir.h>
#include <wx/progdlg.h>
#include <wx/numdlg.h> 
//Tools
#include "DistanceTool.h"
#include "AngleTool.h"
#include "ElevationTool.h"
#include "ViewTool.h"
#include "AlignWithAxisTool.h"
#include "VolumeTool.h"
#include "CheckCamTool.h"
#include "DeleteTool.h"
#include "UpdateCameraTool.h"
#include "AlignTool.h"
#include "SnapshotTool.h"
#include "AxisWidget.h"
//Dialogs
#include "ICPDialog.h"
#include "TransformDialog.h"
#include "SettingsDialog.h"
#include "ReconstructionDialog.h"
#include "PoissonDialog.h"
#include "SSDDialog.h"
#include "AboutDialog.h"
#include "VRDialog.h"
#include "M3C2Dialog.h"

#include "PoissonRecon.h"
#include "SSDRecon.h"
#include "SurfaceTrimmer.h"
#include "InteractorStyle.h"
#include "wxVTKRenderWindowInteractor.h"
#include "OutputErrorWindow.h"
#include "GPSData.h"
#include "Calibration.h"
#include "Volume.h"
#include "Draw.h"
#include "Utils.h"
#include "Mesh.h"
#include "Camera.h"
#include "MeshIO.h"
#include "ImageIO.h"

//#define DEBUG_MART

BEGIN_EVENT_TABLE(FrmPrincipal, wxFrame)
EVT_MENU(idMenuOpen, FrmPrincipal::OnMenuOpen)
EVT_MENU(idMenuOpenCameras, FrmPrincipal::OnMenuOpenCameras)
EVT_MENU(idMenuExportMesh, FrmPrincipal::OnMenuExportMesh)
EVT_MENU(idMenuExportCameras, FrmPrincipal::OnMenuExportCameras)
EVT_MENU(idMenuExportGeoRegisteredPointCloud, FrmPrincipal::OnMenuExportGeoRegisteredPointCloud)
EVT_MENU(idMenuExportPoisson, FrmPrincipal::OnMenuExportPoisson)
EVT_MENU(idMenuTransform, FrmPrincipal::OnMenuTransform)
EVT_MENU(idMenuUpdateCamera, FrmPrincipal::OnMenuUpdateCamera)
EVT_MENU(idMenuSnapshot, FrmPrincipal::OnMenuSnapshot)
EVT_MENU(idMenuShowAxis, FrmPrincipal::OnMenuShowAxis)
EVT_MENU(idMenuShowViews, FrmPrincipal::OnMenuShowViews)
EVT_MENU(idMenuFlyToPoint, FrmPrincipal::OnMenuFlyToPoint)
EVT_MENU(idMenuSettings, FrmPrincipal::OnMenuSettings)
EVT_MENU(idMenuICP, FrmPrincipal::OnMenuICP)
EVT_MENU(idMenuAlignTool, FrmPrincipal::OnMenuAlignTool)
EVT_MENU(idMenuM3C2, FrmPrincipal::OnMenuM3C2)
EVT_MENU(idMenuSurfaceTrimmer, FrmPrincipal::OnMenuSurfaceTrimmer)
EVT_MENU(idMenuFastQuadricSimplification, FrmPrincipal::OnMenuFastQuadricSimplification)
EVT_MENU(idMenuPoisson, FrmPrincipal::OnMenuPoisson)
EVT_MENU(idMenuSSDRecon, FrmPrincipal::OnMenuSSDRecon)
EVT_MENU(idMenuReconstructClusters, FrmPrincipal::OnMenuReconstructClusters)
EVT_MENU(idMenuHelp, FrmPrincipal::OnMenuHelp)
EVT_MENU(idMenuAbout, FrmPrincipal::OnMenuAbout)
EVT_TOOL(idToolOpen, FrmPrincipal::OnMenuOpen)
EVT_TOOL(idToolDelete, FrmPrincipal::OnToolDelete)
EVT_TOOL(idToolDebug, FrmPrincipal::OnToolDebug)
EVT_TOOL(idToolPoints, FrmPrincipal::OnToolPoints)
EVT_TOOL(idToolWireframe, FrmPrincipal::OnToolWireframe)
EVT_TOOL(idToolSurface, FrmPrincipal::OnToolSurface)
EVT_TOOL(idToolMeasure, FrmPrincipal::OnToolMeasure)
EVT_TOOL(idToolAngle, FrmPrincipal::OnToolAngle)
EVT_TOOL(idToolVolume, FrmPrincipal::OnToolVolume)
EVT_TOOL(idToolElevation, FrmPrincipal::OnToolElevation)
EVT_TOOL(idToolCalibration, FrmPrincipal::OnToolCalibration)
EVT_TOOL(idToolCheckCamVisibility, FrmPrincipal::OnToolCheckCamVisibility)
EVT_TOOL(idToolStartReconstruction, FrmPrincipal::OnToolStartReconstruction)
EVT_TOOL(idToolSnapshot, FrmPrincipal::OnToolSnapshot)
EVT_TOOL(idToolLight, FrmPrincipal::OnToolLight)
EVT_TOOL(idToolDeletePointsFaces, FrmPrincipal::OnToolDeletePointsFaces)
EVT_TOOL(idToolVR, FrmPrincipal::OnToolVR)
EVT_CLOSE(FrmPrincipal::OnClose)
EVT_SIZE(FrmPrincipal::OnSize)
EVT_SPLITTER_DCLICK(idSplitterWindow, FrmPrincipal::OnSplitterDClick)
EVT_SPLITTER_SASH_POS_CHANGED(idSplitterWindow, FrmPrincipal::OnSashPosChanged)
EVT_CHAR_HOOK(FrmPrincipal::OnKeyPress)
EVT_DROP_FILES(FrmPrincipal::OnDropFiles)
END_EVENT_TABLE()

FrmPrincipal::FrmPrincipal(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxFrame(parent, id, title, pos, size, style)
{
	this->SetIcon(wxICON(AAAAAPROGRAM));
	//Help
	helpController = new wxHtmlHelpController(this, wxHF_TOOLBAR | wxHF_CONTENTS | wxHF_INDEX | wxHF_SEARCH | wxHF_BOOKMARKS | wxHF_PRINT);
	//VTK
	outputErrorWindow = vtkSmartPointer<OutputErrorWindow>::New();
	vtkOutputWindow::SetInstance(outputErrorWindow);
	//WX
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	//Menu
	menuBar = new wxMenuBar(0);
	//File
	menuFile = new wxMenu();
	menuFile->Append(new wxMenuItem(menuFile, idMenuOpen, "Open..."));
	menuFile->Append(new wxMenuItem(menuFile, idMenuOpenCameras, "Open cameras..."));
	menuFile->AppendSeparator();
	menuFile->Append(new wxMenuItem(menuFile, idMenuExportMesh, "Export mesh..."));
	menuFile->Append(new wxMenuItem(menuFile, idMenuExportCameras, "Export cameras..."));
	menuFile->Append(new wxMenuItem(menuFile, idMenuExportGeoRegisteredPointCloud, "Export geo-registered point cloud..."));
	menuFile->Append(new wxMenuItem(menuFile, idMenuExportPoisson, "Export poisson/SSD..."));
	menuFile->AppendSeparator();
	menuFile->Append(new wxMenuItem(menuFile, idMenuTransform, "Tranform..."));
	menuItemUpdateCamera = menuFile->AppendCheckItem(idMenuUpdateCamera, "Update camera...");
	menuFile->Append(new wxMenuItem(menuFile, idMenuSnapshot, "Take a snapshot..."));
	menuFile->AppendSeparator();
	menuItemShowAxis = menuFile->AppendCheckItem(idMenuShowAxis, "Show Axis");
	menuItemShowViews =menuFile->AppendCheckItem(idMenuShowViews, "Show Views");
	menuItemFlyToPoint = menuFile->AppendCheckItem(idMenuFlyToPoint, "Fly to point");
	menuFile->AppendSeparator();
	menuFile->Append(new wxMenuItem(menuFile, idMenuSettings, "Settings..."));
	//Edit
	menuEdit = new wxMenu();
	menuRepresentation = new wxMenu();
	menuRepresentation->AppendRadioItem(idToolPoints, "Points");
	menuRepresentation->AppendRadioItem(idToolWireframe, "Wireframe");
	menuRepresentation->AppendRadioItem(idToolSurface, "Surface");
	menuRepresentation->Check(idToolSurface, true);
	menuEdit->AppendSubMenu(menuRepresentation, "Representations");
	menuEdit->AppendSeparator();
	menuEdit->Append(idToolDelete, "Delete");
	menuItemToolDeletePoints = menuEdit->AppendCheckItem(idToolDeletePointsFaces, "Delete points/faces");
	//Filters
	menuFilters = new wxMenu();
	wxMenu* menuRegistration = new wxMenu();
	menuRegistration->Append(new wxMenuItem(menuRegistration, idMenuICP, "ICP..."));
	menuItemAlignTool = menuRegistration->AppendCheckItem(idMenuAlignTool, "Align meshs");
	menuRegistration->AppendSeparator();
	menuRegistration->Append(new wxMenuItem(menuRegistration, idMenuM3C2, "M3C2..."));
	menuFilters->AppendSubMenu(menuRegistration, "Registration");
	wxMenu* menuSimplification = new wxMenu();
	menuSimplification->Append(new wxMenuItem(menuSimplification, idMenuSurfaceTrimmer, "Surface trimmer..."));
	menuSimplification->Append(new wxMenuItem(menuSimplification, idMenuFastQuadricSimplification, "Fast quadric simplification..."));
	menuFilters->AppendSubMenu(menuSimplification, "Simplification");
	wxMenu* menuReconstruction = new wxMenu();
	menuReconstruction->Append(new wxMenuItem(menuReconstruction, idMenuPoisson, "Poisson..."));
	menuReconstruction->Append(new wxMenuItem(menuReconstruction, idMenuSSDRecon, "SSD..."));
	menuReconstruction->AppendSeparator();
	menuReconstruction->Append(new wxMenuItem(menuReconstruction, idMenuReconstructClusters, "Reconstruct clusters..."));
	menuFilters->AppendSubMenu(menuReconstruction, "Reconstruction");
	//Views
	menuView = new wxMenu();
	menuItemToolCheckCamVisibility = menuView->AppendCheckItem(idToolCheckCamVisibility, "Check camera visibility");
	menuView->AppendSeparator();
	menuView->Append(idToolLight, "Light");
	//Tools
	menuTools = new wxMenu();
	menuItemToolMeasure = menuTools->AppendCheckItem(idToolMeasure, "Distance");
	menuItemToolAngle = menuTools->AppendCheckItem(idToolAngle, "Angle");
	menuItemToolVolume = menuTools->AppendCheckItem(idToolVolume, "Compute volume");
	menuItemToolElevation = menuTools->AppendCheckItem(idToolElevation, "Elevation");
	menuTools->AppendSeparator();
	menuTools->Append(new wxMenuItem(menuTools, idToolSnapshot, "Snapshot..."));
	//Help
	menuHelp = new wxMenu();
	menuHelp->Append(new wxMenuItem(menuHelp, idMenuHelp, "Help..."));
	menuHelp->AppendSeparator();
	menuHelp->Append(new wxMenuItem(menuHelp, idMenuAbout, "About..."));

	menuBar->Append(menuFile, "File");
	menuBar->Append(menuEdit, "Edit");
	menuBar->Append(menuFilters, "Filters");
	menuBar->Append(menuTools, "Tools");
	menuBar->Append(menuView, "View");
	menuBar->Append(menuHelp, "Help");

	//Bitmpas
	bmpToolMeasureOFF = wxICON(ICON_MEASURE_OFF);
	bmpToolMeasureON = wxICON(ICON_MEASURE_ON);
	bmpToolAngleOFF = wxICON(ICON_ANGLE_OFF);
	bmpToolAngleON = wxICON(ICON_ANGLE_ON);
	bmpToolCalcVolumeOFF = wxICON(ICON_CALC_VOLUME_OFF);
	bmpToolCalcVolumeON = wxICON(ICON_CALC_VOLUME_ON);
	bmpToolElevationOFF = wxICON(ICON_ELEVATION_OFF);
	bmpToolElevationON = wxICON(ICON_ELEVATION_ON);
	bmpToolCheckCamVisibilityOFF = wxICON(ICON_CHECKCAM_OFF);
	bmpToolCheckCamVisibilityON = wxICON(ICON_CHECKCAM_ON);
	bmpToolDeletePointsFacesOFF = wxICON(ICON_DELETE_POINTS_OFF);
	bmpToolDeletePointsFacesON = wxICON(ICON_DELETE_POINTS_ON);
	bmpToolVROFF = wxICON(ICON_VR_OFF);
	bmpToolVRON = wxICON(ICON_VR_ON);

	//ToolBar
	toolBar = this->CreateToolBar(wxTB_HORIZONTAL, wxID_ANY);
	toolBar->AddTool(idToolOpen, "ToolOpen", wxICON(ICON_OPEN), wxNullBitmap, wxITEM_NORMAL, "Open...");
	toolBar->AddTool(idToolDelete, "ToolDelete", wxICON(ICON_DELETE), wxNullBitmap, wxITEM_NORMAL, "Delete");
	toolBar->AddSeparator();
#ifdef DEBUG_MART
	toolBar->AddTool(idToolDebug, "ToolDebug", wxICON(ICON_DEBUG), wxNullBitmap, wxITEM_NORMAL, "Debug");
	toolBar->AddSeparator();
#endif // DEBUG_MART
	toolBar->AddTool(idToolPoints, "ToolPoints", wxICON(ICON_POINTS), wxNullBitmap, wxITEM_NORMAL, "Points");
	toolBar->AddTool(idToolWireframe, "ToolWireframe", wxICON(ICON_WIREFRAME), wxNullBitmap, wxITEM_NORMAL, "Wireframe");
	toolBar->AddTool(idToolSurface, "ToolSurface", wxICON(ICON_SURFACE), wxNullBitmap, wxITEM_NORMAL, "Surface");
	toolBar->AddSeparator();
	toolMeasure = toolBar->AddTool(idToolMeasure, "ToolMeasure", bmpToolMeasureOFF, wxNullBitmap, wxITEM_NORMAL, "Distance");
	toolAngle = toolBar->AddTool(idToolAngle, "ToolAngle", bmpToolAngleOFF, wxNullBitmap, wxITEM_NORMAL, "Angle");
	toolCalcVolume = toolBar->AddTool(idToolVolume, "ToolCalcVolume", bmpToolCalcVolumeOFF, wxNullBitmap, wxITEM_NORMAL, "Compute volume");
	toolElevation = toolBar->AddTool(idToolElevation, "ToolElevation", bmpToolElevationOFF, wxNullBitmap, wxITEM_NORMAL, "Elevation");
	toolBar->AddTool(idToolCalibration, "ToolCalibration", wxICON(ICON_CALIBRATION), wxNullBitmap, wxITEM_NORMAL, "Calibration...");
	toolCheckCamVisibility = toolBar->AddTool(idToolCheckCamVisibility, "ToolCheckCamVisibility", bmpToolCheckCamVisibilityOFF, wxNullBitmap, wxITEM_NORMAL, "Check camera visibility");
	toolBar->AddSeparator();
	toolBar->AddTool(idToolStartReconstruction, "ToolStartReconstruction", wxICON(ICON_START_RECONS), wxNullBitmap, wxITEM_NORMAL, "Start reconstruction...");
	toolBar->AddTool(idToolSnapshot, "ToolSnapshot", wxICON(ICON_SNAPSHOT), wxNullBitmap, wxITEM_NORMAL, "Take a snapshot...");

	toolBar->AddTool(idToolLight, "ToolLight", wxICON(ICON_LIGHT), wxNullBitmap, wxITEM_NORMAL, "Light");
	toolDeletePoints = toolBar->AddTool(idToolDeletePointsFaces, "DeletePointsFaces", bmpToolDeletePointsFacesOFF, wxNullBitmap, wxITEM_NORMAL, "Delete points/faces");
	toolVR = toolBar->AddTool(idToolVR, "VR", bmpToolVROFF, wxNullBitmap, wxITEM_NORMAL, "Send to VR...");

	//InteractorStyle
	iterStyle = vtkSmartPointer<InteractorStyle>::New();

	//Splitter
	splitterWind = new wxSplitterWindow(this, idSplitterWindow, wxDefaultPosition, wxDefaultSize, wxSP_3D, "Splitter");
	splitterWind->SetMinimumPaneSize(2);//Needed otherwise it will unslip in the double click

	//TreeListCtrl
	treeMesh = new wxTreeListCtrl(splitterWind, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTL_MULTIPLE | wxTL_NO_HEADER | wxTL_CHECKBOX);
	treeMesh->AppendColumn(wxT("Meshs"), wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, 1);
	//Load image
	bmpTreeCheckCamera = wxICON(ICON_TREE_CHECKCAM);
	wxImage imgTreeCheckCamera = bmpTreeCheckCamera.ConvertToImage();
	imgTreeCheckCamera.Rescale(16, 16);
	//Define the image that will be shown if the camera is seeing the point on checkCamVisibility
	wxImageList *treeMeshImageList = new wxImageList(imgTreeCheckCamera.GetWidth(), imgTreeCheckCamera.GetHeight(), true);
	treeMeshImageList->Add(imgTreeCheckCamera);
	treeMesh->AssignImageList(treeMeshImageList);

	//VTK panel
	vtk_panel = new wxVTKRenderWindowInteractor(splitterWind, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "VTK");

	renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renWin;

	renWin = vtk_panel->GetRenderWindow();
	renWin->AddRenderer(renderer.GetPointer());
	iterStyle->SetInteractor(vtk_panel);
	iterStyle->SetDefaultRenderer(renderer.GetPointer());
	renWin->GetInteractor()->SetInteractorStyle(iterStyle.Get());

	splitterWind->SplitVertically(vtk_panel, treeMesh, 0);
	setTreeVisibility(false);

	//IterStyle
	iterStyle->meshVector = &meshVector;
	iterStyle->treeMesh = treeMesh;

	//Tools
	distanceTool = vtkSmartPointer<DistanceTool>::New();
	distanceTool->SetInteractor(vtk_panel);
	angleTool = vtkSmartPointer<AngleTool>::New();
	angleTool->SetInteractor(vtk_panel);
	elevationTool = vtkSmartPointer<ElevationTool>::New();
	elevationTool->SetInteractor(vtk_panel);
	elevationTool->setMeshTree(treeMesh);
	viewTool = vtkSmartPointer<ViewTool>::New();
	viewTool->setMeshVector(&meshVector);
	viewTool->setTree(treeMesh);
	viewTool->SetInteractor(vtk_panel);
	aligWithAxisTool = vtkSmartPointer<AlignWithAxisTool>::New();
	aligWithAxisTool->SetInteractor(vtk_panel);
	volumeTool = vtkSmartPointer<VolumeTool>::New();
	volumeTool->SetInteractor(vtk_panel);
	checkCamTool = vtkSmartPointer<CheckCamTool>::New();
	checkCamTool->SetInteractor(vtk_panel);
	checkCamTool->setTreeMesh(treeMesh);
	deleteTool = vtkSmartPointer<DeleteTool>::New();
	deleteTool->SetInteractor(vtk_panel);
	updateCameraTool = vtkSmartPointer<UpdateCameraTool>::New();
	updateCameraTool->SetInteractor(vtk_panel);
	alignTool = vtkSmartPointer<AlignTool>::New();
	alignTool->SetInteractor(vtk_panel);


	//Axis
	vtkSmartPointer<vtkAxesActor> axisActor = vtkSmartPointer<vtkAxesActor>::New();
	axisWidget = vtkSmartPointer<AxisWidget>::New();
	axisWidget->SetOutlineColor(0.9300, 0.5700, 0.1300);
	axisWidget->SetOrientationMarker(axisActor);
	axisWidget->SetInteractor(vtk_panel->GetRenderWindow()->GetInteractor());
	axisWidget->SetViewport(0.0, 0.0, 0.4, 0.4);
	axisWidget->SetEnabled(true);
	axisWidget->InteractiveOff();
	axisWidget->SetEnabled(false);


	//Sizer
	wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
	boxSizer->Add(splitterWind, 1, wxEXPAND | wxALL, 0);

	this->SetMenuBar(menuBar);
	toolBar->Realize();

	this->SetSizer(boxSizer);
	this->Layout();

	this->Centre(wxBOTH);

	Connect(wxEVT_TREELIST_ITEM_CHECKED, wxTreeListEventHandler(FrmPrincipal::OnTreeChoice), NULL, this);
	Connect(wxEVT_TREELIST_ITEM_ACTIVATED, wxTreeListEventHandler(FrmPrincipal::OnTreeItemActivated), NULL, this);
}

FrmPrincipal::~FrmPrincipal()
{
}

void FrmPrincipal::changeInterface(int interfaceType)
{
	if (interfaceType == InterfaceTypes::reconstructor)
	{
		//Menu
		menuBar = new wxMenuBar(0);
		menuBar->Append(menuFile, "File");
		menuBar->Append(menuEdit, "Edit");
		menuBar->Append(menuFilters, "Filters");
		menuBar->Append(menuHelp, "Help");
		this->SetMenuBar(menuBar);
		//ToolBar
	}
	else if (interfaceType == InterfaceTypes::analyzer)
	{
		//Menu
		menuBar = new wxMenuBar(0);
		menuBar->Append(menuFile, "File");
		menuBar->Append(menuEdit, "Edit");
		menuBar->Append(menuTools, "Tools");
		menuBar->Append(menuView, "View");
		menuBar->Append(menuHelp, "Help");
		this->SetMenuBar(menuBar);
	}
	else if (interfaceType == InterfaceTypes::advanced)
	{
		//Menu
		menuBar = new wxMenuBar(0);
		menuBar->Append(menuFile, "File");
		menuBar->Append(menuEdit, "Edit");
		menuBar->Append(menuFilters, "Filters");
		menuBar->Append(menuTools, "Tools");
		menuBar->Append(menuView, "View");
		menuBar->Append(menuHelp, "Help");
		this->SetMenuBar(menuBar);
	}
}
//Menu
void FrmPrincipal::OnMenuOpen(wxCommandEvent& event)
{
	wxFileDialog* openDialog = new wxFileDialog(this, "Find some cool 3D", "", "", 
		"OBJ, PLY and WRL files (*.obj;*.ply;*.wrl)|*.obj;*.ply;*.wrl", wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
	if (openDialog->ShowModal() == wxID_OK)
	{
		wxArrayString paths;
		openDialog->GetPaths(paths);
		loadMesh(paths);
	}
	delete openDialog;
}
void FrmPrincipal::OnMenuOpenCameras(wxCommandEvent & event)
{
	Mesh* mesh = getMeshFromTree();
	if (!mesh)
	{
		return;
	}
	wxFileDialog* openDialog = new wxFileDialog(this, "Find some NVM/SFM file", "", "", "NVM and SFM files (*.nvm;*.sfm)|*.nvm;*.sfm", wxFD_FILE_MUST_EXIST);
	if (openDialog->ShowModal() == wxID_OK)
	{
		mesh->destructCameras(renderer, treeMesh);
		loadCameras(mesh, openDialog->GetPath().ToStdString());
	}
	delete openDialog;
}
void FrmPrincipal::OnMenuExportMesh(wxCommandEvent & event)
{
	Mesh* mesh = getMeshFromTree();
	if (mesh != NULL)
	{
		wxFileDialog* saveDialog;
		if (mesh->getHasTexture())
		{
			saveDialog = new wxFileDialog(this, "Save the mesh", "", "", "OBJ files (*.obj)|*.obj", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			if (saveDialog->ShowModal() == wxID_OK)
			{
				wxBeginBusyCursor();
				if (!mesh->exportMesh(saveDialog->GetPath().ToStdString()))
				{
					wxLogError("Error while exporting mesh");
				}
				wxEndBusyCursor();
			}
		}
		else
		{
			saveDialog = new wxFileDialog(this, "Save the mesh", "", "", "PLY files (*.ply)|*.ply", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			if (saveDialog->ShowModal() == wxID_OK)
			{
				wxBeginBusyCursor();
				if(!mesh->exportMesh(saveDialog->GetPath().ToStdString()))
				{
					wxLogError("Error while exporting mesh");
				}
				wxEndBusyCursor();
			}
		}
		delete saveDialog;
	}
}
void FrmPrincipal::OnMenuExportCameras(wxCommandEvent & event)
{
	Mesh* mesh = getMeshFromTree();
	if (mesh)
	{
		if (mesh->cameras.size() == 0)
		{
			wxLogError("Please select a mesh with cameras");
			return;
		}
		wxFileDialog* saveDialog;
		saveDialog = new wxFileDialog(this, "Save the cameras", "", "", "SFM and NVM files (*.sfm;*.nvm)|*.sfm;*.nvm", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (saveDialog->ShowModal() == wxID_OK)
		{
			wxBeginBusyCursor();
			std::string filename = saveDialog->GetPath().ToStdString();
			if (Utils::getFileExtension(filename) == "sfm")
			{
				mesh->saveSFM(filename);
			}
			else
			{
				mesh->saveNVM(filename);
			}
			wxEndBusyCursor();
		}
		delete saveDialog;
	}
}
void FrmPrincipal::OnMenuExportGeoRegisteredPointCloud(wxCommandEvent & event)
{
	Mesh* mesh = getMeshFromTree();
	if (mesh)
	{
		if (mesh->cameras.size() == 0)
		{
			wxLogError("Please select a mesh with cameras");
			return;
		}
		if (mesh->cameras[0]->gpsData == nullptr)
		{
			wxLogError("Please select a mesh with cameras that have GPS data");
			return;
		}
		wxFileDialog* saveDialog;
		saveDialog = new wxFileDialog(this, "Save the geo registered point cloud", "", "", "PLY files (*.ply)|*.ply", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (saveDialog->ShowModal() != wxID_OK)
		{
			delete saveDialog;
			return;
		}
		std::string plyFilePath = saveDialog->GetPath().ToStdString();
		std::string gpsCamerasFilePath = plyFilePath.substr(0, plyFilePath.size() - 4) + ".gps";
		delete saveDialog;
		wxBeginBusyCursor();
		vtkNew<vtkPoints> sourcePoints, targetPoints;
		sourcePoints->SetDataTypeToDouble();
		targetPoints->SetDataTypeToDouble();
		for (auto camera : mesh->cameras)
		{
			if (camera->cameraPoints.size() != 0)
			{
				sourcePoints->InsertNextPoint(camera->cameraPoints[0]);
				double newPoint[3];
				Utils::lla_to_ecef(camera->gpsData->getLatitude(),
					camera->gpsData->getLongitude(),
					camera->gpsData->getAltitude(),
					newPoint);
				targetPoints->InsertNextPoint(newPoint);
			}
		}
		//Landmark transform
		vtkNew<vtkLandmarkTransform> landmarkTransform;
		landmarkTransform->SetSourceLandmarks(sourcePoints);
		landmarkTransform->SetTargetLandmarks(targetPoints);
		vtkNew<vtkTransform> T;
		T->SetMatrix(landmarkTransform->GetMatrix());
		//Print error
		//double sum = 0;
		//for (size_t i = 0; i < mesh->cameras.size(); i++)
		//{
		//	double* point;// = T->TransformPoint(mesh->cameras[i]->cameraPoints[0]);
		//	point = T->TransformDoublePoint(mesh->cameras[i]->cameraPoints[0]);
		//	for (int j = 0; j < 3; j++)
		//	{
		//		sum += std::abs(point[j] - targetPoints->GetPoint(i)[j]);
		//	}
		//}
		//std::stringstream ss;
		//ss << std::fixed << std::setprecision(15) << sum;
		//wxMessageBox(ss.str());
		//Save PLY
		bool hasColor = false;
		bool hasNormals = false;
		bool hasFaces = false;
		size_t qtdPoints;
		vtkSmartPointer<vtkPoints> points = mesh->getPolyData()->GetPoints();
		if (!points)
		{
			wxEndBusyCursor();
			return;
		}
		qtdPoints = points->GetNumberOfPoints();
		vtkSmartPointer<vtkUnsignedCharArray> colors = vtkUnsignedCharArray::SafeDownCast(mesh->getPolyData()->GetPointData()->GetScalars());
		if (colors)
		{
			hasColor = true;
		}
		vtkSmartPointer<vtkFloatArray> normals = vtkFloatArray::SafeDownCast(mesh->getPolyData()->GetPointData()->GetNormals());
		if (normals)
		{
			hasNormals = true;
		}
		//Write the ply file
		std::ofstream outputFile(plyFilePath, std::ios::binary);
		outputFile << "ply" << "\n";
		outputFile << "format binary_little_endian 1.0" << "\n";
		outputFile << "element vertex " << qtdPoints << "\n";
		outputFile << "property double x" << "\n";
		outputFile << "property double y" << "\n";
		outputFile << "property double z" << "\n";
		if (hasColor)
		{
			outputFile << "property uchar red" << "\n";
			outputFile << "property uchar green" << "\n";
			outputFile << "property uchar blue" << "\n";
		}
		if (hasNormals)
		{
			outputFile << "property float nx" << "\n";
			outputFile << "property float ny" << "\n";
			outputFile << "property float nz" << "\n";
		}
		outputFile << "end_header" << "\n";
		if (outputFile.is_open())
		{
			double *point, *color, *normal;
			for (size_t i = 0; i < qtdPoints; i++)
			{
				point = points->GetPoint(i);
				point = T->TransformDoublePoint(point);
				Utils::writeBin<double>(outputFile, point[0]);
				Utils::writeBin<double>(outputFile, point[1]);
				Utils::writeBin<double>(outputFile, point[2]);
				if (hasColor)
				{
					color = colors->GetTuple3(i);
					Utils::writeBin<unsigned char>(outputFile, color[0]);
					Utils::writeBin<unsigned char>(outputFile, color[1]);
					Utils::writeBin<unsigned char>(outputFile, color[2]);
				}
				if (hasNormals)
				{
					normal = normals->GetTuple(i);
					normal = T->TransformDoubleNormal(normal);
					Utils::writeBin<float>(outputFile, normal[0]);
					Utils::writeBin<float>(outputFile, normal[1]);
					Utils::writeBin<float>(outputFile, normal[2]);
				}
			}
		}
		else
		{
			wxEndBusyCursor();
			wxLogError("Could not open the geo registered point cloud");
			return;
		}
		outputFile.close();
		//Save GPS cameras
		std::ofstream gpsCamerasFile;
		gpsCamerasFile.open(gpsCamerasFilePath);
		if (gpsCamerasFile.is_open())
		{
			gpsCamerasFile << "point.x,point.y,point.z;lat,long,alt;X,Y,Z(ECEF)\n";
			gpsCamerasFile << mesh->cameras.size() << "\n";
			for (int i = 0; i < sourcePoints->GetNumberOfPoints(); i++)
			{
				gpsCamerasFile << std::fixed << std::setprecision(7) << sourcePoints->GetPoint(i)[0] << ","
					<< sourcePoints->GetPoint(i)[1] << ","
					<< sourcePoints->GetPoint(i)[2] << ";" <<
					std::fixed << std::setprecision(7) << mesh->cameras[i]->gpsData->getLatitude() << ","
					<< mesh->cameras[i]->gpsData->getLongitude() << ","
					<< mesh->cameras[i]->gpsData->getAltitude() << ";" <<
					std::fixed << std::setprecision(15) << targetPoints->GetPoint(i)[0] << ","
					<< targetPoints->GetPoint(i)[1] << ","
					<< targetPoints->GetPoint(i)[2] << "\n";
			}
		}
		else
		{
			wxEndBusyCursor();
			wxLogError("Could not open the GPS cameras file");
			return;
		}
		gpsCamerasFile.close();
		wxEndBusyCursor();
	}
}
void FrmPrincipal::OnMenuExportPoisson(wxCommandEvent & event)
{
	Mesh* mesh = getMeshFromTree();
	if (mesh != NULL)
	{
		if (mesh->volumeActor == NULL)
		{
			wxMessageBox("Please select some mesh that have a poisson reconstruction", "Error", wxICON_ERROR, this);
			return;
		}
		wxFileDialog* saveDialog = new wxFileDialog(this, "Save the poisson result", "", "", "PLY files (*.ply)|*.ply", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (saveDialog->ShowModal() == wxID_OK)
		{
			wxBeginBusyCursor();
			vtkSmartPointer<vtkPLYWriter> plyWriter = vtkSmartPointer<vtkPLYWriter>::New();
			plyWriter->SetFileName(saveDialog->GetPath().c_str());
			plyWriter->SetInputData(mesh->volumeActor->GetMapper()->GetInput());
			plyWriter->SetArrayName("RGB");
			plyWriter->Write();
			wxEndBusyCursor();
		}
		delete saveDialog;
	}
}
void FrmPrincipal::OnMenuSnapshot(wxCommandEvent & event)
{
	SnapshotTool* snapshot = new SnapshotTool();
	snapshot->takeSnapshot(vtk_panel->GetRenderWindow(), true);
	delete snapshot;
}
void FrmPrincipal::OnMenuTransform(wxCommandEvent & event)
{
	Mesh* mesh = getMeshFromTree();
	if (mesh != NULL)
	{
		if (aligWithAxisTool->GetEnabled())
		{
			aligWithAxisTool->SetEnabled(false);
			return;
		}
		disableMeasureTools(-1);
		TransformDialog* transformDialog = new TransformDialog(this);
		int id = transformDialog->ShowModal();
		if (id == wxID_OK)
		{
			wxBeginBusyCursor(wxHOURGLASS_CURSOR);
			mesh->transform(renderer, transformDialog->transform);
			renderer->ResetCamera(mesh->getPolyData()->GetBounds());
			renderer->ResetCameraClippingRange();
			vtk_panel->GetRenderWindow()->Render();
			wxEndBusyCursor();
		}
		else if (id == TransformDialog::idAlignWithTool)
		{
			aligWithAxisTool->setMesh(mesh);
			aligWithAxisTool->SetEnabled(true);
		}
		delete transformDialog;
	}
}
void FrmPrincipal::OnMenuUpdateCamera(wxCommandEvent & event)
{
	disableMeasureTools(6);
	if (!updateCameraTool->GetEnabled())
	{
		Camera* cam = getCameraFromTree();
		if (cam == nullptr)
		{
			menuItemUpdateCamera->Check(false);
			return;
		}
		cam->setVisibility(true);
		treeMesh->CheckItem(cam->getListItemCamera(), wxCHK_CHECKED);
		Utils::updateCamera(renderer, cam);
		cam->createImageActor(renderer);
		updateCameraTool->setCamera(cam);
		updateCameraTool->SetEnabled(true);
		menuItemUpdateCamera->Check();
	}
	else
	{
		updateCameraTool->SetEnabled(false);
		menuItemUpdateCamera->Check(false);
	}
	vtk_panel->SetFocus();
}
void FrmPrincipal::OnMenuShowAxis(wxCommandEvent & event)
{
	axisWidget->SetEnabled(menuItemShowAxis->IsChecked());
	vtk_panel->GetRenderWindow()->Render();
}
void FrmPrincipal::OnMenuShowViews(wxCommandEvent & event)
{
	viewTool->SetEnabled(menuItemShowViews->IsChecked());
	vtk_panel->GetRenderWindow()->Render();
}
void FrmPrincipal::OnMenuFlyToPoint(wxCommandEvent & event)
{
	this->iterStyle->setFlyToPoint(!this->iterStyle->getFlyToPoint());
}
void FrmPrincipal::OnMenuSettings(wxCommandEvent & event)
{
	SettingsDialog* settingsDialog = new SettingsDialog(this);
	if (settingsDialog->ShowModal() == wxID_OK)
	{
		changeInterface(SettingsDialog::getInterfaceType());
	}
	delete settingsDialog;
}
void FrmPrincipal::OnMenuPoisson(wxCommandEvent & event)
{
	Mesh* mesh = getMeshFromTree();
	if (mesh)
	{
		PoissonDialog* poissonDialog = new PoissonDialog(this);
		if (poissonDialog->ShowModal() != wxID_OK)
		{
			delete poissonDialog;
			return;
		}
		wxBeginBusyCursor();
		vtkSmartPointer<vtkPolyData> polyData = mesh->getPolyData();
		if (polyData->GetPointData()->GetNormals() == NULL)
		{
			//Just generate mesh for cells, not points
			vtkSmartPointer<vtkPolyDataNormals> normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
			normalGenerator->SetInputData(polyData);
			normalGenerator->Update();
			polyData = normalGenerator->GetOutput();
			if (polyData->GetPointData()->GetNormals() == NULL)//It is a point cloud 
			{
				wxMessageBox("The point cloud has no normals", "Error", wxICON_ERROR);
				wxEndBusyCursor();
				delete poissonDialog;
				return;
			}
		}
		PoissonRecon* poisson = new PoissonRecon();
		polyData = poisson->createPoisson(polyData, poissonDialog->getOptions());
		unsigned int depth = poissonDialog->getOptions()->depth;
		delete poisson, poissonDialog;
		if (polyData == NULL)
		{
			wxMessageBox("It was not possible to reconstruct", "Error", wxICON_ERROR);
			wxEndBusyCursor();
			return;
		}
		mesh->destructVolume(renderer, treeMesh);//clear the actual reconstruction, if any.
		mesh->volumeActor = Draw::createPolyData(renderer, polyData);
		std::stringstream ss;
		ss << "Poisson Reconstruction (Depth " << depth << ")";
		mesh->setListItemMeshVolume(treeMesh->AppendItem(mesh->getListItemMesh(), ss.str()));
		treeMesh->CheckItem(mesh->getListItemMeshVolume(), wxCHK_CHECKED);
		vtk_panel->GetRenderWindow()->Render();
		wxEndBusyCursor();
	}
}
void FrmPrincipal::OnMenuSSDRecon(wxCommandEvent & event)
{
	Mesh* mesh = getMeshFromTree();
	if (mesh)
	{
		SSDDialog* ssdDialog = new SSDDialog(this);
		if (ssdDialog->ShowModal() != wxID_OK)
		{
			delete ssdDialog;
			return;
		}
		wxBeginBusyCursor();
		vtkSmartPointer<vtkPolyData> polyData = mesh->getPolyData();
		if (!polyData->GetPointData()->GetNormals())
		{
			//Just generate mesh for cells, not points
			vtkSmartPointer<vtkPolyDataNormals> normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
			normalGenerator->SetInputData(polyData);
			normalGenerator->Update();
			polyData = normalGenerator->GetOutput();
			if (!polyData->GetPointData()->GetNormals())//It is a point cloud 
			{
				wxMessageBox("The point cloud has no normals", "Error", wxICON_ERROR);
				wxEndBusyCursor();
				delete ssdDialog;
				return;
			}
		}
		SSDRecon* ssdRecon = new SSDRecon();
		polyData = ssdRecon->createSSD(polyData, ssdDialog->getOptions());
		unsigned int depth = ssdDialog->getOptions()->depth;
		delete ssdRecon, ssdDialog;;
		if (!polyData)
		{
			wxMessageBox("It was not possible to reconstruct", "Error", wxICON_ERROR);
			wxEndBusyCursor();
			return;
		}
		mesh->destructVolume(renderer, treeMesh);//clear the actual reconstruction, if any.
		mesh->volumeActor = Draw::createPolyData(renderer, polyData);
		std::stringstream ss;
		ss << "SSD Reconstruction (Depth " << depth << ")";
		mesh->setListItemMeshVolume(treeMesh->AppendItem(mesh->getListItemMesh(), ss.str()));
		treeMesh->CheckItem(mesh->getListItemMeshVolume(), wxCHK_CHECKED);
		vtk_panel->GetRenderWindow()->Render();
		wxEndBusyCursor();
	}
}
void FrmPrincipal::OnMenuReconstructClusters(wxCommandEvent & event)
{
	std::string dirPath;
	wxDirDialog* dirDialog = new wxDirDialog(this, "Select the folder with the clusters");
	if (dirDialog->ShowModal() != wxID_OK)
	{
		delete dirDialog;
		return;
	}
	dirPath = dirDialog->GetPath();
	delete dirDialog;
	wxBeginBusyCursor();
	wxDir* directory = new wxDir(dirPath);
	if (!directory->IsOpened())
	{
		wxLogError("Could not open the folder");
		wxEndBusyCursor();
		return;
	}
	wxArrayString* files = new wxArrayString();
	directory->GetAllFiles(dirPath, files, wxEmptyString, wxDIR_FILES);
	if (files->size() == 0)
	{
		wxLogError("There is no cluster in the folder");
		wxEndBusyCursor();
		return;
	}
	std::string clusterPath = files->Item(0).ToStdString();
	std::string nvmPath = "";
	std::string extension;
	//Progress dialog to show the progress while loading the clusters
	wxProgressDialog* progDialog = new wxProgressDialog("Clusters", "Loading " + Utils::getFileName(clusterPath),
		files->size(), this, wxPD_SMOOTH | wxPD_AUTO_HIDE | wxPD_APP_MODAL);
	//Filter to join group the cluster in one polydata
	vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
	for (int i = 0; i < files->size(); i++)
	{
		clusterPath = files->Item(i).ToStdString();
		progDialog->Update(i, "Loading " + Utils::getFileName(clusterPath));
		extension = clusterPath.substr(clusterPath.size() - 4, clusterPath.size());
		if (extension == ".ply")
		{
			//Read the cluster
			vtkSmartPointer<vtkPLYReader> reader = vtkSmartPointer<vtkPLYReader>::New();
			reader->SetFileName(clusterPath.c_str());
			reader->Update();
			vtkSmartPointer<vtkPolyData> polyData = reader->GetOutput();
			if (polyData)
			{
				if (!polyData->GetPointData()->GetNormals())
				{
					wxLogError(wxString("Could not load normals from the file " + Utils::getFileName(clusterPath)));
					delete progDialog;
					wxEndBusyCursor();
					return;
				}
				PoissonRecon* poisson = new PoissonRecon();
				OptionsPoisson* options = new OptionsPoisson();
				if (clusterPath[0] == 'o')//Object, increase the poisson depth
				{
					options->depth = 12;
					polyData = poisson->createPoisson(polyData, options);
				}
				else//Planar region, no need to increase the poisson depth
				{
					options->depth = 8;
					polyData = poisson->createPoisson(polyData, options);
				}
				delete poisson, options;
				//Add the cluster in the append filter
				appendFilter->AddInputData(polyData);
			}
			else
			{
				wxLogError(wxString("Could not load " + Utils::getFileName(clusterPath)));
				delete progDialog;
				wxEndBusyCursor();
				return;
			}
		}
		else if (extension == ".nvm")
		{
			nvmPath = clusterPath;
		}
	}
	delete progDialog;
	//Run the appendFilter
	appendFilter->Update();
	if (appendFilter->GetOutput() == NULL)
	{
		wxEndBusyCursor();
		return;
	}
	//Clean polydata
	vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
	cleaner->SetInputData(appendFilter->GetOutput());
	cleaner->Update();
	//Save the polydata with all clusters
	wxString plyPath = dirPath + "/fusion.ply";
	vtkSmartPointer<vtkPLYWriter> plyWriter = vtkSmartPointer<vtkPLYWriter>::New();
	plyWriter->SetFileName(plyPath);
	plyWriter->SetInputData(cleaner->GetOutput());
	plyWriter->Write();
	//Texturize
	if (nvmPath != "")
	{
		ReconstructionDialog* recons = new ReconstructionDialog(this);
		recons->texturizeMesh(plyPath.ToStdString(), nvmPath, plyPath.ToStdString());
		if (wxMessageBox("The reconstruction was a success! Do you wanna see it?", "", wxYES_NO | wxICON_QUESTION, this) == 2)
		{
			wxArrayString paths;
			paths.push_back(recons->getOutputPath());
			loadMesh(paths);
		}
		delete recons;
	}
	else
	{
		if (wxMessageBox("The reconstruction was a success! Do you wanna see it?", "", wxYES_NO | wxICON_QUESTION, this) == 2)
		{
			wxArrayString paths;
			paths.push_back(plyPath);
			loadMesh(paths);
		}
	}
	wxEndBusyCursor();
}
void FrmPrincipal::OnMenuICP(wxCommandEvent & event)
{
	std::set<Mesh*> meshs = getMeshsFromTree(2);
	if (meshs.size() != 2)
	{
		return;
	}
	ICPDialog* icpDialog = new ICPDialog(this);
	icpDialog->setSourceMesh(*std::next(meshs.begin(), 0));
	icpDialog->setTargetMesh(*std::next(meshs.begin(), 1));
	if (icpDialog->ShowModal() == wxID_OK)
	{
		if (icpDialog->getTransform())
		{
			(*std::next(meshs.begin(), 0))->transform(renderer, icpDialog->getTransform());
		}
	}
	delete icpDialog;
}
void FrmPrincipal::OnMenuAlignTool(wxCommandEvent & event)
{
	disableMeasureTools(7);
	if (alignTool->GetEnabled())
	{
		alignTool->SetEnabled(false);
		menuItemAlignTool->Check(false);
		return;
	}
	std::set<Mesh*> meshs = getMeshsFromTree(2);
	if (meshs.size() != 2)
	{
		menuItemAlignTool->Check(false);
		return;
	}
	alignTool->setMeshSource(*std::next(meshs.begin(), 0));
	alignTool->setMeshTarget(*std::next(meshs.begin(), 1));
	alignTool->SetEnabled(true);
}
void FrmPrincipal::OnMenuM3C2(wxCommandEvent & event)
{
	M3C2Dialog* diag = new M3C2Dialog(this);
	diag->ShowModal();
	delete diag;
}
void FrmPrincipal::OnMenuSurfaceTrimmer(wxCommandEvent & event)
{
	Mesh* mesh = getMeshFromTree();
	if (mesh)
	{
		if (mesh->volumeActor)
		{
			int trimValue = wxGetNumberFromUser("Choose the trim value", "Trim", "Surface trimmer", 7, 1, 100, this);
			if (trimValue == -1)
			{
				return;
			}
			wxBeginBusyCursor();
			vtkSmartPointer<vtkPolyData> polyData = vtkPolyData::SafeDownCast(mesh->volumeActor->GetMapper()->GetInput());
			if (polyData->GetPointData())
			{
				if (!polyData->GetPointData()->GetArray("Values"))
				{
					wxMessageBox("The reconstruction has no value field", "Error", wxICON_ERROR);
					wxEndBusyCursor();
					return;
				}
			}
			OptionsTrimmer* options = new OptionsTrimmer();
			options->trimValue = trimValue;
			SurfaceTrimmer* surfaceTrimmer = new SurfaceTrimmer();
			polyData = surfaceTrimmer->trimSurface(polyData, options);
			delete surfaceTrimmer, options;
			if (!polyData)
			{
				wxMessageBox("It was not possible to trim", "Error", wxICON_ERROR);
				wxEndBusyCursor();
				return;
			}
			mesh->destructVolume(renderer, treeMesh);//clear the actual reconstruction, if any.
			mesh->volumeActor = Draw::createPolyData(renderer, polyData);
			mesh->setListItemMeshVolume(treeMesh->AppendItem(mesh->getListItemMesh(), "Trimmed Reconstruction"));
			treeMesh->CheckItem(mesh->getListItemMeshVolume(), wxCHK_CHECKED);
			vtk_panel->GetRenderWindow()->Render();
			wxEndBusyCursor();
		}
		else
		{
			wxMessageBox("Please reconstruct the mesh with SSD or Poisson", "Error", wxICON_ERROR);
			wxEndBusyCursor();
			return;
		}
	}
}
void FrmPrincipal::OnMenuFastQuadricSimplification(wxCommandEvent & event)
{
	Mesh* mesh = getMeshFromTree();
	if (mesh != NULL)
	{
		if (mesh->actors.size() == 1)
		{
			if (mesh->getPolyData()->GetPolys()->GetNumberOfCells() == 0)
			{
				wxLogError("This method does not work with point clouds");
				return;
			}
		}
		else if (mesh->textures.size() != 0)
		{
			wxLogError("This method does not work with textured meshes");
			return;
		}
		int reduction = wxGetNumberFromUser("Choose the percentage of reduction", "Reduction (%)", "Fast quadric simplification", 50, 1, 100, nullptr);
		if (reduction != -1)
		{
			wxBeginBusyCursor();
			mesh->fastQuadricSimplification((100 - reduction) / 100.0f);
			renderer->GetRenderWindow()->Render();
			wxEndBusyCursor();
		}
	}
}
void FrmPrincipal::OnMenuHelp(wxCommandEvent & event)
{
	helpController->DisplayContents();
}
void FrmPrincipal::OnMenuAbout(wxCommandEvent & event)
{
	AboutDialog* about = new AboutDialog(this);
	about->ShowModal();
	delete about;
}
//ToolBar
void FrmPrincipal::OnToolDelete(wxCommandEvent & event)
{
	wxTreeListItems itens;
	if (!treeMesh->GetSelections(itens))
	{
		wxMessageBox("Please select something on the tree", "Error", wxICON_ERROR, this);
		return;
	}
	disableTools();
	for (auto item : itens)
	{
		if (!item.IsOk())
		{
			continue;
		}
		for (int i = 0; i < meshVector.size(); i++)
		{
			if (meshVector.at(i)->getListItemMesh() == item)
			{
				meshVector.at(i)->destruct(renderer, treeMesh);
				delete meshVector.at(i);
				meshVector.erase(meshVector.begin() + i);
				if (meshVector.size() == 0)
				{
					setTreeVisibility(false);
					this->Layout();
				}
				continue;
			}
			else if (meshVector.at(i)->getListItemMeshCameras() == item)
			{
				meshVector.at(i)->destructCameras(renderer, treeMesh);
				continue;
			}
			else if (meshVector.at(i)->getListItemMeshVolumes() == item)
			{
				meshVector.at(i)->destructVolumes(renderer, treeMesh);
				continue;
			}
			else if (meshVector.at(i)->getListItemMeshVolume() == item)
			{
				meshVector.at(i)->destructVolume(renderer, treeMesh);
				continue;
			}
			for (int j = 0; j < meshVector.at(i)->cameras.size(); j++)
			{
				if (meshVector.at(i)->cameras.at(j)->getListItemCamera() == item)
				{
					meshVector.at(i)->deleteCamera(j, renderer, treeMesh);
					continue;
				}
			}
			for (int j = 0; j < meshVector.at(i)->volumes.size(); j++)
			{
				if (meshVector.at(i)->volumes.at(j)->getListItem() == item)
				{
					meshVector.at(i)->deleteVolume(j, renderer, treeMesh);
					continue;
				}
			}
		}
	}
	renderer->GetRenderWindow()->Render();
}
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkExtractSelectedThresholds.h>
void FrmPrincipal::OnToolDebug(wxCommandEvent & event)
{
	Mesh* mesh = getMeshFromTree();
	if (!mesh)
	{
		return;
	}
	return;
	std::ifstream parametersFile("C:/Users/julia/Desktop/Talude_proximo/Talude_3/Talude_3aligned/aligned.mat.txt");
	if (!parametersFile.good())
	{
		return;
	}
	std::string line;
	vtkNew<vtkMatrix4x4> mat;
	int lin = 0;
	while (std::getline(parametersFile, line))
	{
		auto iss = std::istringstream{ line };
		auto str = std::string{};
		int col = 0;
		while (iss >> str) 
		{
			mat->SetElement(lin, col, std::atof(str.c_str()));
			col++;
		}
		lin++;
	}
	vtkNew<vtkTransform> T;
	T->SetMatrix(mat);
	mesh->transform(renderer, T);

	//vtkSmartPointer<vtkPolyData> polyData = mesh->getPolyData();
	//vtkNew<vtkPolyDataConnectivityFilter> connectivityFilter;
	//connectivityFilter->SetInputData(polyData);
	//connectivityFilter->SetExtractionModeToAllRegions();
	//connectivityFilter->ColorRegionsOn();
	//connectivityFilter->Update();

	//std::vector<vtkSmartPointer<vtkIdTypeArray>> originalIds;
	//std::vector<unsigned int> originalIdsSize;
	//unsigned int maxSize = 0;
	//for (int i = 0; i < connectivityFilter->GetNumberOfExtractedRegions(); i++)
	//{
	//	vtkNew<vtkFloatArray> thresh;
	//	thresh->SetNumberOfComponents(1);
	//	thresh->InsertNextValue(i);
	//	thresh->InsertNextValue(i);
	//	thresh->SetName("RegionId");

	//	vtkNew<vtkSelection> selection;
	//	vtkNew<vtkSelectionNode> selectionNode;
	//	selectionNode->SetContentType(vtkSelectionNode::THRESHOLDS);
	//	selectionNode->SetFieldType(vtkSelectionNode::POINT);
	//	selectionNode->SetSelectionList(thresh);
	//	selection->AddNode(selectionNode);

	//	vtkNew<vtkExtractSelectedThresholds> extract;
	//	extract->SetInputConnection(0, connectivityFilter->GetOutputPort());
	//	extract->SetInputData(1, selection);
	//	extract->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "RegionId");
	//	extract->Update(); //outputs a vtkUnstructuredGrid
	//	originalIds.push_back(vtkIdTypeArray::SafeDownCast(vtkUnstructuredGrid::SafeDownCast(extract->GetOutput())->GetPointData()->GetArray("vtkOriginalPointIds")));
	//	originalIdsSize.push_back(originalIds.back()->GetNumberOfTuples());
	//	if (originalIdsSize.back() > maxSize)
	//	{
	//		maxSize = originalIdsSize.back();
	//	}
	//}
	//for (size_t i = 0; i < originalIds.size(); i++)
	//{
	//	if (originalIdsSize[i] <= maxSize * 0.1)
	//	{
	//		continue;
	//	}
	//	for (int j = 0; j < originalIds[i]->GetNumberOfTuples(); j++)
	//	{
	//		vtkNew<vtkIdList> cellIds;
	//		polyData->GetPointCells(originalIds[i]->GetTuple1(j), cellIds);
	//		for (size_t k = 0; k < cellIds->GetNumberOfIds(); k++)
	//		{
	//			polyData->DeleteCell(cellIds->GetId(k));
	//		}
	//	}
	//	polyData->RemoveDeletedCells();
	//}
	//mesh->updateCells(polyData);
	//Draw::createPolyData(renderer, connectivityFilter->GetOutput());
	//vtkNew<vtkPolyDataMapper> mapper;
	//mapper->SetInputConnection(connectivityFilter->GetOutputPort());
	//mapper->SetScalarRange(connectivityFilter->GetOutput()->GetPointData()->GetArray("RegionId")->GetRange());
	//mapper->Update();

	//vtkNew<vtkActor> actor;
	//actor->SetMapper(mapper);
	//renderer->AddActor(actor);
	//The output has a regionID array, you need to select them and delete vtkExtractSelectedThresholds
	//https://stackoverflow.com/questions/41858270/separating-meshes-with-vtkpolydataconnectivityfilter
}
void FrmPrincipal::OnToolPoints(wxCommandEvent& event)
{
	menuRepresentation->Check(idToolPoints, true);
	for (auto mesh: meshVector)
	{
		for (auto actor : mesh->actors)
		{
			actor->GetProperty()->SetRepresentationToPoints();
		}
	}
	vtk_panel->GetRenderWindow()->Render();
}
void FrmPrincipal::OnToolWireframe(wxCommandEvent& event)
{
	menuRepresentation->Check(idToolWireframe, true);
	for (auto mesh : meshVector)
	{
		for (auto actor : mesh->actors)
		{
			actor->GetProperty()->SetRepresentationToWireframe();
		}
	}
	vtk_panel->GetRenderWindow()->Render();
}
void FrmPrincipal::OnToolSurface(wxCommandEvent& event)
{
	menuRepresentation->Check(idToolSurface, true);
	for (auto mesh : meshVector)
	{
		for (auto actor : mesh->actors)
		{
			actor->GetProperty()->SetRepresentationToSurface();
		}
	}
	vtk_panel->GetRenderWindow()->Render();
}
void FrmPrincipal::OnToolMeasure(wxCommandEvent& event)
{
	disableMeasureTools(0);
	if (!distanceTool->GetEnabled())
	{
		Mesh* mesh = getMeshFromTree();
		if (!mesh)
		{
			menuItemToolMeasure->Check(false);
			return;
		}
		distanceTool->setMesh(mesh);
		distanceTool->SetEnabled(true);
		menuItemToolMeasure->Check(true);
		toolMeasure->SetNormalBitmap(bmpToolMeasureON);
	}
	else
	{
		distanceTool->SetEnabled(false);
		menuItemToolMeasure->Check(false);
		toolMeasure->SetNormalBitmap(bmpToolMeasureOFF);
	}
	toolBar->Realize();
}
void FrmPrincipal::OnToolAngle(wxCommandEvent& event)
{
	disableMeasureTools(1);
	if (!angleTool->GetEnabled())
	{
		Mesh* mesh = getMeshFromTree();
		if (mesh == NULL)
		{
			menuItemToolAngle->Check(false);
			return;
		}
		angleTool->setMesh(mesh);
		angleTool->SetEnabled(true);
		menuItemToolAngle->Check(true);
		toolAngle->SetNormalBitmap(bmpToolAngleON);
	}
	else
	{
		angleTool->SetEnabled(false);
		menuItemToolAngle->Check(false);
		toolAngle->SetNormalBitmap(bmpToolAngleOFF);
	}
	toolBar->Realize();
}
void FrmPrincipal::OnToolVolume(wxCommandEvent & event)
{
	disableMeasureTools(4);
	if (!volumeTool->GetEnabled())
	{
		menuItemToolVolume->Check(false);
		Mesh* mesh = getMeshFromTree();
		if (!mesh)
		{
			return;
		}
		bool computeVolume = false;
		if (!mesh->volumeActor)
		{
			wxBeginBusyCursor();
			//Test if the mesh is watertight
			vtkSmartPointer<vtkPolyData> polyData = mesh->getPolyData();
			if (!polyData)
			{
				wxMessageBox("It was not possible to calculate the volume", "Error", wxICON_ERROR);
				wxEndBusyCursor();
				return;
			}
			if (!mesh->getIsPointCloud())//Mesh
			{
				vtkSmartPointer<vtkFeatureEdges> featureEdges = vtkSmartPointer<vtkFeatureEdges>::New();
				featureEdges->SetInputData(polyData);
				featureEdges->BoundaryEdgesOn();
				featureEdges->FeatureEdgesOff();
				featureEdges->ManifoldEdgesOff();
				featureEdges->NonManifoldEdgesOff();
				featureEdges->Update();
				if (featureEdges->GetOutput()->GetPoints()->GetNumberOfPoints() == 0)//Closed mesh
				{
					wxEndBusyCursor();
					computeVolume = true;
				}
				else
				{
					//Just generate mesh for cells, not points
					vtkNew<vtkPolyDataNormals> normalGenerator;
					normalGenerator->SetInputData(polyData);
					normalGenerator->Update();
					polyData = normalGenerator->GetOutput();
					wxEndBusyCursor();
				}
			}
			else if (!polyData->GetPointData()->GetNormals())//PointCloud
			{
				wxMessageBox("You need a point cloud with normals to calculate the volume", "Error", wxICON_ERROR);
				wxEndBusyCursor();
				return;
			}
			if (!computeVolume)
			{
				PoissonRecon* poisson = new PoissonRecon();
				OptionsPoisson* options = new OptionsPoisson();
				options->depth = 8;
				polyData = poisson->createPoisson(polyData, options);
				delete poisson, options;
				if (!polyData)
				{
					wxMessageBox("It was not possible to calculate the volume", "Error", wxICON_ERROR);
					wxEndBusyCursor();
					return;
				}
				mesh->volumeActor = Draw::createPolyData(renderer, polyData);
				mesh->setListItemMeshVolume(treeMesh->AppendItem(mesh->getListItemMesh(), "Poisson Reconstruction (Depth 8)"));
				treeMesh->CheckItem(mesh->getListItemMeshVolume(), wxCHK_CHECKED);
				vtk_panel->GetRenderWindow()->Render();
				wxEndBusyCursor();
			}
		}
		volumeTool->setMesh(mesh);
		volumeTool->setTree(treeMesh);
		volumeTool->SetEnabled(true);
		menuItemToolVolume->Check(true);
		toolCalcVolume->SetNormalBitmap(bmpToolCalcVolumeON);
	}
	else
	{
		volumeTool->SetEnabled(false);
		menuItemToolVolume->Check(false);
		toolCalcVolume->SetNormalBitmap(bmpToolCalcVolumeOFF);
	}
	toolBar->Realize();
}
void FrmPrincipal::OnToolElevation(wxCommandEvent & event)
{
	disableMeasureTools(2);
	if (!elevationTool->GetEnabled())
	{
		Mesh* mesh = getMeshFromTree();
		if (mesh == NULL)
		{
			menuItemToolElevation->Check(false);
			return;
		}
		wxBeginBusyCursor(wxHOURGLASS_CURSOR);
		elevationTool->setMesh(mesh);
		elevationTool->SetEnabled(true);
		menuItemToolElevation->Check(true);
		toolElevation->SetNormalBitmap(bmpToolElevationON);
		wxEndBusyCursor();
	}
	else
	{
		elevationTool->SetEnabled(false);
		menuItemToolElevation->Check(false);
		toolElevation->SetNormalBitmap(bmpToolElevationOFF);
	}
	toolBar->Realize();
}
void FrmPrincipal::OnToolCalibration(wxCommandEvent& event)
{
	if (distanceTool->hasFinished())
	{
		distanceTool->updateCalibration();
	}
	else if (distanceTool->GetEnabled())
	{
		wxMessageBox("Finish the measure and then come back", "Warning", wxICON_DEFAULT_TYPE, this);
	}
	else
	{
		wxMessageBox("Measure some object that you know the real measure and then come back", "Warning", wxICON_DEFAULT_TYPE, this);
	}
}
void FrmPrincipal::OnToolCheckCamVisibility(wxCommandEvent & event)
{
	if (!checkCamTool->GetEnabled())
	{
		Mesh* mesh = getMeshFromTree();
		if (mesh == NULL)
		{
			menuItemToolCheckCamVisibility->Check(false);
			return;
		}
		if (mesh->cameras.size() > 0)
		{
			if (mesh->cameras.back()->getIs360())
			{
				wxMessageBox("This tool does not work with 360 images.", "Error", wxICON_ERROR, this);
				menuItemToolCheckCamVisibility->Check(false);
				return;
			}
		}
		if (mesh->cameras.size() == 0)
		{
			wxMessageBox("This mesh doesn't have images.", "Error", wxICON_ERROR, this);
			menuItemToolCheckCamVisibility->Check(false);
			return;
		}
		checkCamTool->setMesh(mesh);
		checkCamTool->SetEnabled(true);
		menuItemToolCheckCamVisibility->Check(true);
		toolCheckCamVisibility->SetNormalBitmap(bmpToolCheckCamVisibilityON);
	}
	else
	{
		checkCamTool->SetEnabled(false);
		menuItemToolCheckCamVisibility->Check(false);
		toolCheckCamVisibility->SetNormalBitmap(bmpToolCheckCamVisibilityOFF);
	}
	toolBar->Realize();
}
void FrmPrincipal::OnToolStartReconstruction(wxCommandEvent & event)
{
	ReconstructionDialog* recons = new ReconstructionDialog(this);
	if (recons->ShowModal() == wxID_OK)
	{
		if (wxMessageBox("The reconstruction was a success! Do you wanna see it?", "", wxYES_NO | wxICON_QUESTION, this) == 2)
		{
			wxArrayString paths;
			paths.push_back(recons->getOutputPath());
			loadMesh(paths);
		}
	}
	delete recons;
}
void FrmPrincipal::OnToolSnapshot(wxCommandEvent & event)
{
	SnapshotTool* snapshot = new SnapshotTool();
	snapshot->takeSnapshot(vtk_panel->GetRenderWindow());
	delete snapshot;
	return;
}
void FrmPrincipal::OnToolLight(wxCommandEvent & event)
{
	isLightOn = !isLightOn;
	vtkSmartPointer<vtkActorCollection> ac = renderer->GetActors();
	vtkCollectionSimpleIterator ait;
	for (ac->InitTraversal(ait); vtkActor* actor = ac->GetNextActor(ait); )
	{
		for (actor->InitPathTraversal(); vtkAssemblyPath* path = actor->GetNextPath(); )
		{
			vtkSmartPointer<vtkActor> apart = reinterpret_cast <vtkActor*> (path->GetLastNode()->GetViewProp());
			apart->GetProperty()->SetLighting(isLightOn);
		}
	}
	vtk_panel->GetRenderWindow()->Render();
}
void FrmPrincipal::OnToolDeletePointsFaces(wxCommandEvent & event)
{
	disableMeasureTools(5);
	if (!deleteTool->GetEnabled())
	{
		Mesh* mesh = getMeshFromTree();
		if (mesh == NULL)
		{
			menuItemToolDeletePoints->Check(false);
			return;
		}
		wxBeginBusyCursor(wxHOURGLASS_CURSOR);
		deleteTool->setMesh(mesh);
		deleteTool->SetEnabled(true);
		menuItemToolDeletePoints->Check(true);
		toolDeletePoints->SetNormalBitmap(bmpToolDeletePointsFacesON);
		wxEndBusyCursor();
	}
	else
	{
		deleteTool->SetEnabled(false);
		menuItemToolDeletePoints->Check(false);
		toolDeletePoints->SetNormalBitmap(bmpToolDeletePointsFacesOFF);
	}
	toolBar->Realize();
}
void FrmPrincipal::OnToolVR(wxCommandEvent & event)
{
	Mesh* mesh = getMeshFromTree();
	if (mesh == NULL)
	{
		return;
	}
	VRDialog* vrDialog = new VRDialog(this, mesh);
	vrDialog->setCamera(renderer->GetActiveCamera());
	toolVR->SetNormalBitmap(bmpToolVRON);
	toolBar->Realize();
	vrDialog->ShowModal();
	delete vrDialog;
	toolVR->SetNormalBitmap(bmpToolVROFF);
	toolBar->Realize();
}
//Tree
//CheckBox
void FrmPrincipal::OnTreeChoice(wxTreeListEvent& event)
{
	wxTreeListItem item = event.GetItem();
	if (!item.IsOk())
	{
		return;
	}
	for (auto mesh : meshVector)
	{
		if (mesh->getListItemMesh() == item)
		{
			mesh->setMeshVisibility(!mesh->getMeshVisibility());
			break;
		}
		else if (mesh->getListItemMeshCameras() == item)
		{
			//Here i just need to copy the checkstate
			mesh->setCamerasVisibility(treeMesh->GetCheckedState(item));
			treeMesh->CheckItemRecursively(item, (wxCheckBoxState)mesh->getCamerasVisibility());
			break;
		}
		else if (mesh->getListItemMeshTexture() == item)
		{
			mesh->setTextureVisibility(!mesh->getTextureVisibility());
			break;
		}
		else if (mesh->getListItemMeshVolumes() == item)
		{
			//Here i just need to copy the checkstate
			mesh->setVolumesVisibility(treeMesh->GetCheckedState(item));
			treeMesh->CheckItemRecursively(item, (wxCheckBoxState)mesh->getVolumesVisibility());
			break;
		}
		else if (mesh->getListItemMeshVolume() == item)
		{
			mesh->setVolumeVisibility(!mesh->getVolumeVisibility());
			break;
		}
		for (auto camera : mesh->cameras)
		{
			if (camera->getListItemCamera() == item)
			{
				camera->setVisibility(!camera->getVisibility());
				break;
			}
		}
		for (auto volume : mesh->volumes)
		{
			if (volume->getListItem() == item)
			{
				volume->setVisibility(!volume->getVisibility());
				break;
			}
		}
	}
	vtk_panel->SetFocus();
	//disableMeasureTools(3);
	renderer->ResetCameraClippingRange();
	vtk_panel->GetRenderWindow()->Render();
}
//Double click
void FrmPrincipal::OnTreeItemActivated(wxTreeListEvent & event)
{
	iterStyle->setMode360Image(false);
	wxTreeListItem item = event.GetItem();
	if (!item.IsOk())
	{
		return;
	}
	vtk_panel->SetFocus();
	for (auto mesh : meshVector)
	{
		mesh->update360ClickPoints(renderer, nullptr);
		if (mesh->getListItemMesh() == item)
		{
			mesh->setMeshVisibility(true);
			treeMesh->CheckItem(item, wxCHK_CHECKED);
			renderer->ResetCamera(mesh->getPolyData()->GetBounds());
			renderer->ResetCameraClippingRange();
			vtk_panel->GetRenderWindow()->Render();
			return;
		}
		else if (mesh->getListItemMeshCameras() == item)
		{
			return;
		}
		else if (mesh->getListItemMeshVolumes() == item)
		{
			return;
		}
		else if (mesh->getListItemMeshTexture() == item)
		{
			return;
		}
		else if (mesh->getListItemMeshVolume() == item)
		{
			mesh->setVolumeVisibility(true);
			treeMesh->CheckItem(item, wxCHK_CHECKED);
			renderer->ResetCamera(mesh->volumeActor->GetBounds());
			renderer->ResetCameraClippingRange();
			vtk_panel->GetRenderWindow()->Render();
			return;
		}
		for (auto camera : mesh->cameras)
		{
			if (camera->getListItemCamera() == item)
			{
				camera->setVisibility(true);
				treeMesh->CheckItem(item, wxCHK_CHECKED);
				Utils::updateCamera(renderer, camera);
				camera->createImageActor(renderer);
				if (camera->getIs360() && camera->image360Actor)
				{
					iterStyle->setMode360Image(true);
					mesh->update360ClickPoints(renderer, camera);
				}
				vtk_panel->GetRenderWindow()->Render();
				return;
			}
		}
		for (auto volume : mesh->volumes)
		{
			if (volume->getListItem() == item)
			{
				volume->setVisibility(true);
				treeMesh->CheckItem(item, wxCHK_CHECKED);
				renderer->ResetCamera(volume->actor->GetBounds());
				renderer->ResetCameraClippingRange();
				vtk_panel->GetRenderWindow()->Render();
				return;
			}
		}
	}
}
wxTreeListItem FrmPrincipal::addItemToTree(const wxTreeListItem& parentItem, const std::string& itemName, wxCheckBoxState state)
{
	wxTreeListItem newTreeItem = treeMesh->AppendItem(parentItem, wxString(itemName));
	treeMesh->CheckItem(newTreeItem, state);
	if (!isTreeVisible)
	{
		setTreeVisibility(true);
	}
	return newTreeItem;
}
Mesh* FrmPrincipal::getMeshFromTree()
{
	wxTreeListItem item;
	if (meshVector.size() == 1)//Avoid annoying the user if there is just one mesh on the tree
	{
		item = meshVector.at(0)->getListItemMesh();
	}
	else
	{
		wxTreeListItems itens;
		if (treeMesh->GetSelections(itens))
		{
			item = itens[0];
		}
	}
	if (!item.IsOk())
	{
		wxLogWarning("Please select some mesh on the tree");
		return nullptr;
	}
	return getMeshFromTree(item);
}
Mesh * FrmPrincipal::getMeshFromTree(wxTreeListItem item)
{
	if (!item.IsOk())
	{
		return nullptr;
	}
	//If the person selects a texture or a camera, get the mesh item
	while (treeMesh->GetItemParent(item) != treeMesh->GetRootItem())
	{
		item = treeMesh->GetItemParent(item);
	}
	for (auto mesh : meshVector)
	{
		if (item == mesh->getListItemMesh())
		{
			treeMesh->CheckItem(item, wxCHK_CHECKED);
			mesh->setMeshVisibility(true);
			vtk_panel->GetRenderWindow()->Render();
			return mesh;
		}
	}
	return nullptr;
}
std::set<Mesh*> FrmPrincipal::getMeshsFromTree(unsigned int numberOfMeshs)
{
	std::set<Mesh*> meshs;
	wxTreeListItems itens;
	if (treeMesh->GetSelections(itens))
	{
		if (itens.size() != numberOfMeshs)
		{
			wxLogWarning(wxString("Please select " + std::to_string(numberOfMeshs)  + " meshs on the tree"));
			return meshs;
		}
		for (auto item : itens)
		{
			if (item.IsOk())
			{
				Mesh* mesh = getMeshFromTree(item);
				if (mesh)
				{
					meshs.insert(mesh);
				}
			}
		}
		if (meshs.size() != numberOfMeshs)
		{
			meshs.clear();
			wxLogWarning(wxString("Please select " + std::to_string(numberOfMeshs) + " meshs on the tree"));
		}
	}
	else
	{
		wxLogWarning(wxString("Please select " + std::to_string(numberOfMeshs) + " meshs on the tree"));
	}
	return meshs;
}
Camera* FrmPrincipal::getCameraFromTree()
{
	wxTreeListItem item;
	wxTreeListItems itens;
	if (treeMesh->GetSelections(itens))
	{
		item = itens[0];
	}
	if (!item.IsOk())
	{
		wxLogWarning("Please select some camera on the tree");
		return nullptr;
	}
	for (auto mesh : meshVector)
	{
		for (auto camera : mesh->cameras)
		{
			if (camera->getListItemCamera() == item)
			{
				return camera;
			}
		}
	}
	wxLogWarning("Please select some camera on the tree");
	return nullptr;
}

void FrmPrincipal::loadMesh(wxArrayString paths)
{
	outputErrorWindow->clearFlags();
	wxProgressDialog* progDialog = new wxProgressDialog("Meshs", "Loading " + Utils::getFileName(paths.Item(0).ToStdString()), 
		paths.size(), this, wxPD_SMOOTH | wxPD_AUTO_HIDE | wxPD_APP_MODAL);
	wxBeginBusyCursor();
	for (unsigned int i = 0; i < paths.size(); i++)
	{
		const std::string path = paths.Item(i).ToStdString();
		progDialog->Update(i, "Loading " + Utils::getFileName(path));
		Mesh* mesh = new Mesh(path);
		if (mesh->isLoaded())
		{
			//Add actors int the scene
			for (auto actor : mesh->actors)
			{
				renderer->AddActor(actor);
			}
			//Tree
			mesh->setListItemMesh(addItemToTree(treeMesh->GetRootItem(), Utils::getFileName(path), wxCHK_CHECKED));
			if (mesh->getHasColor() || mesh->getHasTexture())
			{
				mesh->setListItemMeshTexture(addItemToTree(mesh->getListItemMesh(), "Texture", wxCHK_CHECKED));
			}
			meshVector.push_back(mesh);
			if (mesh->getHasTexture())
			{
				loadCameras(mesh, path.substr(0, path.find_last_of('_')) + ".nvm");
			}
		}
		else
		{
			mesh->destruct(renderer, treeMesh);
			delete mesh;
		}
	}
	delete progDialog;
	paths.clear();
	this->Update();
	renderer->GetActiveCamera()->SetPosition(10, 10, 10);
	renderer->ResetCamera();
	renderer->GetRenderWindow()->Render();
	wxEndBusyCursor();
}

bool FrmPrincipal::loadCameras(Mesh * mesh, std::string camerasFilePath = "")
{
	if (!mesh)
	{
		wxLogError("No mesh was selected");
		wxEndBusyCursor();
		return 0;
	}
	std::vector<std::string> imagePaths;
	if (!ImageIO::getCamerasFileImagePaths(camerasFilePath, imagePaths))
	{
		if (wxMessageBox("It was not possible to load any camera parametes file(.SFM or .NVM), do you want to select it manually?", "Error",
			wxYES_NO | wxICON_ERROR, this)
			== wxYES)
		{
			wxFileDialog* parametersDialog = new wxFileDialog(this, "Find the parameters file(.SFM or .NVM)", "", "", "SFM and NVM files (*.sfm;*.nvm)|*.sfm;*.nvm", wxFD_FILE_MUST_EXIST);
			if (parametersDialog->ShowModal() == wxID_OK)
			{
				camerasFilePath = parametersDialog->GetPath();
				if (!ImageIO::getCamerasFileImagePaths(camerasFilePath, imagePaths))
				{
					wxLogError(wxString("Error loading " + Utils::getFileName(camerasFilePath)));
					delete parametersDialog;
					wxEndBusyCursor();
					return 0;
				}
			}
			else
			{
				delete parametersDialog;
				wxEndBusyCursor();
				return 0;
			}
			delete parametersDialog;
		}
		else
		{
			wxEndBusyCursor();
			return 0;
		}
	}
	wxBeginBusyCursor();
	//Test to see if the paths inside the SFM/NVM are correct
	if (!ImageIO::getImagePathsExist(imagePaths))
	{
		std::string newImageDir;
		wxMessageBox("Please select the correct image folder", "Error", wxICON_ERROR, this);
		wxDirDialog* imageDialog = new wxDirDialog(this, "Choose the image folder", "", wxDD_DEFAULT_STYLE);
		//Just stop when all the images are in the selected folder
		bool insist = 0;
		do
		{
			if (insist)
			{
				wxMessageBox("The folder still wrong, please select the correct image folder", "Warning", wxICON_WARNING, this);
			}
			insist = 1;
			if (imageDialog->ShowModal() == wxID_OK)
			{
				newImageDir = imageDialog->GetPath();
			}
			else
			{
				wxLogError(wxString("Error loading " + Utils::getFileName(camerasFilePath)));
				delete imageDialog;
				wxEndBusyCursor();
				return 0;
			}
		} while (!ImageIO::getImagePathsExist(imagePaths, newImageDir));
		delete imageDialog;
		ImageIO::replaceCamerasFileImageDir(camerasFilePath, newImageDir);
	}
	//Now the SFM/NVM is correct, lets load the cameras
	std::vector<Camera*> cameras;
	if (!ImageIO::loadCameraParameters(camerasFilePath, cameras))
	{
		wxLogError(wxString("Error loading " + Utils::getFileName(camerasFilePath)));
		wxEndBusyCursor();
		return 0;
	}
	//Add cameras in the mesh
	mesh->cameras = cameras;
	//SortByName
	mesh->sortCameras();
	//Add cameras tree item in the mesh
	if (!mesh->getListItemMeshCameras())
	{
		mesh->setListItemMeshCameras(addItemToTree(mesh->getListItemMesh(), "Cameras", wxCHK_UNCHECKED));
	}
	mesh->setCamerasVisibility(false);
	treeMesh->CheckItem(mesh->getListItemMeshCameras(), wxCHK_UNCHECKED);
	//Create the frustum and tree itens
	for (auto camera : mesh->cameras)
	{
		camera->createActorFrustrum(renderer);
		camera->setListItemCamera(addItemToTree(mesh->getListItemMeshCameras(), Utils::getFileName(camera->filePath), wxCHK_UNCHECKED));
		camera->setVisibility(false);
	}
	this->vtk_panel->GetRenderWindow()->Render();
	wxEndBusyCursor();
	return 1;
}

void FrmPrincipal::setTreeVisibility(bool visibility)
{
	int x, y;
	this->GetSize(&x, &y);
	if (x >= 0)
	{
		if (visibility)//If we want to show, we need to calc the correct size
		{
			if ((x - x * .7) > 250)//More than 250 is too much
			{
				x = x - 250;
			}
			else
			{
				x = x * .7;//Tree get 30% of the screen
			}
		}
		splitterWind->SetSashPosition(x, true);
		isTreeVisible = visibility;
	}
}

void FrmPrincipal::OnClose(wxCloseEvent& event)
{
	axisWidget->SetEnabled(false);
	viewTool->SetEnabled(false);
	disableTools();
	for (int i = 0; i < meshVector.size(); i++)
	{
		meshVector.at(i)->destruct(renderer, treeMesh);
		delete meshVector.at(i);
	}
	meshVector.clear();
	vtk_panel->Delete();
	event.Skip();
}

void FrmPrincipal::OnSplitterDClick(wxSplitterEvent & event)
{
	setTreeVisibility(!isTreeVisible);
	event.Skip();
}

void FrmPrincipal::OnSashPosChanged(wxSplitterEvent & event)
{
	int x, y;
	this->GetSize(&x, &y);
	if (x >= 0)
	{
		if (splitterWind->GetSashPosition() < x*.90)
		{
			isTreeVisible = true;
		}
		else
		{
			isTreeVisible = false;
		}
	}
	event.Skip();
}

void FrmPrincipal::OnSize(wxSizeEvent & event)
{
	if (splitterWind != NULL)
	{
		setTreeVisibility(isTreeVisible);
	}
	wxSize* size = &this->GetSize();
	if (size != NULL && vtk_panel != NULL)
	{
		vtk_panel->UpdateSize(size->x, size->y);
	}
	event.Skip();
}

void FrmPrincipal::OnKeyPress(wxKeyEvent & event)
{
	char key = (char)event.GetKeyCode();
	if (event.ControlDown())
	{
		if (key == 'S')
		{
			SnapshotTool* snapshot = new SnapshotTool();
			snapshot->takeSnapshot(vtk_panel->GetRenderWindow());
			delete snapshot;
		}
		else if (key == 'M')
		{
			this->OnToolMeasure((wxCommandEvent)NULL);
		}
		else if (key == 'A')
		{
			this->OnToolAngle((wxCommandEvent)NULL);
		}
		else if (key == 'I')
		{
			this->OnToolCalibration((wxCommandEvent)NULL);
		}
		else if (key == 'K')
		{
			this->OnToolCheckCamVisibility((wxCommandEvent)NULL);
		}
		else if (key == 'T')
		{
			Mesh* mesh = getMeshFromTree();
			if (mesh != NULL)
			{
				mesh->setTextureVisibility(!mesh->getTextureVisibility());
				treeMesh->CheckItem(mesh->getListItemMeshTexture(), (wxCheckBoxState)mesh->getTextureVisibility());
				vtk_panel->GetRenderWindow()->Render();
			}
		}
		else if (key == 'L')
		{
			OnToolLight((wxCommandEvent)NULL);
		}
		else if (key == 'R')
		{
			renderer->ResetCamera();
			renderer->GetRenderWindow()->Render();
		}
		else if (key == 'V')
		{
			wxTreeListItem cameraItem = treeMesh->GetSelection();
			if (!cameraItem.IsOk())
			{
				return;
			}
			wxTreeListItem meshItem = treeMesh->GetItemParent(treeMesh->GetItemParent(cameraItem));
			for (int i = 0; i < meshVector.size(); i++)
			{
				if (meshVector.at(i)->getListItemMesh() == meshItem)
				{
					if (meshVector.at(i)->cameras.size() == 0)
					{
						wxMessageBox("There is no camera to change the view UP", "Error", wxICON_ERROR);
						return;
					}
					for (int k = 0; k < meshVector.at(i)->cameras.size(); k++)
					{
						if (meshVector.at(i)->cameras.at(k)->getListItemCamera() == cameraItem)
						{
							if (meshVector.at(i)->cameras.at(k)->changeViewUp())
							{
								Utils::updateCamera(renderer, meshVector.at(i)->cameras.at(k));
								vtk_panel->GetRenderWindow()->Render();
								return;
							}
						}
					}
				}
			}
		}
		else if (key == 'U')
		{
			//Representation models
			//VTK_POINTS    0
			//VTK_WIREFRAME 1
			//VTK_SURFACE   2
			int oldRepresentation = 2;
			for (int i = 0; i < meshVector.size(); i++)
			{
				Mesh* mesh = meshVector.at(i);
				for (int j = 0; j < mesh->actors.size(); j++)
				{
					if (mesh->actors.at(j)->GetProperty()->GetRepresentation() < 2)
					{
						oldRepresentation = mesh->actors.at(j)->GetProperty()->GetRepresentation();
						mesh->actors.at(j)->GetProperty()->SetRepresentationToSurface();
					}
				}
			}
			//now we can do the pick
			double* focalDisplay = iterStyle->getDisplayPosition(renderer->GetActiveCamera()->GetFocalPoint());
			double clickPos[3];
			vtkSmartPointer<vtkPropPicker>  picker = vtkSmartPointer<vtkPropPicker>::New();
			picker->Pick(focalDisplay[0], focalDisplay[1], 0, renderer);
			picker->GetPickPosition(clickPos);
			//Let's back to the old representation
			if (oldRepresentation != 2)
			{
				for (int i = 0; i < meshVector.size(); i++)
				{
					Mesh* mesh = meshVector.at(i);
					for (int j = 0; j < mesh->actors.size(); j++)
					{
						mesh->actors.at(j)->GetProperty()->SetRepresentation(oldRepresentation);
					}
				}
			}
			//We put it down here to avoid spending time rendering the change in the representation
			if (picker->GetActor() != NULL)
			{
				renderer->GetActiveCamera()->SetFocalPoint(clickPos);
				renderer->ResetCameraClippingRange();
				renderer->GetRenderWindow()->Render();
			}
			//props.clear();
			delete focalDisplay;
		}
		else if (key == 'F')
		{
			this->ShowFullScreen(!this->IsFullScreen());
		}
		else if (key == 'N')
		{
			Mesh* mesh = getMeshFromTree();
			if (mesh != NULL)
			{
				if (mesh->cameras.size() != 0)
				{
					mesh->createCamerasPath(renderer);
					mesh->setCameraPathVisibility(!mesh->getCameraPathVisibility());
					vtk_panel->GetRenderWindow()->Render();
				}
			}
		}
		else if (key == 'E')
		{
			this->OnToolElevation((wxCommandEvent)NULL);
		}
		else if (key == 'D')
		{
#ifdef DEBUG_MART
		OnToolDebug((wxCommandEvent)NULL);
#endif // DEBUG_MART
#ifndef DEBUG_MART
		OnToolDeletePointsFaces((wxCommandEvent)NULL);
#endif // !DEBUG_MART
		}
	}
	if (key == WXK_ESCAPE)
	{
		disableTools();
	}
	else if (key == WXK_DELETE)
	{
		if (deleteTool->GetEnabled())
		{
			if (deleteTool->enterKeyPressed())
			{
				toolDeletePoints->SetNormalBitmap(bmpToolDeletePointsFacesOFF);
				toolBar->Realize();
			}
		}
		else
		{
			OnToolDelete((wxCommandEvent)NULL);
		}
	}
	else if (key == 'r' || key == WXK_RETURN)//Necessary to be here because VTK does not support ENTER Key // 'r' is the NUMPAD_ENTER the WXK_NUMPAD_ENTER DOES NOT WORK
	{
		if (deleteTool->GetEnabled())
		{
			if (deleteTool->enterKeyPressed())
			{
				menuItemToolDeletePoints->Check(false);
				toolDeletePoints->SetNormalBitmap(bmpToolDeletePointsFacesOFF);
				toolBar->Realize();
			}
		}
		if (alignTool->GetEnabled())
		{
			alignTool->enterKeyPressed();
		}
		if (volumeTool->GetEnabled())
		{
			volumeTool->enterKeyPressed();
			if (volumeTool->isVolumeComputed())
			{
				OnToolVolume((wxCommandEvent)NULL);
			}
		}
	}
	else if (event.GetKeyCode() == WXK_F1)
	{
		OnMenuHelp((wxCommandEvent)NULL);
	}
	event.Skip();
}

void FrmPrincipal::OnDropFiles(wxDropFilesEvent& event)
{
	if (event.GetNumberOfFiles() != 0)
	{
		wxArrayString paths;
		for (int i = 0; i < event.GetNumberOfFiles(); i++)
		{
			paths.push_back(event.GetFiles()[i]);
		}
		if (paths.size() == 1)
		{
			std::string path = paths.Item(0).ToStdString();
			std::string extension = Utils::getFileExtension(path);
			if (extension == "sfm" || extension == "nvm")
			{
				Mesh* mesh = getMeshFromTree();
				if (mesh == NULL)
				{
					return;
				}
				mesh->destructCameras(renderer, treeMesh);
				loadCameras(mesh, path);
			}
			else
			{
				loadMesh(paths);
			}
		}
		else
		{
			loadMesh(paths);
		}

	}
	event.Skip();
}

void FrmPrincipal::OnLeftDClick(wxMouseEvent & event)
{
	int x, y;
	int w, h;
	vtk_panel->GetLastEventPosition(x, y);
	vtk_panel->GetClientSize(&w, &h);
	wxPoint p = event.GetPosition();
	if (iterStyle != NULL && x == p.x && (h - (y + 1)) == p.y)
	{
		iterStyle->doubleClick();
	}
	//.Skip is used in the ProjetoMeshApp::OnLeftDClick
}

wxHtmlHelpController * FrmPrincipal::getHelpController()
{
	return helpController;
}

void FrmPrincipal::disableTools()
{
	this->ShowFullScreen(false);
	disableMeasureTools(-1);
}

void FrmPrincipal::disableMeasureTools(int toolToAvoid)
{
	if (distanceTool->GetEnabled() && toolToAvoid != 0)
	{
		OnToolMeasure((wxCommandEvent)NULL);
	}
	if (angleTool->GetEnabled() && toolToAvoid != 1)
	{
		OnToolAngle((wxCommandEvent)NULL);
	}
	if (elevationTool->GetEnabled() && toolToAvoid != 2)
	{
		OnToolElevation((wxCommandEvent)NULL);
	}
	if (checkCamTool->GetEnabled() && toolToAvoid != 3)
	{
		OnToolCheckCamVisibility((wxCommandEvent)NULL);
	}
	if (volumeTool->GetEnabled() && toolToAvoid != 4)
	{
		OnToolVolume((wxCommandEvent)NULL);
	}
	if (deleteTool->GetEnabled() && toolToAvoid != 5)
	{
		OnToolDeletePointsFaces((wxCommandEvent)NULL);
	}
	if (updateCameraTool->GetEnabled() && toolToAvoid != 6)
	{
		OnMenuUpdateCamera((wxCommandEvent)NULL);
	}
	if (alignTool->GetEnabled() && toolToAvoid != 7)
	{
		OnMenuAlignTool((wxCommandEvent)NULL);
	}
}