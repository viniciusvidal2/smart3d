#pragma once
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);

#define vtkRenderingCore_AUTOINIT 3(vtkInteractionStyle,vtkRenderingFreeType,vtkRenderingOpenGL2)
#define vtkRenderingVolume_AUTOINIT 1(vtkRenderingVolumeOpenGL2)

#include <set>

#include <vtkSmartPointer.h>

#include <wx/frame.h>
#include <wx/treelist.h>
//VTK
class vtkRenderer;
class wxVTKRenderWindowInteractor;
//Tools
class DistanceTool;
class AngleTool;
class ElevationTool;
class ViewTool;
class AlignWithAxisTool;
class VolumeTool;
class CheckCamTool;
class DeleteTool;
class UpdateCameraTool;
class AlignTool;
class AxisWidget;
//Wx
class wxHtmlHelpController;
class wxSplitterEvent;
class wxSplitterWindow;
class wxToolBarToolBase;
class wxMouseEvent;
//
class InteractorStyle;
class OutputErrorWindow;
class Mesh;
class Camera;

class FrmPrincipal : public wxFrame
{
private:
	DECLARE_EVENT_TABLE()

	//VTK
	vtkSmartPointer<vtkRenderer> renderer = nullptr;

	//Help
	wxHtmlHelpController*  helpController = nullptr;

	//Axis
	vtkSmartPointer<AxisWidget> axisWidget = nullptr;

	//Light tool
	bool isLightOn = true;

	//Tools
	vtkSmartPointer<DistanceTool> distanceTool = nullptr;
	vtkSmartPointer<AngleTool> angleTool = nullptr;
	vtkSmartPointer<ElevationTool> elevationTool = nullptr;
	vtkSmartPointer<ViewTool> viewTool = nullptr;
	vtkSmartPointer<AlignWithAxisTool> aligWithAxisTool = nullptr;
	vtkSmartPointer<VolumeTool> volumeTool = nullptr;
	vtkSmartPointer<CheckCamTool> checkCamTool = nullptr;
	vtkSmartPointer<DeleteTool> deleteTool = nullptr;
	vtkSmartPointer<UpdateCameraTool> updateCameraTool = nullptr;
	vtkSmartPointer<AlignTool> alignTool = nullptr;

	void disableTools();
	//Disable all tools but the tool in the parameter
	/*
	Distance - 0
	Angle - 1
	Elevation - 2
	CheckCam - 3
	Volume - 4
	Delete points and faces - 5
	Update camera - 6
	Align - 7
	*/
	void disableMeasureTools(int toolToAvoid);


	vtkSmartPointer<InteractorStyle> iterStyle = nullptr;
	//Camera
	/**
	 * Load the cameras from a SFM/NVM file.
	 *
	 * @param[in] the mesh that will receive the cameras
	 * @param[in] cameras file path, should be .sfm or .nvm 
	 */
	bool loadCameras(Mesh* mesh, std::string camerasFilePath);

	//Tree
	wxTreeListItem addItemToTree(const wxTreeListItem& parentItem, const std::string& itemName, wxCheckBoxState state);
	//Get the selected Mesh from the tree
	Mesh* getMeshFromTree();
	//Get the Mesh from the tree using its tree item or one of its children tree item
	Mesh* getMeshFromTree(wxTreeListItem item);
	//Get multiple selected Meshs from the tree, it obligates the user to select numberOfMeshs meshs on the tree
	std::set<Mesh*> getMeshsFromTree(unsigned int numberOfMeshs);
	//Get the selected Camera from the tree
	Camera* getCameraFromTree();

	//Splitter
	bool isTreeVisible;
	void setTreeVisibility(bool visibility);

	//Interface
	void changeInterface(int interfaceType);
	//Menu
	void OnMenuOpen(wxCommandEvent& event);
	void OnMenuOpenCameras(wxCommandEvent& event);
	void OnMenuExportMesh(wxCommandEvent& event);
	void OnMenuExportCameras(wxCommandEvent& event);
	void OnMenuExportGeoRegisteredPointCloud(wxCommandEvent& event);
	void OnMenuExportPoisson(wxCommandEvent& event);
	void OnMenuTransform(wxCommandEvent& event);
	void OnMenuUpdateCamera(wxCommandEvent& event);
	void OnMenuSnapshot(wxCommandEvent& event);
	void OnMenuShowAxis(wxCommandEvent& event);
	void OnMenuShowViews(wxCommandEvent& event);
	void OnMenuFlyToPoint(wxCommandEvent& event);
	void OnMenuSettings(wxCommandEvent& event);
	void OnMenuPoisson(wxCommandEvent& event);
	void OnMenuSSDRecon(wxCommandEvent& event);
	void OnMenuReconstructClusters(wxCommandEvent& event);
	void OnMenuICP(wxCommandEvent& event);
	void OnMenuAlignTool(wxCommandEvent& event);
	void OnMenuM3C2(wxCommandEvent& event);
	void OnMenuSurfaceTrimmer(wxCommandEvent& event);
	void OnMenuFastQuadricSimplification(wxCommandEvent& event);
	void OnMenuHelp(wxCommandEvent& event);
	void OnMenuAbout(wxCommandEvent& event);
	//Tools
	void OnToolDelete(wxCommandEvent& event);
	void OnToolDebug(wxCommandEvent& event);
	void OnToolPoints(wxCommandEvent& event);
	void OnToolWireframe(wxCommandEvent& event);
	void OnToolSurface(wxCommandEvent& event);
	void OnToolMeasure(wxCommandEvent& event);
	void OnToolAngle(wxCommandEvent& event);
	void OnToolVolume(wxCommandEvent& event);
	void OnToolElevation(wxCommandEvent& event);
	void OnToolCalibration(wxCommandEvent& event);
	void OnToolCheckCamVisibility(wxCommandEvent& event);
	void OnToolStartReconstruction(wxCommandEvent& event);
	void OnToolSnapshot(wxCommandEvent& event);
	void OnToolLight(wxCommandEvent& event);
	void OnToolDeletePointsFaces(wxCommandEvent& event);
	void OnToolVR(wxCommandEvent& event);
	/*
	Select checkbox tree
	*/
	void OnTreeChoice(wxTreeListEvent& event);
	/*
	Double Click tree
	*/
	void OnTreeItemActivated(wxTreeListEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnSplitterDClick(wxSplitterEvent& event);
	void OnSashPosChanged(wxSplitterEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnKeyPress(wxKeyEvent& event);
	void OnDropFiles(wxDropFilesEvent& event);

protected:
	//Execution path
	//std::string executionPath;
	//VTK
	vtkSmartPointer<OutputErrorWindow> outputErrorWindow = nullptr;
	std::vector<Mesh*> meshVector;

	//WX
	wxSplitterWindow* splitterWind;
	vtkSmartPointer<wxVTKRenderWindowInteractor> vtk_panel = nullptr;
	//Menu
	wxMenuBar* menuBar;
	wxMenu* menuFile;
	wxMenuItem* menuItemUpdateCamera;
	wxMenuItem* menuItemShowAxis;
	wxMenuItem* menuItemShowViews;
	wxMenuItem* menuItemFlyToPoint;
	wxMenuItem* menuItemAlignTool;
	wxMenu* menuEdit;
	wxMenu* menuRepresentation;
	wxMenuItem* menuItemToolDeletePoints;
	wxMenu* menuFilters;
	wxMenu* menuView;
	wxMenuItem* menuItemToolCheckCamVisibility;
	wxMenu* menuTools;
	wxMenuItem* menuItemToolMeasure;
	wxMenuItem* menuItemToolAngle;
	wxMenuItem* menuItemToolVolume;
	wxMenuItem* menuItemToolElevation;
	wxMenu* menuHelp;
	//ToolBar
	wxToolBar* toolBar;
	//Just the tools that use bitmap On and Off need an object
	wxToolBarToolBase* toolMeasure;
	wxToolBarToolBase* toolAngle;
	wxToolBarToolBase* toolCalcVolume;
	wxToolBarToolBase* toolElevation;
	wxToolBarToolBase* toolCheckCamVisibility;
	wxToolBarToolBase* toolDeletePoints;
	wxToolBarToolBase* toolVR;
	//Bitmaps
	wxBitmap bmpToolMeasureOFF;
	wxBitmap bmpToolMeasureON;
	wxBitmap bmpToolAngleOFF;
	wxBitmap bmpToolAngleON;
	wxBitmap bmpToolCalcVolumeOFF;
	wxBitmap bmpToolCalcVolumeON;
	wxBitmap bmpToolElevationOFF;
	wxBitmap bmpToolElevationON;
	wxBitmap bmpToolCheckCamVisibilityOFF;
	wxBitmap bmpToolCheckCamVisibilityON;
	wxBitmap bmpToolDeletePointsFacesOFF;
	wxBitmap bmpToolDeletePointsFacesON;
	wxBitmap bmpToolVROFF;
	wxBitmap bmpToolVRON;

	//Tree
	wxTreeListCtrl* treeMesh = nullptr;
	wxBitmap bmpTreeCheckCamera;
public:
	FrmPrincipal(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(500, 300), long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

	~FrmPrincipal();

	//Help
	wxHtmlHelpController* getHelpController();

	//Load
	void loadMesh(wxArrayString paths);

	void OnLeftDClick(wxMouseEvent& event);

};
enum {
	idMenuOpen,
	idErrorEnter,//Do not use this ID, it will trigger when you press Enter
	idMenuOpenCameras,
	idMenuExportMesh,
	idMenuExportCameras,
	idMenuExportGeoRegisteredPointCloud,
	idMenuExportPoisson,
	idMenuTransform,
	idMenuUpdateCamera,
	idMenuSnapshot,
	idMenuShowAxis,
	idMenuShowViews,
	idMenuFlyToPoint,
	idMenuICP,
	idMenuAlignTool,
	idMenuM3C2,
	idMenuSurfaceTrimmer,
	idMenuFastQuadricSimplification,
	idMenuSettings,
	idMenuPoisson,
	idMenuSSDRecon,
	idMenuReconstructClusters,
	idMenuAbout,
	idMenuHelp,
	idToolOpen,
	idToolDelete,
	idToolDebug,
	idToolPoints,
	idToolWireframe,
	idToolSurface,
	idToolMeasure,
	idToolAngle,
	idToolVolume,
	idToolElevation,
	idToolCalibration,
	idToolCheckCamVisibility,
	idToolStartReconstruction,
	idToolSnapshot,
	idToolLight,
	idToolDeletePointsFaces,
	idToolVR,
	idSplitterWindow
};

//Used to define which menus are shown
enum InterfaceTypes
{
	reconstructor = 0,
	analyzer = 1,
	advanced = 2//(reconstructor + analyzer)
};