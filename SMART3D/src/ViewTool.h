#pragma once
#include <vtkAbstractWidget.h>
#include <vtkSmartPointer.h>
#include <vector>
#include "ViewToolRepresentation.h"

class wxTreeListCtrl;
class Mesh;

class ViewTool : public vtkAbstractWidget
{
public:
	/**
	* Method to instantiate class.
	*/
	static ViewTool *New();

	//@{
	/**
	* Standard methods for class.
	*/
	vtkTypeMacro(ViewTool, vtkAbstractWidget);
	void PrintSelf(ostream& os, vtkIndent indent) override;
	//@}

	/**
	* Specify an instance of vtkWidgetRepresentation used to represent this
	* widget in the scene. Note that the representation is a subclass of vtkProp
	* so it can be added to the renderer independent of the widget.
	*/
	void SetRepresentation(ViewToolRepresentation *r)
	{
		this->Superclass::SetWidgetRepresentation(reinterpret_cast<ViewToolRepresentation*>(r));
	}

	/**
	* Return the representation as a vtkBorderRepresentation.
	*/
	ViewToolRepresentation *GetRepresentation()
	{
		return reinterpret_cast<ViewToolRepresentation*>(this->WidgetRep);
	}

	/**
	* Create the default widget representation if one is not set.
	*/
	void CreateDefaultRepresentation() override;

	void setMeshVector(std::vector<Mesh*>* meshVector) { this->meshVector = meshVector; };
	void setTree(wxTreeListCtrl* treeMesh) { this->treeMesh = treeMesh; };

protected:
	ViewTool();
	~ViewTool() override;

	/**
	* Subclasses generally implement this method. The SelectRegion() method
	* offers a subclass the chance to do something special if the interior
	* of the widget is selected.
	*/
	virtual void SelectRegion(double eventPos[2]);

	//processes the registered events
	static void SelectAction(vtkAbstractWidget*);
	static void TranslateAction(vtkAbstractWidget*);
	static void EndSelectAction(vtkAbstractWidget*);
	static void MoveAction(vtkAbstractWidget*);

	// helper methods for cursoe management
	void SetCursor(int State) override;

	//widget state
	int WidgetState;
	enum _WidgetState { Start = 0, Define, Manipulate, Selected };

	std::vector<Mesh*>* meshVector;
	wxTreeListCtrl* treeMesh;
	Mesh* getMeshFromTree();

	void changeView(vtkSmartPointer<vtkRenderer> renderer, int interactionState);

private:
	ViewTool(const ViewTool&) = delete;
	void operator=(const ViewTool&) = delete;
};