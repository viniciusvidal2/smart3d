#pragma once
#include <vtkSmartPointer.h>
#include <wx/treelist.h>

class vtkCaptionActor2D;
class vtkPolyData;
class vtkActor;
class vtkRenderer;
class vtkTransform;

class Volume
{
private:
	wxTreeListItem listItem = nullptr;
	bool visible;
public:
	vtkSmartPointer<vtkCaptionActor2D> text = nullptr;
	vtkSmartPointer<vtkPolyData> polyData = nullptr;
	vtkSmartPointer<vtkActor> actor = nullptr;
	double volume = -1;
	//Index of the volume in the tree(Just to identify it, not the index of the vector)
	int index = -1;

	Volume(bool visible);
	~Volume();

	/*
	Should be called before the delete, it remove the actors from the scene and the listItem from the tree
	*/
	void destruct(vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree);

	void setVisibility(bool visibility);
	bool getVisibility();

	void setListItem(const wxTreeListItem& item) { listItem = item; };
	wxTreeListItem getListItem() const { return listItem; };

	/*
	Update the measure unit
	*/
	void updateText(const std::string&  measureUnit);

	/*
	Re-calculate the volume and update the measure unit
	*/
	void updateCalibration(double calibration, const std::string& measureUnit);

	/*
	Tranform the mesh using T
	*/
	void transform(vtkSmartPointer<vtkTransform> T);

	/*
	Update the text position
	*/
	void updateTextPosition();

};