#include "ICP.h"
//VTK
#include <vtkPoints.h>
#include <vtkTransform.h>
#include <vtkKdTree.h>
#include <vtkCenterOfMass.h>
#include <vtkLandmarkTransform.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
//WX
#include <wx/progdlg.h>
#include <wx/log.h>
//
#include "Utils.h"

//Default parameters
unsigned int ICP::maxNumberOfIterations = 50;
unsigned int ICP::maxNumberOfLandmarks = 50000;
bool ICP::startByMatchingCentroids = false;
bool ICP::useMeanDistance = false;
double ICP::maxMeanDistance = 0.0001;
double ICP::closestPointMaxDistance = 0.01;

void ICP::delimitByMinimumBoudingBox()
{
	double* boundsSource = pointsSource->GetBounds();
	double* boundsTarget = pointsTarget->GetBounds();
	double minimumBounds[6];
	for (int i = 0; i < 6; i++)
	{
		if (boundsSource[i] < boundsTarget[i])
		{
			minimumBounds[i] = boundsSource[i];
		}
		else
		{
			minimumBounds[i] = boundsTarget[i];
		}
	}
	pointsSource = Utils::getPointsInsideBB(pointsSource, minimumBounds);
	pointsTarget = Utils::getPointsInsideBB(pointsTarget, minimumBounds);
}

void ICP::executeICP()
{
	vtkNew<vtkKdTree> kdTree;
	kdTree->BuildLocatorFromPoints(pointsTarget);
	if (!T)
	{
		T = vtkSmartPointer<vtkTransform>::New();
	}
	else
	{
		T->Identity();
	}
	T->PostMultiply();
	int step = 1;
	if (pointsSource->GetNumberOfPoints() > maxNumberOfLandmarks)
	{
		step = pointsSource->GetNumberOfPoints() / maxNumberOfLandmarks;
	}
	vtkIdType numberOfPoints = pointsSource->GetNumberOfPoints() / step;

	vtkSmartPointer<vtkPoints> pointsSourceAux = vtkSmartPointer<vtkPoints>::New();
	pointsSourceAux->SetNumberOfPoints(numberOfPoints);

	if (startByMatchingCentroids)
	{
		double sourceCentroid[3];
		vtkNew<vtkCenterOfMass> centerOfMass;
		vtkNew<vtkPolyData> polyTemp;

		centerOfMass->SetInputData(polyTemp);
		centerOfMass->GetCenter(sourceCentroid);

		double targetCentroid[3];
		polyTemp->SetPoints(pointsTarget);
		centerOfMass->SetInputData(polyTemp);
		centerOfMass->GetCenter(targetCentroid);

		T->Translate(targetCentroid[0] - sourceCentroid[0],
					 targetCentroid[1] - sourceCentroid[1],
					 targetCentroid[2] - sourceCentroid[2]);
		T->Update();
		for (int i = 0, j = 0; i < numberOfPoints; i++, j+=step)
		{
			double outPoint[3];
			T->InternalTransformPoint(pointsSource->GetPoint(j), outPoint);
			pointsSourceAux->SetPoint(i, outPoint);
		}
	}
	else
	{
		for (int i = 0, j = 0; i < numberOfPoints; i++, j += step)
		{
			pointsSourceAux->SetPoint(i, pointsSource->GetPoint(j));
		}
	}

	vtkSmartPointer<vtkPoints> pointsSourceAux_2 = nullptr;
	vtkSmartPointer<vtkPoints> closestPoints = nullptr;
	vtkSmartPointer<vtkPoints> pointsAux = nullptr;
	vtkSmartPointer<vtkPoints> pointsTemp = nullptr;

	vtkNew<vtkLandmarkTransform> landmarkTransform;
	landmarkTransform->SetMode(VTK_LANDMARK_SIMILARITY);
	unsigned int numberOfIterations = 0;
	double closestPointMaxDistanceSqrd = closestPointMaxDistance * closestPointMaxDistance;
	//Progress dialog
	wxProgressDialog* progDialog = new wxProgressDialog("ICP", "Iteration 0/" + std::to_string(this->getMaxNumberOfIterations()),
		this->getMaxNumberOfIterations(), nullptr, wxPD_SMOOTH | wxPD_AUTO_HIDE | wxPD_APP_MODAL);
	do
	{
		pointsSourceAux_2 = vtkSmartPointer<vtkPoints>::New();
		closestPoints = vtkSmartPointer<vtkPoints>::New();
		
		for (int i = 0; i < numberOfPoints; i++)
		{
			double dist;
			vtkIdType id = kdTree->FindClosestPoint(pointsSourceAux->GetPoint(i), dist);
			if (dist < closestPointMaxDistanceSqrd)
			{
				pointsSourceAux_2->InsertNextPoint(pointsSourceAux->GetPoint(i));
				closestPoints->InsertNextPoint(pointsTarget->GetPoint(id));
			}
		}
		numberOfPoints = pointsSourceAux_2->GetNumberOfPoints();
		if (numberOfPoints == 0)
		{
			wxLogError("Maximum closest point distance too restrictive, no points left.");
			break;
		}

		landmarkTransform->SetSourceLandmarks(pointsSourceAux_2);
		landmarkTransform->SetTargetLandmarks(closestPoints);
		landmarkTransform->Update();

		T->Concatenate(landmarkTransform->GetMatrix());

		double totalDist = 0.0;
		pointsAux = vtkSmartPointer<vtkPoints>::New();
		pointsAux->SetNumberOfPoints(numberOfPoints);
#pragma omp for
		for (int i = 0; i < numberOfPoints; i++)
		{
			double pointAux[3], pointAuxT[3];
			pointsSourceAux_2->GetPoint(i, pointAux);
			landmarkTransform->InternalTransformPoint(pointAux, pointAuxT);
			pointsAux->SetPoint(i, pointAuxT);
			if (useMeanDistance)
			{
				double dist = vtkMath::Distance2BetweenPoints(pointAux, pointAuxT);
#pragma omp atomic
				totalDist += dist;
			}
		}

		numberOfIterations++;
		if (useMeanDistance)
		{
			meanDistance = sqrt(totalDist / (double)numberOfPoints);
			if (meanDistance <= maxMeanDistance)
			{
				break;
			}
			progDialog->Update(numberOfIterations, "Iteration " + std::to_string(numberOfIterations) +"/" + 
				std::to_string(this->getMaxNumberOfIterations()) +
				" mean distance: " + std::to_string(meanDistance) + "/" + std::to_string(this->getMaxMeanDistance()));
		}
		else
		{
			progDialog->Update(numberOfIterations, "Iteration " + std::to_string(numberOfIterations) + "/" + 
				std::to_string(this->getMaxNumberOfIterations()));
		}
		if (numberOfIterations >= maxNumberOfIterations)
		{
			break;
		}

		pointsTemp = pointsSourceAux;
		pointsSourceAux = pointsAux;
		pointsAux = pointsTemp;

	} while (true);
	delete progDialog;
}
