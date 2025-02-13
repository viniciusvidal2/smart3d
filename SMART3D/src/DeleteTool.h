#pragma once
#include <vtkInteractionWidgetsModule.h> // For export macro
#include <vtk3DWidget.h>
#include <vtkSmartPointer.h>
#include <vector>

class vtkSelection;
class vtkActor;
class vtkPolyData;
class LineWidget;
class Mesh;

class DeleteTool : public vtk3DWidget
{
public:
    /**
    * Instantiate the object.
    */
    static DeleteTool *New();

    vtkTypeMacro(DeleteTool, vtk3DWidget);

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
    void setMesh(Mesh* mesh);
    //@}

    bool enterKeyPressed();

protected:
    DeleteTool();
    ~DeleteTool();

    //handles the events
    static void ProcessEvents(vtkObject* object, unsigned long event,
        void* clientdata, void* calldata);

    vtkSmartPointer<LineWidget> lineWidget = nullptr;

    // Controlling ivars
    void UpdateRepresentation();
    void createActorPolygon();


    Mesh* mesh = nullptr;

    //CELL or POINT
    int fieldType = -1;

    std::vector<vtkSmartPointer<vtkPolyData>> polyDatas;
    std::vector<vtkSmartPointer<vtkActor>> actorsPolygon;
    std::vector<vtkSmartPointer<vtkSelection>> selections;

private:
    DeleteTool(const DeleteTool&) VTK_DELETE_FUNCTION;
    void operator=(const DeleteTool&) VTK_DELETE_FUNCTION;
};
