#pragma once
#include <vtkSmartPointer.h>

class vtkPoints;
class vtkTransform;

class ICP
{
public:
	ICP() {};
	~ICP() 
	{
		pointsSource = nullptr;
		pointsTarget = nullptr;
	};

	//Inputs
	void setSourcePoints(vtkSmartPointer<vtkPoints> points) { pointsSource = points; };
	void setTargetPoints(vtkSmartPointer<vtkPoints> points) { pointsTarget = points; };

	//Parameters
	void setMaxNumberOfIterations(unsigned int maxNumberOfIterations) { this->maxNumberOfIterations = maxNumberOfIterations; };
	static unsigned int getMaxNumberOfIterations() { return maxNumberOfIterations; };
	void setMaxNumberOfLandmarks(unsigned int maxNumberOfLandmarks) { this->maxNumberOfLandmarks = maxNumberOfLandmarks; };
	static unsigned int getMaxNumberOfLandmarks() { return maxNumberOfLandmarks; };
	void setStartByMatchingCentroids(bool startByMatchingCentroids) { this->startByMatchingCentroids = startByMatchingCentroids; };
	static bool getStartByMatchingCentroids() { return startByMatchingCentroids; };
	void setUseMeanDistance(bool useMeanDistance) { this->useMeanDistance = useMeanDistance; };
	static bool getUseMeanDistance() { return useMeanDistance; };
	void setMaxMeanDistance(double maxMeanDistance) { this->maxMeanDistance = maxMeanDistance; };
	static double getMaxMeanDistance() { return maxMeanDistance; };
	void setClosestPointMaxDistance(double closestPointMaxDistance) { this->closestPointMaxDistance = closestPointMaxDistance; };
	static double getClosestPointMaxDistance() { return closestPointMaxDistance; };

	//Process
	void delimitByMinimumBoudingBox();
	void executeICP();

	//Outputs
	vtkSmartPointer<vtkTransform> getTransform() const { return T; };
	double getMeanDistance() const { return meanDistance; };

private:
	//Parameters
	static unsigned int maxNumberOfIterations;
	static unsigned int maxNumberOfLandmarks;
	static bool startByMatchingCentroids;
	static bool useMeanDistance;
	static double maxMeanDistance;
	static double closestPointMaxDistance;

	double meanDistance = 0.0;
	
	vtkSmartPointer<vtkPoints> pointsSource = nullptr;
	vtkSmartPointer<vtkPoints> pointsTarget = nullptr;
	vtkSmartPointer<vtkTransform> T = nullptr;
};