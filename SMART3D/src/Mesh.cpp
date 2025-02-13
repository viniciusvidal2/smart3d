#include "Mesh.h"

#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkFloatArray.h>
#include <vtkCellArray.h>
#include <vtkTriangle.h>
#include <vtkTransformFilter.h>
#include <vtkTransform.h>
#include <vtkMapper.h>
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkRenderer.h>
#include <vtkCellLocator.h>
#include <vtkAbstractPicker.h>
#include <vtkTexture.h>

#include "Volume.h"
#include "GPSData.h"
#include "FastQuadricSimplification.h"
#include "Calibration.h"
#include "MeshIO.h"
#include "Draw.h"
#include "Utils.h"
#include "Camera.h"

Mesh::Mesh(const std::string& filename) : calibration(new Calibration())
{
	if (!Utils::exists(filename))
	{
		return;
	}
	filePath = filename;
	if (!MeshIO::getActorsFromFile(filePath, actors, textureNames))
	{
		actors.clear();
		return;
	}
	if (actors.size() == 0)
	{
		return;
	}
	visible = true;
	//Update internal polyData
	getPolyData();
	//Test to see if there is point color
	if (polyData->GetPointData())
	{
		if (polyData->GetPointData()->GetScalars())
		{
			textureVisibility = true;
			hasColor = true;
		}
	}
	//Test to see if there is texture
	if (actors.front()->GetTexture())
	{
		//Texture names already loaded inside MeshIO::getActorsFromFile
		textureVisibility = true;
		hasTexture = true;
		for (auto actor : actors)
		{
			textures.push_back(actor->GetTexture());
			actor->GetProperty()->SetAmbient(false);
			actor->GetProperty()->SetLighting(false);
		}
	}
	loaded = true;
	//Test to see if it is a point cloud
	if (polyData->GetVerts())
	{
		if (polyData->GetVerts()->GetNumberOfCells() == 1)
		{
			isPointCloud = true;
		}
	}
}

Mesh::Mesh() : calibration(new Calibration())
{
}

Mesh::~Mesh()
{
	actors.clear();
	textureNames.clear();
	textures.clear();
	cameras.clear();
	volumes.clear();
	if (lastPosition)
	{
		delete lastPosition;
		lastPosition = nullptr;
	}
}

void Mesh::destruct(vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree)
{
	destructCameras(renderer, tree);
	destructTextures(renderer, tree);
	destructVolumes(renderer, tree);
	destructVolume(renderer, tree);
	for (auto actor : actors)
	{
		renderer->RemoveActor(actor);
	}
	actors.clear();
	if (listItemMesh)
	{
		tree->DeleteItem(listItemMesh);
		listItemMesh = nullptr;
	}
	polyData = nullptr;
	meshCellLocator = nullptr;
}

void Mesh::destructCameras(vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree)
{
	for (int i = 0; i < cameras.size(); i++)
	{
		cameras.at(i)->destruct(renderer, tree);
		delete cameras.at(i);
	}
	cameras.clear();
	if (listItemMeshCameras)
	{
		tree->DeleteItem(listItemMeshCameras);
		listItemMeshCameras = nullptr;
	}
	if (camerasPath)
	{
		renderer->RemoveActor(camerasPath);
		camerasPath = nullptr;
	}
}

void Mesh::destructTextures(vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree)
{
	textures.clear();
	textureNames.clear();
	if (listItemMeshTexture)
	{
		tree->DeleteItem(listItemMeshTexture);
		listItemMeshTexture = nullptr;
	}
}

void Mesh::destructVolumes(vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree)
{
	for (int i = 0; i < volumes.size(); i++)
	{
		volumes.at(i)->destruct(renderer, tree);
		delete volumes.at(i);
	}
	volumes.clear();
	if (listItemMeshVolumes)
	{
		tree->DeleteItem(listItemMeshVolumes);
		listItemMeshVolumes = nullptr;
	}
}

void Mesh::destructVolume(vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl * tree)
{
	if (volumeActor)
	{
		renderer->RemoveActor(volumeActor);
		volumeActor = nullptr;
	}
	if (listItemMeshVolume)
	{
		tree->DeleteItem(listItemMeshVolume);
		listItemMeshVolume = nullptr;
	}
}

void Mesh::deleteCamera(int cameraIndex, vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree)
{
	cameras.at(cameraIndex)->destruct(renderer, tree);
	delete cameras.at(cameraIndex);
	cameras.erase(cameras.begin() + cameraIndex);
}

void Mesh::deleteVolume(int volumeIndex, vtkSmartPointer<vtkRenderer> renderer, wxTreeListCtrl* tree)
{
	volumes.at(volumeIndex)->destruct(renderer, tree);
	delete volumes.at(volumeIndex);
	volumes.erase(volumes.begin() + volumeIndex);
}

bool Mesh::exportMesh(const std::string& filePath)
{
	return MeshIO::exportMesh(filePath, this);
}

void Mesh::update360ClickPoints(vtkSmartPointer<vtkRenderer> renderer, Camera * newCam)
{
	for (auto cam : cameras)
	{
		renderer->RemoveActor(cam->image360ClickPoint);
		cam->image360ClickPoint = nullptr;
	}
	if (!newCam)
	{
		return;
	}
	double c[3];
	vtkMath::Subtract(newCam->cameraPoints[0], newCam->cameraPoints[5], c);
	double radius = vtkMath::Norm(c);
	for (auto cam : cameras)
	{
		if (cam != newCam || newCam->getDistanceBetweenCameraCenters(cam) < 20)
		{
			double intersectPoint[3];
			if (Utils::intersectLineSphere(newCam->cameraPoints[0], cam->cameraPoints[0], newCam->cameraPoints[0], radius, intersectPoint))
			{
				cam->image360ClickPoint = Draw::createSphere(renderer, intersectPoint, 0.02, 0.5, 0.5, 0.5);
				cam->image360ClickPoint->GetProperty()->SetOpacity(0.5);
				renderer->AddActor(cam->image360ClickPoint);
			}
		}
	}
}

void Mesh::createPickList(vtkSmartPointer<vtkAbstractPicker> picker)
{
	if (picker)
	{
		for (const auto &actor : actors)
		{
			picker->AddPickList(actor);
		}
		for (const auto &volume : volumes)
		{
			picker->AddPickList(volume->actor);
		}
		if (volumeActor)
		{
			picker->AddPickList(volumeActor);
		}
		picker->PickFromListOn();
	}
}

void Mesh::setVolumesVisibility(bool visible)
{
	for (auto volume : volumes)
	{
		volume->setVisibility(visible);
	}
}

bool Mesh::getVolumesVisibility() const
{
	for (auto const &volume : volumes)
	{
		if (volume->getVisibility())
		{
			return true;
		}
	}
	return false;
}

void Mesh::setMeshVisibility(bool visibility)
{
	for (auto actor : actors)
	{
		actor->SetVisibility(visibility);
	}
	visible = visibility;
}

void Mesh::setVolumeVisibility(bool visible)
{
	volumeActor->SetVisibility(visible);
}

bool Mesh::getVolumeVisibility() const
{
	return volumeActor->GetVisibility();
}

void Mesh::setCamerasVisibility(bool visibility)
{
	for (auto camera : cameras)
	{
		camera->setVisibility(visibility);
	}
}

bool Mesh::getCamerasVisibility() const
{
	for (const auto &camera : cameras)
	{
		if (camera->getVisibility())
		{
			return true;
		}
	}
	return false;
}

void Mesh::setTextureVisibility(bool visible)
{
	//PointCloud/Mesh without texture
	if (textures.size() == 0)
	{
		for (auto actor : actors)
		{
			actor->GetMapper()->SetScalarVisibility(visible);
		}
		textureVisibility = visible;
		return;
	}
	//Mesh
	if (textures.size() != actors.size())
	{
		return;
	}
	if (visible)
	{
		for (int i = 0; i < actors.size(); i++)
		{
			actors.at(i)->SetTexture(textures.at(i));
			actors.at(i)->GetProperty()->SetInterpolationToPhong();
			actors.at(i)->GetProperty()->SetLighting(false);
		}
	}
	else
	{
		for (auto actor : actors)
		{
			actor->SetTexture(nullptr);
			actor->GetProperty()->SetInterpolationToFlat();
			actor->GetProperty()->SetAmbient(0);
			actor->GetProperty()->SetLighting(true);
		}
	}
	textureVisibility = visible;
}

void Mesh::createCamerasPath(vtkSmartPointer<vtkRenderer> renderer)
{
	vtkNew<vtkPoints> points;
	for (const auto &camera : cameras)
	{
		points->InsertNextPoint(camera->cameraPoints.at(0));
	}
	camerasPath = Draw::create3DLine(renderer, points);
	camerasPath->SetVisibility(false);
}

void Mesh::setCameraPathVisibility(bool visible)
{
	if (camerasPath)
	{
		camerasPath->SetVisibility(visible);
	}
}

bool Mesh::getCameraPathVisibility() const
{
	if (camerasPath)
	{
		return camerasPath->GetVisibility();
	}
	return false;
}

bool sortByFilename(Camera* i, Camera* j)
{
	return (i->filePath < j->filePath);
}
void Mesh::sortCameras()
{
	std::sort(cameras.begin(), cameras.end(), sortByFilename);
}

vtkSmartPointer<vtkPolyData> Mesh::getPolyData()
{
	if (actors.size() == 0)
	{
		return nullptr;
	}
	if (lastPosition)
	{
		if (sqrt(vtkMath::Distance2BetweenPoints(actors[0]->GetPosition(), lastPosition)) < 1e-5  && polyData)
		{
			return polyData;
		}
		delete lastPosition;
		lastPosition = nullptr;
	}
	if (actors.size() == 1)
	{
		polyData = vtkPolyData::SafeDownCast(actors[0]->GetMapper()->GetInput());
	}
	else//We need to put all the actors in the same PolyData
	{
		vtkNew<vtkAppendPolyData> appendFilter;
		for (const auto &actor : actors)
		{
			appendFilter->AddInputData(vtkPolyData::SafeDownCast(actor->GetMapper()->GetInput()));
		}
		appendFilter->Update();
		polyData = appendFilter->GetOutput();
		if (!polyData)
		{
			return nullptr;
		}
		vtkNew<vtkCleanPolyData> cleaner;
		cleaner->SetInputData(polyData);
		cleaner->Update();
		polyData = cleaner->GetOutput();
	}
	vtkNew<vtkTransformFilter> transformFilter;
	vtkNew<vtkTransform> T;
	vtkSmartPointer<vtkActor> firstActor = actors[0];
	if (firstActor->GetUserMatrix())
	{
		T->SetMatrix(firstActor->GetUserMatrix());
	}
	else if (firstActor->GetMatrix())
	{
		T->SetMatrix(firstActor->GetMatrix());
	}
	else
	{
		Utils::createDoubleVector(firstActor->GetPosition(), lastPosition);
		return polyData;
	}
	transformFilter->SetTransform(T);
	transformFilter->SetInputData(polyData);
	transformFilter->Update();
	polyData = transformFilter->GetPolyDataOutput();
	Utils::createDoubleVector(firstActor->GetPosition(), lastPosition);
	return polyData;
}

vtkSmartPointer<vtkCellLocator> Mesh::getCellLocator()
{
	if (!meshCellLocator)
	{
		meshCellLocator = vtkSmartPointer<vtkCellLocator>::New();
		meshCellLocator->SetDataSet(this->getPolyData());
		meshCellLocator->BuildLocator();
	}
	return meshCellLocator;
}

void Mesh::setPickable(bool pickable)
{
	if (volumeActor)
	{
		volumeActor->SetPickable(pickable);
	}
	for (auto actor : actors)
	{
		actor->SetPickable(pickable);
	}
}

void Mesh::transform(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkTransform> T)
{
	if (camerasPath)
	{
		renderer->RemoveActor(camerasPath);
		camerasPath = nullptr;
	}
	meshCellLocator = nullptr;
	polyData = nullptr;
	vtkNew<vtkTransform> T3;
	for (auto actor : actors)
	{
		vtkSmartPointer<vtkPolyData> polydata = vtkPolyData::SafeDownCast(actor->GetMapper()->GetInput());
		vtkNew<vtkTransformFilter> transformFilter;
		vtkSmartPointer<vtkMatrix4x4> lasT;
		if (actor->GetUserMatrix())
		{
			lasT = actors.at(0)->GetUserMatrix();
		}
		else if (actor->GetMatrix())
		{
			lasT = actor->GetMatrix();
		}
		vtkNew<vtkMatrix4x4> resultT;
		vtkMatrix4x4::Multiply4x4(lasT, T->GetMatrix(), resultT);

		T3->SetMatrix(resultT);

		transformFilter->SetTransform(T3);
		transformFilter->SetInputData(polydata);
		transformFilter->Update();

		actor->GetMapper()->SetInputDataObject(transformFilter->GetOutput());
		actor->GetMapper()->Modified();
	}
	if (volumeActor)
	{
		vtkNew<vtkTransformFilter> transformFilter;
		transformFilter->SetTransform(T3);
		transformFilter->SetInputData(vtkPolyData::SafeDownCast(volumeActor->GetMapper()->GetInputAsDataSet()));
		transformFilter->Update();
		volumeActor->GetMapper()->SetInputConnection(transformFilter->GetOutputPort());
	}
	for (auto volume : volumes)
	{
		volume->transform(T);
	}
	for (auto camera : cameras)
	{
		camera->transform(renderer, T);
	}
}

void Mesh::setCalibration(Calibration * cal)
{
	if (calibration)
	{
		delete calibration;
	}
	calibration = cal;
	//Update the volume's
	for (auto volume : volumes)
	{
		volume->updateCalibration(calibration->getScaleFactor(), calibration->getMeasureUnit());
	}
}

void Mesh::updatePoints(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkPoints> points,
	vtkSmartPointer<vtkUnsignedCharArray> colors, vtkSmartPointer<vtkFloatArray> normals)
{
	if (isPointCloud)
	{
		for (auto actor : actors)
		{
			renderer->RemoveActor(actor);
		}
		actors.clear();
		meshCellLocator = nullptr;
		polyData = nullptr;
		actors.push_back(Draw::createPointCloud(renderer, points, colors, normals));
	}
}

void Mesh::updatePoints(vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkUnsignedCharArray> colors, vtkSmartPointer<vtkFloatArray> normals)
{
	if (isPointCloud)
	{
		meshCellLocator = nullptr;
		polyData = nullptr;
		polyData = Draw::createPointCloud(points, colors, normals);
		actors.at(0)->GetMapper()->SetInputDataObject(polyData);
		actors.at(0)->GetMapper()->Modified();
	}
}

void Mesh::updatePoints(vtkDataObject * data)
{
	if (isPointCloud)
	{
		meshCellLocator = nullptr;
		polyData = nullptr;

		vtkNew<vtkTransformFilter> transformFilter;
		vtkNew<vtkTransform> T;
		if (actors.at(0)->GetUserMatrix())
		{
			T->SetMatrix(actors.at(0)->GetUserMatrix());
		}
		else if (actors.at(0)->GetMatrix())
		{
			T->SetMatrix(actors.at(0)->GetMatrix());
		}
		T->Inverse();
		transformFilter->SetTransform(T);
		transformFilter->SetInputData(data);
		transformFilter->Update();

		actors.at(0)->GetMapper()->SetInputDataObject(transformFilter->GetOutput());
		actors.at(0)->GetMapper()->Modified();
	}
}

void Mesh::updateCells(vtkDataObject * data)
{
	meshCellLocator = nullptr;
	polyData = nullptr;

	vtkNew<vtkTransformFilter> transformFilter;
	vtkNew<vtkTransform> T;
	if (actors.at(0)->GetUserMatrix())
	{
		T->SetMatrix(actors.at(0)->GetUserMatrix());
	}
	else if (actors.at(0)->GetMatrix())
	{
		T->SetMatrix(actors.at(0)->GetMatrix());
	}
	T->Inverse();
	transformFilter->SetTransform(T);
	transformFilter->SetInputData(data);
	transformFilter->Update();

	actors.at(0)->GetMapper()->SetInputDataObject(transformFilter->GetOutput());
	actors.at(0)->GetMapper()->Modified();
}

void Mesh::updateCells(vtkDataObject * data, unsigned int actorIndex)
{
	meshCellLocator = nullptr;
	polyData = nullptr;

	vtkNew<vtkTransformFilter> transformFilter;
	vtkNew<vtkTransform> T;
	if (actors.at(actorIndex)->GetUserMatrix())
	{
		T->SetMatrix(actors.at(actorIndex)->GetUserMatrix());
	}
	else if (actors.at(actorIndex)->GetMatrix())
	{
		T->SetMatrix(actors.at(actorIndex)->GetMatrix());
	}
	T->Inverse();
	transformFilter->SetTransform(T);
	transformFilter->SetInputData(data);
	transformFilter->Update();

	actors.at(actorIndex)->GetMapper()->SetInputDataObject(transformFilter->GetOutput());
	actors.at(actorIndex)->GetMapper()->Modified();
}

void Mesh::updateActors(std::vector<vtkSmartPointer<vtkActor>> newActors)
{
	if (actors.size() != newActors.size())
	{
		return;
	}
	meshCellLocator = nullptr;
	polyData = nullptr;
	for (size_t i = 0; i < actors.size(); i++)
	{
		vtkSmartPointer<vtkPolyData> polydata = vtkPolyData::SafeDownCast(newActors.at(i)->GetMapper()->GetInput());

		vtkNew<vtkTransformFilter> transformFilter;
		vtkNew<vtkTransform> T;
		if (newActors.at(i)->GetUserMatrix())
		{
			T->SetMatrix(newActors.at(i)->GetUserMatrix());
		}
		else if (newActors.at(i)->GetMatrix())
		{
			T->SetMatrix(newActors.at(i)->GetMatrix());
		}
		T->Inverse();
		transformFilter->SetTransform(T);
		transformFilter->SetInputData(polydata);
		transformFilter->Update();

		actors.at(i)->GetMapper()->SetInputDataObject(transformFilter->GetOutput());
		actors.at(i)->GetMapper()->Modified();
	}
}

void Mesh::fastQuadricSimplification(float perc)
{
	for (size_t j = 0; j < actors.size(); j++)
	{
		vtkSmartPointer<vtkPolyData> polyData = vtkPolyData::SafeDownCast(actors.at(j)->GetMapper()->GetInput());
		vtkSmartPointer<vtkUnsignedCharArray> colors = vtkUnsignedCharArray::SafeDownCast(polyData->GetPointData()->GetScalars());
		bool hasColor = false;
		if (colors)
		{
			hasColor = true;
		}
		vtkSmartPointer<vtkFloatArray> normals = vtkFloatArray::SafeDownCast(polyData->GetPointData()->GetNormals());
		bool hasNormals = false;
		if (normals)
		{
			hasNormals = true;
		}
		vtkSmartPointer<vtkFloatArray> tCoords = vtkFloatArray::SafeDownCast(polyData->GetPointData()->GetTCoords());
		bool hastCoords = false;
		if (tCoords)
		{
			hastCoords = true;
		}
		Simplify::FastQuadricSimplification* meshSimplification = new Simplify::FastQuadricSimplification();
		vtkSmartPointer<vtkPoints> points = polyData->GetPoints();
		int numberOfPoints = points->GetNumberOfPoints();
		meshSimplification->vertices.resize(numberOfPoints);
#pragma omp for
		for (int i = 0; i < numberOfPoints; i++)
		{
			Simplify::FastQuadricSimplification::Vertex v;
			double p[3];
			points->GetPoint(i, p);
			v.p.x = p[0];
			v.p.y = p[1];
			v.p.z = p[2];
			if (hasColor)
			{
				double c[3];
				colors->GetTuple(i, c);
				v.color.x = c[0];
				v.color.y = c[1];
				v.color.z = c[2];
			}
			if (hasNormals)
			{
				double n[3];
				normals->GetTuple(i, n);
				v.n.x = n[0];
				v.n.y = n[1];
				v.n.z = n[2];
			}
			if (hastCoords)
			{
				double tCoord[2];
				tCoords->GetTuple(i, tCoord);
				v.tCoord.x = tCoord[0];
				v.tCoord.y = tCoord[1];
				v.tCoord.z = 0;
			}
			meshSimplification->vertices[i] = v;
		}

		vtkNew<vtkIdList> idL;
		polyData->GetPolys()->InitTraversal();
		while (polyData->GetPolys()->GetNextCell(idL))
		{
			Simplify::FastQuadricSimplification::Triangle t;
			bool testValidFace = true;
			for (auto i = 0; i < idL->GetNumberOfIds(); ++i)
			{
				if (idL->GetId(i) > 0)
				{
					t.v[i] = idL->GetId(i);
				}
				else
				{
					testValidFace = false;
				}
			}
			if (testValidFace)
			{
				meshSimplification->triangles.push_back(t);
			}
		}
		meshSimplification->simplify_mesh(perc * polyData->GetNumberOfCells());
		//Get the result
		numberOfPoints = meshSimplification->vertices.size();
		vtkNew<vtkPoints> newPoints;
		newPoints->SetNumberOfPoints(numberOfPoints);
		vtkNew<vtkUnsignedCharArray> newColors;
		if (hasColor)
		{
			newColors->SetNumberOfComponents(3);
			newColors->SetNumberOfTuples(numberOfPoints);
			newColors->SetName("RGB");
		}
		vtkNew<vtkFloatArray> newNormals;
		if (hasNormals)
		{
			newNormals->SetNumberOfComponents(3);
			newNormals->SetNumberOfTuples(numberOfPoints);
			newNormals->SetName("Normals");
		}
		vtkNew<vtkFloatArray> newtCoords;
		if (hastCoords)
		{
			newtCoords->SetNumberOfComponents(2);
			newtCoords->SetNumberOfTuples(numberOfPoints);
			newtCoords->SetName("tCoords");
		}
#pragma omp for
		for (int i = 0; i < numberOfPoints; i++)
		{
			newPoints->InsertPoint(i, meshSimplification->vertices[i].p.x,
				meshSimplification->vertices[i].p.y,
				meshSimplification->vertices[i].p.z);
			if (hasColor)
			{
				newColors->InsertTuple3(i, meshSimplification->vertices[i].color.x,
					meshSimplification->vertices[i].color.y,
					meshSimplification->vertices[i].color.z);
			}
			if (hasNormals)
			{
				newNormals->InsertTuple3(i, meshSimplification->vertices[i].n.x,
					meshSimplification->vertices[i].n.y,
					meshSimplification->vertices[i].n.z);
			}
			if (hastCoords)
			{
				newNormals->InsertTuple2(i, meshSimplification->vertices[i].tCoord.x,
					meshSimplification->vertices[i].tCoord.y);
			}
		}
		vtkNew<vtkPolyData> newMesh;
		newMesh->SetPoints(newPoints);
		if (hasColor)
		{
			newMesh->GetPointData()->SetScalars(newColors);
		}
		if (hasNormals)
		{
			newMesh->GetPointData()->SetNormals(newNormals);
		}
		if (hastCoords)
		{
			newMesh->GetPointData()->SetTCoords(newtCoords);
		}
		vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
		for (auto triFast : meshSimplification->triangles)
		{
			vtkSmartPointer<vtkTriangle> tri = vtkSmartPointer<vtkTriangle>::New();
			for (size_t i = 0; i < 3; i++)
			{
				tri->GetPointIds()->SetId(i, triFast.v[i]);
			}
			triangles->InsertNextCell(tri);
		}
		newMesh->SetPolys(triangles);

		updateCells(newMesh, j);
		delete meshSimplification;
	}
}

bool Mesh::saveSFM(const std::string& filename) const
{
	std::ofstream sfmFile;
	sfmFile.open(filename);
	if (sfmFile.is_open())
	{
		sfmFile << cameras.size() << "\n\n";
		for (const auto &camera : cameras)
		{
			sfmFile << camera->getSFM();
		}
	}
	else
	{
		return 0;
	}
	sfmFile.close();
	return 1;
}

bool Mesh::saveNVM(const std::string& filename) const
{
	std::ofstream nvmFile;
	nvmFile.open(filename);
	if (nvmFile.is_open())
	{
		nvmFile << "NVM_V3\n\n" << cameras.size() << "\n";
		for (const auto &camera : cameras)
		{
			nvmFile << camera->getNVM();
		}
		nvmFile << "\n";
	}
	else
	{
		return 0;
	}
	nvmFile.close();
	return 1;
}
