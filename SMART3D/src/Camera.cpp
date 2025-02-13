#include "Camera.h"

#include <sstream>

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkQuaternion.h>
#include <vtkMatrix4x4.h>
#include <vtkTransformFilter.h>
#include <vtkTransform.h>
#include <vtkPolygon.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkTriangle.h>

#include <vtkActor.h>
#include <vtkImageActor.h>
#include <vtkPolyDataMapper.h>

#include "GPSData.h"
#include "exif.h"
#include "ImageIO.h"
#include "Draw.h"
#include "Utils.h"

Camera::Camera()
{
}

Camera::Camera(vtkSmartPointer<vtkRenderer> renderer, const std::string& cameraParameters)
{
	std::vector<std::string> tokens;
	std::istringstream iss(cameraParameters);
	std::string token;
	while (std::getline(iss, token, ' '))
	{
		tokens.push_back(token);
	}
	vtkNew<vtkMatrix4x4> matrixRT;
	//NVM file
	if (tokens.size() == 11)
	{
		this->filePath = tokens[0];
		this->focalX = std::stod(tokens[1]);
		this->focalY = this->focalX;
		vtkQuaternion<double> *quaternion = new vtkQuaternion<double>;
		//Camera rotation and center
		double quat[4];
		for (int j = 0; j < 4; ++j)
			quat[j] = std::stod(tokens[2 + j]);

		quaternion->Set(quat);
		double rotation[3][3];
		quaternion->ToMatrix3x3(rotation);

		double center[3], trans[3];
		for (int j = 0; j < 3; ++j)
			center[j] = std::stod(tokens[6 + j]);

		trans[0] = trans[1] = trans[2] = 0;
		for (int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				trans[j] += rotation[j][k] * (-center[k]);
			}
		}
		//Rotation
		for (int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				matrixRT->Element[j][k] = rotation[j][k];
			}
			//Translation
			matrixRT->Element[j][3] = trans[j];
		}
		//Last line
		matrixRT->Element[3][0] = matrixRT->Element[3][1] = matrixRT->Element[3][2] = 0; matrixRT->Element[3][3] = 1;
	}
	//SFM file
	else if (tokens.size() == 17)
	{
		this->filePath = tokens[0];
		this->focalX = std::stod(tokens[13]);
		this->focalY = std::stod(tokens[14]);
		//Rotation
		for (int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				matrixRT->Element[j][k] = std::stod(tokens[1 + (j * 3) + k]);
			}
			//Translation
			matrixRT->Element[j][3] = std::stod(tokens[10 + j]);
		}
		//Last line
		matrixRT->Element[3][0] = matrixRT->Element[3][1] = matrixRT->Element[3][2] = 0; matrixRT->Element[3][3] = 1;
	}
	else
	{
		return;
	}
	this->setMatrixRt(matrixRT);
	this->updateGPSData();
	ImageIO::getImageSize(this->filePath, this->width, this->height);
	calcCameraPoints();
	if (cameraPoints.size() > 0)
	{
		loaded = true;
	}
	if (renderer)
	{
		createActorFrustrum(renderer);
	}
}

Camera::~Camera()
{
	for (int i = 0; i < cameraPoints.size(); i++)
	{
		delete cameraPoints.at(i);
	}
	cameraPoints.clear();
	if (viewUp)
	{
		delete viewUp;
	}
	if (gpsData)
	{
		delete gpsData;
	}
}

void Camera::destruct(vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl * tree)
{
	if (imageActor)
	{
		renderer->RemoveActor(imageActor);
		imageActor = nullptr;
	}
	if (image360ClickPoint)
	{
		renderer->RemoveActor(image360ClickPoint);
		image360ClickPoint = nullptr;
	}
	if (image360Actor)
	{
		renderer->RemoveActor(image360Actor);
		image360Actor = nullptr;
	}
	if (actorFrustum)
	{
		renderer->RemoveActor(actorFrustum);
		actorFrustum = nullptr;
	}
	if (listItemCamera)
	{
		tree->DeleteItem(listItemCamera);
		listItemCamera = nullptr;
	}
}

void Camera::transform(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkTransform> T)
{
	if (actorFrustum)
	{
		actorFrustum->SetVisibility(true);
		//For some reason we can just transform whats was already rendered  at least once
		renderer->GetRenderWindow()->Render();
		vtkNew<vtkTransformFilter> transformFilter;
		transformFilter->SetTransform(T);
		transformFilter->SetInputData(actorFrustum->GetMapper()->GetInputAsDataSet());
		transformFilter->Update();
		actorFrustum->GetMapper()->SetInputConnection(transformFilter->GetOutputPort());
		actorFrustum->SetVisibility(this->visible);

		vtkNew<vtkMatrix4x4> result;
		vtkMatrix4x4::Multiply4x4(T->GetMatrix(), matrixRtInverted, result);
		matrixRtInverted = result;
		vtkMatrix4x4::Invert(matrixRtInverted, matrixRt);
		calcCameraPoints();
		if (imageActor)
		{
			imageActor->SetUserMatrix(result);
		}
	}
}

void Camera::setVisibility(bool visible)
{
	if (actorFrustum)
	{
		actorFrustum->SetVisibility(visible);
	}
	if (imageActor)
	{
		imageActor->SetVisibility(visible);
	}
	if (image360Actor)
	{
		image360Actor->SetVisibility(visible);
	}
	this->visible = visible;
}

bool Camera::calcCameraPoints()
{
	if (width > 0 && height > 0 && matrixRtInverted)
	{
		for (int i = 0; i < cameraPoints.size(); i++)
		{
			delete cameraPoints.at(i);
		}
		cameraPoints.clear();
		if (viewUp)
		{
			delete viewUp;
			viewUp = nullptr;
		}
		double* p1 = new double[3];
		Utils::createDoubleVector(0, 0, 0, p1);
		Utils::transformPoint(p1, matrixRtInverted);
		double dist = 0.5;
		double minX, minY, maxX, maxY;
		maxX = dist * (width / (2.0*focalX));
		minX = -maxX;
		maxY = dist * (height / (2.0*focalY));
		minY = -maxY;
		/*
			origin of the camera = p1
			p2--------p3
			|		   |
			|  pCenter |<--- Looking from p1 to pCenter
			|          |
			p5--------p4
			*/
		double* p2 = new double[3];
		Utils::createDoubleVector(minX, minY, dist, p2);
		Utils::transformPoint(p2, matrixRtInverted);
		double* p3 = new double[3];
		Utils::createDoubleVector(maxX, minY, dist, p3);
		Utils::transformPoint(p3, matrixRtInverted);
		double* p4 = new double[3];
		Utils::createDoubleVector(maxX, maxY, dist, p4);
		Utils::transformPoint(p4, matrixRtInverted);
		double* p5 = new double[3];
		Utils::createDoubleVector(minX, maxY, dist, p5);
		Utils::transformPoint(p5, matrixRtInverted);
		double* pCenter = new double[3];
		Utils::createDoubleVector(0, 0, dist, pCenter);
		if (!is360)
		{
			//we multiply for .95 because if we use 100% when we zoom in we pass the image
			pCenter[2] = dist * 0.95;
		}
		Utils::transformPoint(pCenter, matrixRtInverted);
		double* pUP = new double[3];
		Utils::createDoubleVector(0, maxY, dist*0.95, pUP);
		Utils::transformPoint(pUP, matrixRtInverted);
		viewUpDirection = 0;
		cameraPoints.reserve(6);
		cameraPoints.emplace_back(p1);
		cameraPoints.emplace_back(p2);
		cameraPoints.emplace_back(p3);
		cameraPoints.emplace_back(p4);
		cameraPoints.emplace_back(p5);
		cameraPoints.emplace_back(pCenter);
		viewUp = new double[3];
		vtkMath::Subtract(pCenter, pUP, viewUp);
		vtkMath::Normalize(viewUp);

		imagePolygon = vtkSmartPointer<vtkPolygon>::New();
		imagePolygon->GetPoints()->InsertNextPoint(p2);
		imagePolygon->GetPoints()->InsertNextPoint(p3);
		imagePolygon->GetPoints()->InsertNextPoint(p4);
		imagePolygon->GetPoints()->InsertNextPoint(p5);

		return 1;
	}
	return 0;
}

int Camera::changeViewUp()
{
	if (is360)
	{
		return 0;
	}
	if (matrixRtInverted)
	{
		double dist = 0.5;
		double pCenter[3] = { 0, 0, dist*.95 };
		Utils::transformPoint(pCenter, matrixRtInverted);

		double pViewUp[3];
		if (viewUpDirection == -1 || viewUpDirection == 3)
		{
			double maxY = dist * (height / (2.0*focalY));
			Utils::createDoubleVector(0, maxY, dist*.95, pViewUp);
			Utils::transformPoint(pViewUp, matrixRtInverted);
			viewUpDirection = 0;
		}
		else if (viewUpDirection == 0)
		{
			double minX = -dist * (width / (2.0*focalX));
			Utils::createDoubleVector(minX, 0, dist*.95, pViewUp);
			Utils::transformPoint(pViewUp, matrixRtInverted);
			viewUpDirection = 1;
		}
		else if (viewUpDirection == 1)
		{
			double minY = -dist * (height / (2.0*focalY));
			Utils::createDoubleVector(0, minY, dist*.95, pViewUp);
			Utils::transformPoint(pViewUp, matrixRtInverted);
			viewUpDirection = 2;
		}
		else if (viewUpDirection == 2)
		{
			double maxX = dist * (width / (2.0*focalX));
			Utils::createDoubleVector(maxX, 0, dist*.95, pViewUp);
			Utils::transformPoint(pViewUp, matrixRtInverted);
			viewUpDirection = 3;
		}
		else
		{
			return 0;
		}
		vtkMath::Subtract(pCenter, pViewUp, viewUp);
		vtkMath::Normalize(viewUp);
		return 1;
	}
	return 0;
}

void Camera::updateGPSData()
{
	easyexif::EXIFInfo result;
	if (ImageIO::loadEXIFData(filePath, result))
	{
		if (result.GeoLocation.Latitude == 0 && result.GeoLocation.Longitude == 0 && result.GeoLocation.Altitude == 0)
		{
			return;
		}
		if (!gpsData)
		{
			gpsData = new GPSData();
		}
		gpsData->setLatitude(result.GeoLocation.Latitude);
		gpsData->setLongitude(result.GeoLocation.Longitude);
		gpsData->setAltitude(result.GeoLocation.Altitude);
	}
}

double Camera::getDistanceBetweenCameraCenters(Camera * c)
{
	return sqrt(vtkMath::Distance2BetweenPoints(this->cameraPoints.at(0), c->cameraPoints.at(0)));
}

void Camera::createActorFrustrum(vtkSmartPointer<vtkRenderer> renderer)
{
	if (actorFrustum)
	{
		renderer->RemoveActor(actorFrustum);
	}
	if (is360)
	{
		return;
	}
	if (cameraPoints.size() == 0)
	{
		if (!calcCameraPoints())
		{
			return;
		}
	}
	actorFrustum = Draw::createFrustum(renderer, cameraPoints);
}

void Camera::createImageActor(vtkSmartPointer<vtkRenderer> renderer)
{
	if (imageActor || image360Actor)
	{
		//Assuming that the camera never change its source
		return;
	}
	if (is360)
	{
		if (viewUp)
		{
			calcCameraPoints();
			double* radiusV = new double[3];
			vtkMath::Subtract(cameraPoints[0], cameraPoints[5], radiusV);
			image360Actor = Draw::createImage360Actor(renderer, Utils::loadImage(filePath), vtkMath::Norm(radiusV));
			image360Actor->SetUserMatrix(matrixRtInverted);
			image360Actor->SetPickable(false);
			delete radiusV;
		}
	}
	else
	{
		imageActor = Draw::createImageActor(renderer, Utils::loadImage(filePath));
		if (imageActor)
		{
			double dist = 0.5;
			double minX, minY, maxX, maxY;
			maxX = dist * (width / (2.0*focalX));
			minX = -maxX;
			maxY = dist * (height / (2.0*focalY));
			minY = -maxY;
			imageActor->SetUserMatrix(matrixRtInverted);
			imageActor->SetScale(2 * (maxX / width), 2 * (maxY / height), 1);
			imageActor->RotateX(180);
			imageActor->SetPosition(minX, maxY, dist);
		}
	}
}

void Camera::updateFocalDistance(vtkSmartPointer<vtkRenderer> renderer, double focalX, double focalY)
{
	this->focalX = focalX;
	this->focalY = focalY;
	if (calcCameraPoints())
	{
		if (is360)
		{
			return;
		}
		else
		{
			createActorFrustrum(renderer);
			if (imageActor)
			{
				double dist = 0.5;
				double minX, minY, maxX, maxY;
				maxX = dist * (width / (2.0*focalX));
				minX = -maxX;
				maxY = dist * (height / (2.0*focalY));
				minY = -maxY;
				imageActor->SetUserMatrix(matrixRtInverted);
				imageActor->SetScale(2 * (maxX / width), 2 * (maxY / height), 1);
				imageActor->SetPosition(minX, maxY, dist);
			}
		}
	}
}

void Camera::updateMatrixRt(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkMatrix4x4> matrixRt)
{
	if (!this->matrixRt)
	{
		this->matrixRt = vtkSmartPointer<vtkMatrix4x4>::New();
		matrixRtInverted = vtkSmartPointer<vtkMatrix4x4>::New();
	}
	this->matrixRt->DeepCopy(matrixRt);
	matrixRt->Invert();
	this->matrixRtInverted->DeepCopy(matrixRt);
	if (calcCameraPoints())
	{
		createActorFrustrum(renderer);
		if (imageActor)
		{
			double dist = 0.5;
			double minX, minY, maxX, maxY;
			maxX = dist * (width / (2.0*focalX));
			minX = -maxX;
			maxY = dist * (height / (2.0*focalY));
			minY = -maxY;
			imageActor->SetUserMatrix(matrixRtInverted);
			imageActor->SetScale(2 * (maxX / width), 2 * (maxY / height), 1);
			imageActor->SetPosition(minX, maxY, dist);
		}
		else if (image360Actor)
		{
			image360Actor->SetUserMatrix(matrixRtInverted);
		}
	}
}

void Camera::setMatrixRt(vtkSmartPointer<vtkMatrix4x4> matrixRt)
{
	if (!this->matrixRt)
	{
		this->matrixRt = vtkSmartPointer<vtkMatrix4x4>::New();
		matrixRtInverted = vtkSmartPointer<vtkMatrix4x4>::New();
	}
	this->matrixRt->DeepCopy(matrixRt);
	matrixRt->Invert();
	this->matrixRtInverted->DeepCopy(matrixRt);
}

vtkSmartPointer<vtkMatrix4x4> Camera::getMatrixRt()
{
	if (matrixRt)
	{
		vtkSmartPointer<vtkMatrix4x4> temp = vtkSmartPointer<vtkMatrix4x4>::New();
		temp->DeepCopy(matrixRt);
		return temp;
	}
	return nullptr;
}

void Camera::getPitchVector(double * pitchVector)
{
	if (matrixRtInverted)
	{
		double maxX, maxY;
		maxX = width / (2.0*focalX);
		maxY = height / (2.0*focalY);
		double pOrigin[3] = { 0, 0, 0 };
		Utils::transformPoint(pOrigin, matrixRtInverted);
		double pRight[3] = { maxX, 0, 0 };
		Utils::transformPoint(pRight, matrixRtInverted);
		vtkMath::Subtract(pOrigin, pRight, pitchVector);
		vtkMath::Normalize(pitchVector);
	}
}

void Camera::getYawVector(double * yawVector)
{
	if (matrixRtInverted)
	{
		double maxX, maxY;
		maxX = width / (2.0*focalX);
		maxY = height / (2.0*focalY);
		double pOrigin[3] = { 0, 0, 0 };
		Utils::transformPoint(pOrigin, matrixRtInverted);
		double pDown[3] = { 0, -maxY, 0 };
		Utils::transformPoint(pDown, matrixRtInverted);
		vtkMath::Subtract(pOrigin, pDown, yawVector);
		vtkMath::Normalize(yawVector);
	}
}

std::string Camera::getSFM() const
{
	std::stringstream ss;
	ss << filePath << " ";
	//Rotation
	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			ss << matrixRt->GetElement(i, j) << " ";
		}
	}
	//Translation
	for (size_t j = 0; j < 3; j++)
	{
		ss << matrixRt->GetElement(j, 3) << " ";
	}
	ss << focalX << " " << focalY << " " << width / 2.0f << " " << height / 2.0f << "\n";
	return ss.str();
}

std::string Camera::getNVM() const
{
	std::stringstream ss;
	ss << filePath << " " << focalX << " ";
	//MatrixR to quaternion
	double matrixR[3][3];
	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			matrixR[i][j] = matrixRt->GetElement(i, j);
		}
	}
	vtkQuaternion<double>* quat = new vtkQuaternion<double>();
	quat->FromMatrix3x3(matrixR);
	ss << quat->GetW() << " " << quat->GetX() << " " << quat->GetY() << " " << quat->GetZ() << " ";
	ss << "0 0 0 0 0" << "\n";
	delete quat;
	return ss.str();
}

bool Camera::isPointOnCameraPlane(double point[3])
{
	double p[3] = { point[0], point[1], point[2] };
	Utils::transformPoint(p, matrixRtInverted);
	p[0] /= p[2];
	p[1] /= p[2];
	return (p[0] >= width || p[1] >= height || p[0] <= 0 || p[1] <= 0);
}

vtkSmartPointer<vtkPolyData> Camera::getClosedFrustum(double dist)
{
	if (!calcCameraPoints())
	{
		return nullptr;
	}
	double minX, minY, maxX, maxY;
	maxX = dist * (width / (2.0*focalX));
	minX = -maxX;
	maxY = dist * (height / (2.0*focalY));
	minY = -maxY;
	double p1[3] = { 0, 0, 0 };
	Utils::transformPoint(p1, matrixRtInverted);
	double p2[3] = {minX, minY, dist}; 
	Utils::transformPoint(p2, matrixRtInverted);
	double p3[3] = {maxX, minY, dist}; 
	Utils::transformPoint(p3, matrixRtInverted);
	double p4[3] = {maxX, maxY, dist}; 
	Utils::transformPoint(p4, matrixRtInverted);
	double p5[3] = {minX, maxY, dist}; 
	Utils::transformPoint(p5, matrixRtInverted);
	/*
	origin of the camera = p1
	p2--------p3
	|		   |
	|          |
	|          |
	p5--------p4
	*/
	vtkNew<vtkPoints> frustumPoints;
	frustumPoints->InsertNextPoint(p1);//0
	frustumPoints->InsertNextPoint(p2);//1
	frustumPoints->InsertNextPoint(p3);//2
	frustumPoints->InsertNextPoint(p4);//3
	frustumPoints->InsertNextPoint(p5);//4

	vtkNew<vtkCellArray> frustumPolys;
	vtkNew<vtkTriangle> tri;
	tri->GetPointIds()->SetId(0, 0);
	tri->GetPointIds()->SetId(1, 1);
	tri->GetPointIds()->SetId(2, 4);
	frustumPolys->InsertNextCell(tri);
	vtkNew<vtkTriangle> tri2;
	tri2->GetPointIds()->SetId(0, 0);
	tri2->GetPointIds()->SetId(1, 2);
	tri2->GetPointIds()->SetId(2, 1);
	frustumPolys->InsertNextCell(tri2);
	vtkNew<vtkTriangle> tri3;
	tri3->GetPointIds()->SetId(0, 0);
	tri3->GetPointIds()->SetId(1, 2);
	tri3->GetPointIds()->SetId(2, 3);
	frustumPolys->InsertNextCell(tri3);
	vtkNew<vtkTriangle> tri4;
	tri4->GetPointIds()->SetId(0, 0);
	tri4->GetPointIds()->SetId(1, 3);
	tri4->GetPointIds()->SetId(2, 4);
	frustumPolys->InsertNextCell(tri4);
	vtkNew<vtkTriangle> tri5;
	tri5->GetPointIds()->SetId(0, 1);
	tri5->GetPointIds()->SetId(1, 2);
	tri5->GetPointIds()->SetId(2, 3);
	frustumPolys->InsertNextCell(tri5);
	vtkNew<vtkTriangle> tri6;
	tri6->GetPointIds()->SetId(0, 1);
	tri6->GetPointIds()->SetId(1, 3);
	tri6->GetPointIds()->SetId(2, 4);
	frustumPolys->InsertNextCell(tri6);

	vtkNew<vtkPolyData> frustumPolyData;
	frustumPolyData->SetPoints(frustumPoints);
	frustumPolyData->SetPolys(frustumPolys);
	return frustumPolyData;
}

void Camera::setIs360(vtkSmartPointer<vtkRenderer> renderer, bool is360)
{
	this->is360 = is360;
	if (actorFrustum)
	{
		renderer->RemoveActor(actorFrustum);
		actorFrustum = nullptr;
	}
}
