#pragma once

#include <vtkSmartPointer.h>
#include <wx/treelist.h>

class vtkPolyData;
class vtkCellLocator;
class vtkRenderer;
class vtkTexture;
class vtkAbstractPicker;
class vtkTransform;
class vtkPoints;
class vtkUnsignedCharArray;
class vtkFloatArray;
class vtkDataObject;
class vtkActor;

class Volume;
class Calibration;
class Camera;

class Mesh 
{
private:
	//Tree
	wxTreeListItem listItemMesh = nullptr;
	wxTreeListItem listItemMeshVolume = nullptr;
	wxTreeListItem listItemMeshCameras = nullptr;
	wxTreeListItem listItemMeshTexture = nullptr;
	wxTreeListItem listItemMeshVolumes = nullptr;

	Calibration* calibration = nullptr;
	vtkSmartPointer<vtkPolyData> polyData = nullptr;
	vtkSmartPointer<vtkCellLocator> meshCellLocator = nullptr;
	double* lastPosition = nullptr;

	//Tests
	bool loaded = false;
	bool isPointCloud = false;
	bool hasColor = false;
	bool hasTexture = false;
	bool visible = false;
	bool textureVisibility = false;

public:
	Mesh(const std::string& filename);
	Mesh();
	~Mesh();

	/*
	Should be called before the delete, it remove the actors from the scene and the listItem from the tree
	*/
	void destruct(vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree);

	void destructCameras(vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree);
	void destructTextures(vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree);
	void destructVolumes(vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree);
	void destructVolume(vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree);

	void deleteCamera(int cameraIndex, vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree);
	void deleteVolume(int volumeIndex, vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree);

	bool isLoaded() const { return loaded; };
	bool getIsPointCloud() const { return isPointCloud; };
	bool getHasColor() { return hasColor; };
	bool getHasTexture() { return hasTexture; };

	bool exportMesh(const std::string& filePath);

	std::string filePath = "";

	std::vector<vtkSmartPointer<vtkActor>> actors;
	vtkSmartPointer<vtkActor> volumeActor = nullptr;
	std::vector<std::string> textureNames;
	std::vector<vtkSmartPointer<vtkTexture>> textures;
	std::vector<Camera*> cameras;
	vtkSmartPointer<vtkActor> camerasPath = nullptr;

	//Use the addVolume to add some volumes to the mesh
	std::vector<Volume*> volumes;
	//Just to track how many volumes already passed through this mesh
	int qtdVolumes = 0;

	//360 mode
	void update360ClickPoints(vtkSmartPointer<vtkRenderer> renderer, Camera* newCam);

	//Create a pickList from the actors, and set the picker to pick from list
	void createPickList(vtkSmartPointer<vtkAbstractPicker> picker);

	//Set the visibility of all volumes
	void setVolumesVisibility(bool visible);
	bool getVolumesVisibility() const;
	void addVolume(Volume* vol) 
	{
		volumes.push_back(vol);
		qtdVolumes++;
	};

	void setMeshVisibility(bool visible);
	bool getMeshVisibility() const { return visible; };

	void setVolumeVisibility(bool visible);
	bool getVolumeVisibility() const;


	void setCamerasVisibility(bool visible);
	//True if some camera is visible
	bool getCamerasVisibility() const;
	//Enable/disable the textures
	void setTextureVisibility(bool visible);
	bool getTextureVisibility() const { return textureVisibility; };

	//Create a line connecting the cameras origins
	void createCamerasPath(vtkSmartPointer<vtkRenderer> renderer);

	//Enable/disable the camera path
	void setCameraPathVisibility(bool visible);
	bool getCameraPathVisibility() const;

	//Used to sort the cameras by filename.
	void sortCameras();

	//Get the PolyData of the mesh
	vtkSmartPointer<vtkPolyData> getPolyData();

	//Get the CellLocator of the mesh
	vtkSmartPointer<vtkCellLocator> getCellLocator();

	//Set if the actors can be picked
	void setPickable(bool pickable);

	//Tranform the mesh using T
	void transform(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkTransform> T);

	//Set the calibration
	void setCalibration(Calibration* cal);

	//Get the calibration
	Calibration* getCalibration() const { return calibration; };

	//Change the points of a point cloud
	void updatePoints(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkUnsignedCharArray> colors, vtkSmartPointer<vtkFloatArray> normals);
	void updatePoints(vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkUnsignedCharArray> colors, vtkSmartPointer<vtkFloatArray> normals);
	void updatePoints(vtkDataObject* data);

	//Change the cells of a mesh
	void updateCells(vtkDataObject* data);

	//Change the cells of an actor of this mesh
	void updateCells(vtkDataObject* data, unsigned int actorIndex);

	//Used to update the actors after changes in VR
	void updateActors(std::vector<vtkSmartPointer<vtkActor> > newActors);

	/*
	Method to reduce the number of faces, define a percentage of desired reduction
	Only works with triangular meshes without texture (the way VTK handles texture, all vertices are considered borders in the fast quadic simplification algorithm)
	*/
	void fastQuadricSimplification(float perc);

	//Save the cameras using the SFM format
	bool saveSFM(const std::string& filename) const;

	//Save the cameras using the NVM format
	bool saveNVM(const std::string& filename) const;

	//Tree
	//Set the wxTreeItem that represents this mesh on the tree
	void setListItemMesh(const wxTreeListItem& listItem) { listItemMesh = listItem; };
	wxTreeListItem getListItemMesh() const { return listItemMesh; };

	//Set the wxTreeItem that represents the item "cameras" of this mesh on the tree
	void setListItemMeshCameras(const wxTreeListItem& listItem) { listItemMeshCameras = listItem; };
	wxTreeListItem getListItemMeshCameras() const { return listItemMeshCameras; };

	//Set the wxTreeItem that represents the texture of this mesh on the tree
	void setListItemMeshTexture(const wxTreeListItem& listItem) { listItemMeshTexture = listItem; };
	wxTreeListItem getListItemMeshTexture() const { return listItemMeshTexture; };

	//Set the wxTreeItem that represents the volumes of this mesh on the tree
	void setListItemMeshVolumes(const wxTreeListItem& listItem) { listItemMeshVolumes = listItem; };
	wxTreeListItem getListItemMeshVolumes() const { return listItemMeshVolumes; };

	//Set the wxTreeItem that represents the volume actor of this mesh on the tree
	void setListItemMeshVolume(const wxTreeListItem& listItem) { listItemMeshVolume = listItem; };
	wxTreeListItem getListItemMeshVolume() const { return listItemMeshVolume; };
};
