#pragma once
#include <vtkInteractionWidgetsModule.h> // For export macro
#include <vtk3DWidget.h>
#include <vtkSmartPointer.h>

class vtkTextProperty;
class vtkTextActor;
class Camera;

class UpdateCameraTool : public vtk3DWidget
{
public:
	/**
	* Instantiate the object.
	*/
	static UpdateCameraTool *New();

	vtkTypeMacro(UpdateCameraTool, vtk3DWidget);

	//@{
	/**
	* Methods that satisfy the superclass' API.
	*/
	virtual void SetEnabled(int);
	virtual void PlaceWidget(double bounds[6]) {};
	//@}

	//@{
	/**
	* Set the mesh that is going to be used
	*/
	void setCamera(Camera* cam);
	//@}

protected:
	UpdateCameraTool();
	~UpdateCameraTool();

	//Delete what is necessary to properly disable/kill this class
	void destruct();

	//handles the events
	static void ProcessEvents(vtkObject* object, unsigned long event,
		void* clientdata, void* calldata);

	// ProcessEvents() dispatches to these methods.
	void OnKeyPressed();

	//Text
	float textXPosition = 55;
	float textYPosition = 20;
	vtkSmartPointer<vtkTextProperty> textProperty = nullptr;
	vtkSmartPointer<vtkTextActor> textActor = nullptr;
	void updateTextScale();
	void updateText();

	//Movements
	float step = 0.1;
	bool translate = false;

	Camera* camera = nullptr;

	//Instructions
	bool showDialog = true;

private:
	UpdateCameraTool(const UpdateCameraTool&) VTK_DELETE_FUNCTION;
	void operator=(const UpdateCameraTool&) VTK_DELETE_FUNCTION;
};

