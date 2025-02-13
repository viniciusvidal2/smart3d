#pragma once

#include <vtkSmartPointer.h>
#include <wx/treelist.h>

class GPSData;
class vtkImageActor;
class vtkPolygon;
class vtkMatrix4x4;
class vtkRenderer;
class vtkTransform;
class vtkActor;
class vtkPolyData;

class Camera
{
private:
	wxTreeListItem listItemCamera = nullptr;
	wxTreeListItem listItemGPSData = nullptr;
	bool loaded = false;
	bool visible = false;
	bool is360 = false;

	double focalX = -1;
	double focalY = -1;
	vtkSmartPointer<vtkMatrix4x4> matrixRt = nullptr;
	vtkSmartPointer<vtkMatrix4x4> matrixRtInverted = nullptr;
public:
	Camera();
	//Load the camera from its line in the SFM/NVM file
	Camera(vtkSmartPointer<vtkRenderer> renderer, const std::string& cameraParameters);
	~Camera();

	/*
	Should be called before the delete, it remove the actors from the scene and the listItem from the tree
	*/
	void destruct(vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree);

	std::string filePath = "";
	int width = -1;
	int height = -1;
	/*
	The View Up vector of the camera
	*/
	double* viewUp = nullptr;
	/*
	0 - UP
	1 - Right
	2 - Down
	3 - Left
	*/
	int viewUpDirection = -1;
	/*
	GPS Data extracted from the image.
	*/
	GPSData* gpsData = nullptr;

	bool isLoaded() const { return loaded; };

	/*
	Tranform the mesh using T
	*/
	void transform(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkTransform> T);

	void setListItemCamera(const wxTreeListItem& listItem) { listItemCamera = listItem; };
	wxTreeListItem getListItemCamera() const { return listItemCamera; };
	void setVisibility(bool visible);
	bool getVisibility() const { return visible; };
	/*
	Calc the camera points:
	origin of the camera = [0]
	[1]--------[2]
	|		    |
	|----[5]----|
	|           |
	[4]--------[3]
	[5]->the distance(z coordinate) is actually 5% less than the others points, to avoid exceed the image with zoon when we do Utils::updateCamera
	*/
	bool calcCameraPoints();
	int changeViewUp();



	vtkSmartPointer<vtkActor> actorFrustum = nullptr;
	vtkSmartPointer<vtkImageActor> imageActor = nullptr;
	/*Vector with these points
	origin of the camera = [0]
	[1]--------[2]
	|		    |
	|----[5]----|
	|           |
	[4]--------[3]
	[5]->the distance(z coordinate) is actually 5% less than the others points, to avoid exceed the image with zoom when we do Utils::updateCamera
	*/
	std::vector<double*> cameraPoints;
	/*
	We use this to make some colliding tests faster
	[1]--------[2]
	|		    |
	|           |
	|           |
	[4]--------[3]
	*/
	vtkSmartPointer<vtkPolygon> imagePolygon = nullptr;

	/*
	Get the Latitude, Longitude and Altitude data.
	*/
	void updateGPSData();

	double getDistanceBetweenCameraCenters(Camera* c);
	void createActorFrustrum(vtkSmartPointer<vtkRenderer> renderer);
	void createImageActor(vtkSmartPointer<vtkRenderer> renderer);

	void updateFocalDistance(vtkSmartPointer<vtkRenderer> renderer, double focalX, double focalY);
	void updateMatrixRt(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkMatrix4x4> matrixRt);

	double getFocalX() const { return focalX; };
	double getFocalY() const { return focalY; };
	void setMatrixRt(vtkSmartPointer<vtkMatrix4x4> matrixRt);
	vtkSmartPointer<vtkMatrix4x4> getMatrixRt();
	void getPitchVector(double* pitchVector);
	void getYawVector(double* yawVector);

	std::string getSFM() const;
	std::string getNVM() const;

	//Project a 3D point in the camera plane, to test if it is inside the camera's frustum
	bool isPointOnCameraPlane(double point[3]);

	//Creates a closed frustum with the image plane with the distance to the origin point = dist
	vtkSmartPointer<vtkPolyData> getClosedFrustum(double dist = 0.5);

	//360 image, transform is not implemented
	//Spheres used to create point to go to this 360 camera
	vtkSmartPointer<vtkActor> image360ClickPoint = nullptr;
	vtkSmartPointer<vtkActor> image360Actor = nullptr;
	void setIs360(vtkSmartPointer<vtkRenderer> renderer, bool is360);
	bool getIs360() const { return is360; };
};