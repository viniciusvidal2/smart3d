#pragma once
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vector>

class vtkAbstractPicker;
class vtkPropPicker;
class vtkCellPicker;
class vtkPolygon;
class wxTreeListCtrl;
class Mesh;

class InteractorStyle : public vtkInteractorStyleTrackballCamera
{
private:
	~InteractorStyle();

	static void ProcessEvents(vtkObject* object, unsigned long event,
		void* clientdata, void* calldata);

	vtkNew<vtkPropPicker>  picker;
	vtkNew<vtkCellPicker> cellPicker;

	bool leftButtonDown = false;

	//mode 360 image
	bool mode360Image = false;

	//FlyToPoint
	bool flyToPoint = false;

public:
	static InteractorStyle* New();
	vtkTypeMacro(InteractorStyle, vtkInteractorStyleTrackballCamera);
	//For the image double click operation
	std::vector<Mesh*>* meshVector;
	wxTreeListCtrl* treeMesh;

	//mode 360 image
	void setMode360Image(bool mode360Image);
	bool getMode360Image() const { return mode360Image; };

	virtual void OnLeftButtonDown();
	virtual void OnLeftButtonUp();
	virtual void OnMouseWheelForward();
	virtual void OnMouseWheelBackward();
	virtual void OnMouseMove();
	void doubleClick();

	//flyToPoint
	void setFlyToPoint(bool flyToPoint) { this->flyToPoint = flyToPoint; };
	bool getFlyToPoint() const { return flyToPoint; };



	/*
	Pick the position of the mouse in 3D if the actor is != NULL
	*/
	int getMousePosition(vtkSmartPointer<vtkAbstractPicker> picker, double* point);
	/*
	Pick in the pointDisplay position if the actor is != NULL
	*/
	int getMousePosition(vtkSmartPointer<vtkAbstractPicker> picker, double* pointDisplay, double* point);
	/*
	Pick the position of the mouse in 3D, using a Picker
	*/
	int getMousePosition(double* point);
	/*
	Pick the position of the mouse in 3D if the actor is == actor(Parameter)
	*/
	int getMousePosition(double* point, vtkSmartPointer<vtkActor> actor);
	/*
	Do a pick in the specified display position
	Input-> display position
	Output-> world position
	*/
	double* pickPosition(double* pointDisplay);
	/*
	p1->Camera position
	PointCheckCam->Camera focal point
	polygon->Polygon that is treated as a plane
	*/
	bool IntersectPlaneWithLine(double* p1, vtkSmartPointer<vtkPolygon> polygon, double* pointCheckCam);
	double* getDisplayPosition(double* point);
};