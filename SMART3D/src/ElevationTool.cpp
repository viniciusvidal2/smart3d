#include "ElevationTool.h"

#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkXYPlotActor.h>
#include <vtkSliderWidget.h>
#include <vtkSliderRepresentation.h>
#include <vtkPropPicker.h>
#include <vtkActor.h>
#include <vtkMapper.h>
#include <vtkCellLocator.h>
#include <vtkElevationFilter.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkCutter.h>
#include <vtkStripper.h>
#include <vtkSortDataArray.h>
#include <vtkAxisActor2D.h>
#include <vtkPlane.h>

#include <wx/msgdlg.h>

#include "LineWidget.h"
#include "captionActor2D.h"
#include "sliderRep2D.h"
#include "Draw.h"
#include "Utils.h"
#include "Calibration.h"
#include "Mesh.h"

vtkStandardNewMacro(ElevationTool);

//----------------------------------------------------------------------------
ElevationTool::ElevationTool() : vtk3DWidget()
{
	this->EventCallbackCommand->SetCallback(ElevationTool::ProcessEvents);
}

//----------------------------------------------------------------------------
ElevationTool::~ElevationTool()
{
	destruct();
}

void ElevationTool::destruct()
{
	if (textActor)
	{
		this->CurrentRenderer->RemoveActor2D(textActor);
		textActor = nullptr;
	}
	if (elevationMapActor)
	{
		this->CurrentRenderer->RemoveActor(elevationMapActor);
		elevationMapActor = nullptr;
	}
	if (plotElevation)
	{
		this->CurrentRenderer->RemoveActor2D(plotElevation);
		plotElevation = nullptr;
	}
	if (sliderWidget)
	{
		sliderWidget->EnabledOff();
		sliderWidget = nullptr;
	}
	propPicker = nullptr;
	outputElevationFilter = nullptr;
	elevationFilter = nullptr;
	setZeroByMouse = false;
	//ElevationProfile
	lineWidget = nullptr;
	if (elevationProfileActor)
	{
		this->CurrentRenderer->RemoveActor(elevationProfileActor);
		elevationProfileActor = nullptr;
	}
	lastElevationHeight = nullptr;
	if (mesh)
	{
		mesh->setMeshVisibility(treeMesh->GetCheckedState(mesh->getListItemMesh()));
	}
	mesh = nullptr;
}

//----------------------------------------------------------------------------
void ElevationTool::SetEnabled(int enabling)
{
	if (!this->Interactor)
	{
		return;
	}

	if (enabling) //------------------------------------------------------------
	{
		if (this->Enabled || !mesh || !treeMesh) //already enabled, just return
		{
			return;
		}
		if (!this->CurrentRenderer)
		{
			this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(
				this->Interactor->GetLastEventPosition()[0],
				this->Interactor->GetLastEventPosition()[1]));
			if (!this->CurrentRenderer)
			{
				return;
			}
		}
		this->Enabled = 1;

		// listen for the following events
		vtkRenderWindowInteractor *i = this->Interactor;
		i->AddObserver(vtkCommand::MiddleButtonPressEvent,
			this->EventCallbackCommand, this->Priority);
		i->AddObserver(vtkCommand::KeyPressEvent,
			this->EventCallbackCommand, this->Priority);

		//Actors
		if (outputElevationFilter)
		{
			elevationMapActor = Draw::createPolyData(this->CurrentRenderer, outputElevationFilter);
		}
		//Picking stuff
		if (!propPicker)
		{
			this->propPicker = vtkSmartPointer<vtkPropPicker>::New();
			propPicker->AddPickList(elevationMapActor);
			propPicker->PickFromListOn();
		}
		createSlider();

		sliderWidget->AddObserver(vtkCommand::InteractionEvent,
			this->EventCallbackCommand, this->Priority);
		sliderWidget->AddObserver(vtkCommand::EndInteractionEvent,
			this->EventCallbackCommand, this->Priority);


		if (!lineWidget)
		{
			lineWidget = vtkSmartPointer<LineWidget>::New();
			lineWidget->SetInteractor(this->Interactor);
			lineWidget->setMaxNumberOfNodes(2);
			mesh->setMeshVisibility(false);
			vtkNew<LineWidgetRepresentation> rep;
			rep->addProp(elevationMapActor);
			vtkNew<vtkCellLocator> cellLocator;
			cellLocator->SetDataSet(elevationMapActor->GetMapper()->GetInputAsDataSet());
			cellLocator->BuildLocator();
			rep->addLocator(cellLocator);
			lineWidget->SetRepresentation(rep);
		}
		lineWidget->EnabledOn();
		lineWidget->AddObserver(vtkCommand::InteractionEvent, this->EventCallbackCommand, this->Priority);
		lineWidget->AddObserver(vtkCommand::EndInteractionEvent, this->EventCallbackCommand, this->Priority);

		if (mesh)
		{
			mesh->setMeshVisibility(false);
		}

		this->InvokeEvent(vtkCommand::EnableEvent, nullptr);
	}

	else //disabling----------------------------------------------------------
	{
		if (!this->Enabled) //already disabled, just return
		{
			return;
		}
		this->Enabled = 0;

		// don't listen for events any more
		this->Interactor->RemoveObserver(this->EventCallbackCommand);
		sliderWidget->RemoveObserver(this->EventCallbackCommand);

		//ElevationProfile
		if (lineWidget)
		{
			lineWidget->EnabledOff();
			lineWidget->RemoveObserver(this->EventCallbackCommand);
			lineWidget = nullptr;
			if (elevationProfileActor)
			{
				this->CurrentRenderer->RemoveActor(elevationProfileActor);
				elevationProfileActor = nullptr;
			}
			if (plotElevation)
			{
				this->CurrentRenderer->RemoveActor2D(plotElevation);
				plotElevation = nullptr;
			}
		}
		destruct();

		this->InvokeEvent(vtkCommand::DisableEvent, nullptr);
		this->SetCurrentRenderer(nullptr);
	}

	this->Interactor->Render();
}

//----------------------------------------------------------------------------
void ElevationTool::ProcessEvents(vtkObject* vtkNotUsed(object),
	unsigned long event,
	void* clientdata,
	void* vtkNotUsed(calldata))
{
	ElevationTool* self =
		reinterpret_cast<ElevationTool *>(clientdata);


	//okay, let's do the right thing
	if (event == vtkCommand::MiddleButtonPressEvent)
	{
		self->OnMiddleButtonDown();
	}
	else if (event == vtkCommand::KeyPressEvent)
	{
		self->OnKeyPressed();
	}
	else if (event == vtkCommand::EndInteractionEvent || event == vtkCommand::InteractionEvent)
	{
		self->createElevationProfileLine();
		self->OnSliderChanged();
	}
}

void ElevationTool::updateElevationMap()
{
	if (sliderWidget)
	{
		vtkSmartPointer<vtkSliderRepresentation> slider = static_cast<vtkSliderRepresentation *>(sliderWidget->GetRepresentation());
		if (mesh->getCalibration()->getCalibratedUsingGPSData())
		{
			double porc = (slider->GetValue() - slider->GetMinimumValue()) / (slider->GetMaximumValue() - slider->GetMinimumValue());
			elevationFilter->SetHighPoint(0.0, 0.0, porc * (bounds[5] - bounds[4]) + bounds[4]);
		}
		else
		{
			elevationFilter->SetHighPoint(0.0, 0.0, slider->GetValue() + bounds[4]);
		}
	}
	else
	{
		elevationFilter->SetHighPoint(0.0, 0.0, bounds[4]);
	}
	if (!lastElevationHeight)
	{
		Utils::createDoubleVector(elevationFilter->GetHighPoint(), lastElevationHeight);
	}
	else
	{
		if (Utils::isSamePoint(lastElevationHeight, elevationFilter->GetHighPoint()))
		{
			return;
		}
		else
		{
			delete lastElevationHeight;
			Utils::createDoubleVector(elevationFilter->GetHighPoint(), lastElevationHeight);
		}
	}
	elevationFilter->Update();
	outputElevationFilter->GetPointData()->SetScalars(elevationFilter->GetOutput()->GetPointData()->GetArray("Elevation"));
}

void ElevationTool::updateText()
{
	if (textActor)
	{
		textActor->SetCaption(mesh->getCalibration()->getCalibratedText(getPointElevation(textActor->GetAttachmentPoint())).c_str());
	}
}

double ElevationTool::getElevationDistance(double dist)
{
	if (!sliderWidget)
	{
		return -1;
	}
	return dist - (static_cast<vtkSliderRepresentation *>(sliderWidget->GetRepresentation())->GetValue() + bounds[4]);
}

double ElevationTool::getPointElevation(double * point)
{
	if (!sliderWidget)
	{
		return -1;
	}
	double dist = 0;
	vtkSmartPointer<vtkSliderRepresentation> rep = (static_cast<vtkSliderRepresentation *>(sliderWidget->GetRepresentation()));
	if (mesh->getCalibration()->getCalibratedUsingGPSData())
	{
		double porc = (point[2] - bounds[4]) / (bounds[5] - bounds[4]);
		dist = porc * (rep->GetMaximumValue() - rep->GetMinimumValue()) + rep->GetMinimumValue();
		dist = dist - rep->GetValue() + rep->GetMinimumValue();
	}
	else
	{
		dist = point[2] - rep->GetValue() + bounds[4];
	}
	return dist;
}

void ElevationTool::setMesh(Mesh * mesh)
{
	if (mesh)
	{
		this->mesh = mesh;
		vtkSmartPointer<vtkPolyData> polyMesh = mesh->getPolyData();
		bounds = polyMesh->GetBounds();
		elevationFilter = vtkSmartPointer<vtkElevationFilter>::New();
		elevationFilter->SetInputData(polyMesh);
		//To avoid the bad vector problem with the elevation filter
		if (bounds[5] < 0)
		{
			elevationFilter->SetLowPoint(0.0, 0.0, bounds[5] * 0.99999);
		}
		else
		{
			elevationFilter->SetLowPoint(0.0, 0.0, bounds[5] * 1.00001);
		}
		elevationFilter->SetHighPoint(0.0, 0.0, bounds[4]);
		elevationFilter->Update();
		outputElevationFilter = vtkSmartPointer<vtkPolyData>::New();
		outputElevationFilter->DeepCopy(vtkPolyData::SafeDownCast(elevationFilter->GetOutput()));
		updateElevationMap();
	}
}

//----------------------------------------------------------------------------
void ElevationTool::OnMiddleButtonDown()
{
	double* pickedPosition = new double[3];
	if (getMousePosition(pickedPosition))
	{
		if (textActor)
		{
			this->CurrentRenderer->RemoveActor2D(textActor);
			textActor = nullptr;
		}
		if (setZeroByMouse)
		{
			vtkSmartPointer<vtkSliderRepresentation> rep = (static_cast<vtkSliderRepresentation *>(sliderWidget->GetRepresentation()));
			if (mesh->getCalibration()->getCalibratedUsingGPSData())
			{
				double porc = (pickedPosition[2] - bounds[4]) / (bounds[5] - bounds[4]);
				rep->SetValue(porc * (rep->GetMaximumValue() - rep->GetMinimumValue()) + rep->GetMinimumValue());
			}
			else
			{
				rep->SetValue(pickedPosition[2] - bounds[4]);
			}
			setZeroByMouse = false;
			OnSliderChanged();
		}
		else
		{
			//Text
			textActor = Draw::createNumericIndicator(this->CurrentRenderer, pickedPosition, mesh->getCalibration()->getCalibratedText(getPointElevation(pickedPosition)), 24, 1.0, 1.0, 1.0);
		}
	}
	else
	{
		delete pickedPosition;
		return;
	}
	this->Interactor->Render();
}

void ElevationTool::OnKeyPressed()
{
	char key = this->Interactor->GetKeyCode();
	if (this->Interactor->GetControlKey())
	{
		if (key == 'Y')
		{
			setZeroByMouse = !setZeroByMouse;
		}
	}
}

void ElevationTool::createSlider()
{
	if (!outputElevationFilter)
	{
		return;
	}
	vtkSmartPointer<sliderRep2D> sliderRepresentation = vtkSmartPointer<sliderRep2D>::New();
	if (mesh->getCalibration()->getCalibratedUsingGPSData())
	{
		sliderRepresentation->SetMinimumValue(mesh->getCalibration()->getMinimumAltitude() / mesh->getCalibration()->getScaleFactor());
		sliderRepresentation->SetMaximumValue(mesh->getCalibration()->getMaximumAltitude() / mesh->getCalibration()->getScaleFactor());
	}
	else
	{
		sliderRepresentation->SetMinimumValue(0);
		sliderRepresentation->SetMaximumValue(bounds[5] - bounds[4]);
	}
	sliderRepresentation->SetMinLimitText(mesh->getCalibration()->getCalibratedText(sliderRepresentation->GetMinimumValue()).c_str());
	sliderRepresentation->SetMaxLimitText(mesh->getCalibration()->getCalibratedText(sliderRepresentation->GetMaximumValue()).c_str());
	sliderRepresentation->SetValue(sliderRepresentation->GetMinimumValue());
	sliderRepresentation->SetTitleText("Elevation");
	sliderRepresentation->SetLabelFormat(("%.3f" + mesh->getCalibration()->getMeasureUnit()).c_str());
	sliderRepresentation->setCalibration(mesh->getCalibration());

	sliderWidget = vtkSmartPointer<vtkSliderWidget>::New();
	sliderWidget->SetInteractor(this->Interactor);
	sliderWidget->SetRepresentation(sliderRepresentation);
	sliderWidget->SetAnimationModeToJump();
	sliderWidget->EnabledOn();
}

void ElevationTool::OnSliderChanged()
{
	updateElevationMap();
	updateText();
}

int ElevationTool::getMousePosition(double * point)
{
	if (propPicker->Pick(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], 0, this->CurrentRenderer))
	{
		propPicker->GetPickPosition(point);
		return 1;
	}
	return 0;
}

void ElevationTool::createElevationProfileLine()
{
	if (lineWidget)
	{
		if (plotElevation)
		{
			this->CurrentRenderer->RemoveActor2D(plotElevation);
			plotElevation = nullptr;
		}
		if (elevationProfileActor)
		{
			this->CurrentRenderer->RemoveActor(elevationProfileActor);
			elevationProfileActor = nullptr;
		}
		vtkSmartPointer<LineWidgetRepresentation> rep = lineWidget->GetRepresentation();
		vtkSmartPointer<vtkPoints> pointsLine = rep->getPoints();
		if (!pointsLine || lineWidget->getWidgetState() != LineWidget::Finished)
		{
			return;
		}
		if (pointsLine->GetNumberOfPoints() != 2)
		{
			return;
		}
		double point0[3];
		pointsLine->GetPoint(0, point0);
		double point1[3];
		pointsLine->GetPoint(1, point1);
		double point2[3];
		Utils::createDoubleVector(point0, point2);
		point2[2] += 1;
		vtkNew<vtkPlane> plane;
		double origin[3];
		Utils::getMidpoint(point0, point1, origin);
		plane->SetOrigin(origin);
		double n[3];
		Utils::getNormal(point0, point1, point2, n);
		plane->SetNormal(n);

		vtkSmartPointer<vtkPolyData> meshPolyData = mesh->getPolyData();

		vtkSmartPointer<vtkCutter> cutter = vtkSmartPointer<vtkCutter>::New();
		cutter->SetCutFunction(plane);
		if (meshPolyData->GetPolys()->GetNumberOfCells() == 0)// point cloud
		{
			return;
		}
		else // Mesh
		{
			cutter->SetInputData(meshPolyData);
		}
		vtkSmartPointer<vtkStripper> stripper = vtkSmartPointer<vtkStripper>::New();
		stripper->SetInputConnection(cutter->GetOutputPort());
		stripper->Update();

		vtkSmartPointer<vtkPoints> pointsStripper = stripper->GetOutput()->GetPoints();
		vtkSmartPointer<vtkCellArray> cells = stripper->GetOutput()->GetLines();
		if (pointsStripper->GetNumberOfPoints() == 0)
		{
			return;
		}


		vtkSmartPointer<vtkPoints> newPoints = vtkSmartPointer<vtkPoints>::New();
		vtkIdType *indices;
		vtkIdType numberOfPoints;
		unsigned int lineCount = 0;
		for (cells->InitTraversal(); cells->GetNextCell(numberOfPoints, indices); lineCount++)
		{
			for (vtkIdType i = 0; i < numberOfPoints; i++)
			{
				double point[3];
				pointsStripper->GetPoint(indices[i], point);
				if (isInsideBoundingBox(point, point0, point1))
				{
					newPoints->InsertNextPoint(point);
					if (i + 1 < numberOfPoints)
					{
						double point3[3];
						pointsStripper->GetPoint(indices[i], point3);
						double point4[3];
						pointsStripper->GetPoint(indices[i + 1], point4);
						increasePointDensity(newPoints, point3, point4);
					}
				}
				else
				{
					//test if the next point is inside
					if (i + 1 < numberOfPoints)
					{
						pointsStripper->GetPoint(indices[i + 1], point);
						if (isInsideBoundingBox(point, point0, point1))
						{
							double point3[3];
							pointsStripper->GetPoint(indices[i], point3);
							double point4[3];
							pointsStripper->GetPoint(indices[i + 1], point4);
							increasePointDensity(newPoints, point3, point4);
						}
					}
				}
			}
		}

		numberOfPoints = newPoints->GetNumberOfPoints();
		vtkSortDataArray::SortArrayByComponent(newPoints->GetData(), 0);
		vtkSmartPointer<vtkPoints> finalPoints = vtkSmartPointer<vtkPoints>::New();
		double maxZ = VTK_DOUBLE_MIN;
		vtkIdType indexMaxZ = -1;
		//Plot
		int numberOfPointsPerSegment = 1000;
		int cont = 0;
		for (vtkIdType i = 0; i < numberOfPoints; i++)
		{
			double point[3];
			newPoints->GetPoint(i, point);
			cont = 0;
			while (cont < numberOfPointsPerSegment)
			{
				if (point[2] > maxZ)
				{
					maxZ = point[2];
					indexMaxZ = i;
				}
				i++;
				if (i >= numberOfPoints)
				{
					break;
				}
				newPoints->GetPoint(i, point);
				cont++;
			}
			if (indexMaxZ != -1)
			{
				finalPoints->InsertNextPoint(newPoints->GetPoint(indexMaxZ));
			}
			if (i >= numberOfPoints)
			{
				break;
			}
			maxZ = VTK_DOUBLE_MIN;
			indexMaxZ = -1;
		}

		if (finalPoints->GetNumberOfPoints() < 2)
		{
			return;
		}

		createPlot(finalPoints, point0, point1);

		// Create plane actor
		elevationProfileActor = Draw::create3DLine(this->CurrentRenderer, finalPoints);
		elevationProfileActor->GetMapper()->SetRelativeCoincidentTopologyLineOffsetParameters(0, -66000);
		elevationProfileActor->GetMapper()->Update();
		this->CurrentRenderer->Render();
	}
}

bool ElevationTool::isInsideBoundingBox(double * point, double * pointA, double * pointB)
{
	if (point[0] > pointA[0] && point[0] < pointB[0] || point[0] < pointA[0] && point[0] > pointB[0])
	{
		if (point[1] > pointA[1] && point[1] < pointB[1] || point[1] < pointA[1] && point[1] > pointB[1])
		{
			return true;
		}
	}
	return false;
}

void ElevationTool::increasePointDensity(vtkSmartPointer<vtkPoints> points, double * point1, double * point2)
{
	//double minDist = 0.0000001;
	double numDiv = 1000;//ceil(sqrt(vtkMath::Distance2BetweenPoints(point1,point2))/minDist);
	double xIncrement = abs(point2[0] - point1[0]) / numDiv;
	double yIncrement = abs(point2[1] - point1[1]) / numDiv;
	double zIncrement = abs(point2[2] - point1[2]) / numDiv;
	double pointLine[3] = { point1[0],point1[1],point1[2] };
	for (vtkIdType i = 0; i < numDiv; i++)
	{
		if (point1[0] > point2[0])
		{
			pointLine[0] -= xIncrement;
		}
		else
		{
			pointLine[0] += xIncrement;
		}
		if (point1[1] > point2[1])
		{
			pointLine[1] -= yIncrement;
		}
		else
		{
			pointLine[1] += yIncrement;
		}
		if (point1[2] > point2[2])
		{
			pointLine[2] -= zIncrement;
		}
		else
		{
			pointLine[2] += zIncrement;
		}
		points->InsertNextPoint(pointLine);
	}
}

void ElevationTool::createPlot(vtkSmartPointer<vtkPoints> points, double* point0Line, double* point1Line)
{
	int numberOfPoints = points->GetNumberOfPoints();
	vtkSmartPointer<vtkDataArray> xAxis = vtkDataArray::CreateDataArray(VTK_DOUBLE);
	xAxis->SetNumberOfTuples(numberOfPoints);
	vtkSmartPointer<vtkDataArray> yAxis = vtkDataArray::CreateDataArray(VTK_DOUBLE);
	yAxis->SetNumberOfTuples(numberOfPoints);
	double* initialPoint;
	if (point0Line[0] < point1Line[0])
	{
		initialPoint = point0Line;
	}
	else
	{
		initialPoint = point1Line;
	}
	double distX, distY;
	for (vtkIdType i = 0; i < numberOfPoints; i++)
	{
		distX = mesh->getCalibration()->getCalibratedDistance(Utils::distanceBetween2DisplayPoints(initialPoint, points->GetPoint(i)));
		xAxis->SetTuple(i, &distX);
		distY = mesh->getCalibration()->getCalibratedDistance(getPointElevation(points->GetPoint(i)));
		yAxis->SetTuple(i, &distY);
	}

	vtkSmartPointer<vtkFieldData> fieldData = vtkSmartPointer<vtkFieldData>::New();
	fieldData->AllocateArrays(2);
	fieldData->AddArray(xAxis);
	fieldData->AddArray(yAxis);

	vtkSmartPointer<vtkDataObject> dataObject = vtkSmartPointer<vtkDataObject>::New();
	dataObject->SetFieldData(fieldData);


	plotElevation = vtkSmartPointer<vtkXYPlotActor>::New();
	plotElevation->AddDataObjectInput(dataObject);
	plotElevation->SetTitle("");
	plotElevation->SetXTitle("");
	plotElevation->SetYTitle("");
	plotElevation->SetXValuesToValue();
	plotElevation->SetGlyphSize(0.05);
	plotElevation->SetWidth(0.4);
	plotElevation->SetHeight(0.4);
	plotElevation->SetPosition(0.01, 0.58);
	plotElevation->PickableOff();
	plotElevation->PlotCurvePointsOn();
	plotElevation->PlotCurveLinesOn();

	plotElevation->SetDataObjectXComponent(0, 0);
	plotElevation->SetDataObjectYComponent(0, 1);
	plotElevation->SetPlotColor(0, 1.0, 0.0, 0.0);
	plotElevation->GetXAxisActor2D()->SetLabelFormat("%.2f");
	plotElevation->GetXAxisActor2D()->SetFontFactor(0.8);
	plotElevation->GetYAxisActor2D()->SetFontFactor(0.8);
	this->CurrentRenderer->AddActor2D(plotElevation);
}
