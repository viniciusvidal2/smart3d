#include "ViewToolRepresentation.h"

#include <vtkObjectFactory.h>
#include <vtkImageMapper.h>
#include <vtkActor2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkImageData.h>

#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

#include "Utils.h"

vtkStandardNewMacro(ViewToolRepresentation);

//-------------------------------------------------------------------------
ViewToolRepresentation::ViewToolRepresentation()
{
	this->InteractionState = ViewToolRepresentation::Outside;

	//Images
	viewImages.push_back(wxBitmap(wxICON(ICON_TOP_VIEW)).ConvertToImage());
	viewImages.push_back(wxBitmap(wxICON(ICON_BOTTOM_VIEW)).ConvertToImage());
	viewImages.push_back(wxBitmap(wxICON(ICON_FRONT_VIEW)).ConvertToImage());
	viewImages.push_back(wxBitmap(wxICON(ICON_BACK_VIEW)).ConvertToImage());
	viewImages.push_back(wxBitmap(wxICON(ICON_LEFT_VIEW)).ConvertToImage());
	viewImages.push_back(wxBitmap(wxICON(ICON_RIGHT_VIEW)).ConvertToImage());

	for (auto viewImage : viewImages)
	{
		vtkSmartPointer<vtkImageMapper> imageMapper = vtkSmartPointer<vtkImageMapper>::New();
		imageMapper->SetInputData(Utils::wxImage2ImageData(viewImage));
		imageMapper->SetColorWindow(255);
		imageMapper->SetColorLevel(127.5);
		imageMapper->Update();

		viewActors.push_back(vtkSmartPointer<vtkActor2D>::New());
		viewActors.back()->SetMapper(imageMapper);
		viewActorsDisplayPosition.push_back(new int[2]);
	}
}

//-------------------------------------------------------------------------
ViewToolRepresentation::~ViewToolRepresentation()
{
	viewActors.clear();
	viewImages.clear();
}

bool ViewToolRepresentation::isInsideButton(int x, int y, int* pos)
{
	if ((x > pos[0] && x < pos[0] + HalfButtonSize * 2) && (y > pos[1] && y < pos[1] + HalfButtonSize * 2))
	{
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------
void ViewToolRepresentation::StartWidgetInteraction(double eventPos[2])
{
	this->StartEventPosition[0] = eventPos[0];
	this->StartEventPosition[1] = eventPos[1];
}

//-------------------------------------------------------------------------
void ViewToolRepresentation::WidgetInteraction(double eventPos[2])
{
	this->Modified();
	this->BuildRepresentation();
}

//-------------------------------------------------------------------------
int ViewToolRepresentation::ComputeInteractionState(int X, int Y, int vtkNotUsed(modify))
{
	if (X != 0 && Y != 0)
	{
		for (int i = 0; i < viewActorsDisplayPosition.size(); i++)
		{
			if (isInsideButton(X, Y, viewActorsDisplayPosition.at(i)))
			{
				this->InteractionState = i + 1;
				return this->InteractionState;
			}
		}
	}
	this->InteractionState = ViewToolRepresentation::Outside;
	return this->InteractionState;
}

//-------------------------------------------------------------------------
void ViewToolRepresentation::BuildRepresentation()
{
	if (this->Renderer &&
		(this->GetMTime() > this->BuildTime ||
		(this->Renderer->GetVTKWindow() &&
			this->Renderer->GetVTKWindow()->GetMTime() > this->BuildTime)))
	{
		int *size = this->Renderer->GetSize();
		if (0 == size[0] || 0 == size[1])
		{
			// Renderer has no size yet: wait until the next
			// BuildRepresentation...
			return;
		}
		double tx = HalfButtonSize + 20;
		double ty = size[1] - (HalfButtonSize + 25);

		for (int i = 0; i < viewActors.size(); i++)
		{
			viewActorsDisplayPosition.at(i)[0] = 10 + i * (tx);
			viewActorsDisplayPosition.at(i)[1] = ty;
			viewActors.at(i)->SetDisplayPosition(viewActorsDisplayPosition.at(i)[0], ty);
		}

		this->BuildTime.Modified();
	}
}
//-------------------------------------------------------------------------
void ViewToolRepresentation::GetActors2D(vtkPropCollection *pc)
{
	for (auto actor2D : viewActors)
	{
		pc->AddItem(actor2D);
	}
}

//-------------------------------------------------------------------------
void ViewToolRepresentation::ReleaseGraphicsResources(vtkWindow *w)
{
	for (auto actor2D : viewActors)
	{
		actor2D->ReleaseGraphicsResources(w);
	}
}

//-------------------------------------------------------------------------
int ViewToolRepresentation::RenderOverlay(vtkViewport *w)
{
	this->BuildRepresentation();
	int count = viewActors.at(0)->RenderOverlay(w);
	for (auto actor2D : viewActors)
	{
		count += actor2D->RenderOverlay(w);
	}
	return count;
}

//-------------------------------------------------------------------------
int ViewToolRepresentation::RenderOpaqueGeometry(vtkViewport *w)
{
	this->BuildRepresentation();
	int count = viewActors.at(0)->RenderOpaqueGeometry(w);
	for (auto actor2D : viewActors)
	{
		count += actor2D->RenderOpaqueGeometry(w);
	}
	return count;
}

//-----------------------------------------------------------------------------
int ViewToolRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport *w)
{
	this->BuildRepresentation();
	int count = viewActors.at(0)->RenderTranslucentPolygonalGeometry(w);
	for (auto actor2D : viewActors)
	{
		count += actor2D->RenderTranslucentPolygonalGeometry(w);
	}
	return count;
}

//-----------------------------------------------------------------------------
// Description:
// Does this prop have some translucent polygonal geometry?
int ViewToolRepresentation::HasTranslucentPolygonalGeometry()
{
	this->BuildRepresentation();
	int count = viewActors.at(0)->HasTranslucentPolygonalGeometry();
	for (auto actor2D : viewActors)
	{
		count += actor2D->HasTranslucentPolygonalGeometry();
	}
	return count;
}

//-------------------------------------------------------------------------
void ViewToolRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
}