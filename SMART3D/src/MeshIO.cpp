#include "MeshIO.h"

#include <Windows.h>

#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkFloatArray.h>

#include <vtkAbstractPolyDataReader.h>
#include <vtkPLYReader.h>
#include <vtkOBJReader.h>
#include <vtkOBJImporter.h>
#include <vtkVRMLImporter.h>
#include <vtkOBJExporter.h>

#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtksys/SystemTools.hxx>

#include <wx/log.h>

#include "Utils.h"
#include "Mesh.h"
#include "OutputErrorWindow.h"

bool MeshIO::getPolyDataFromFile(const std::string & filePath, vtkSmartPointer<vtkPolyData>& polyData)
{
	if (!Utils::exists(filePath))
	{
		return 0;
	}
	std::string extension = Utils::getFileExtension(filePath);
	if (extension == "ply" || extension == "PLY" || 
		extension == "obj" || extension == "OBJ")
	{
		return getPolyDataFromPLYandOBJ(filePath, polyData);
	}
	else
	{
		wxLogError(wxString("File extension " + extension + " not supported"));
	}
	return 0;
}

bool MeshIO::getActorsFromFile(const std::string& filePath, std::vector<vtkSmartPointer<vtkActor>>& actors, std::vector<std::string>& textureNames)
{
	if (!Utils::exists(filePath))
	{
		return 0;
	}
	std::string extension = Utils::getFileExtension(filePath);
	if (extension == "ply" || extension == "PLY")
	{
		return getActorsFromPLYandOBJ(filePath, actors);
	}
	else if (extension == "obj" || extension == "OBJ")
	{
		std::string filenameMTL = getMTLFilenameFromOBJ(filePath);
		if (filenameMTL != "")
		{
			filenameMTL = Utils::getPath(filePath) + filenameMTL;
			if (!Utils::exists(filenameMTL))
			{
				return getActorsFromPLYandOBJ(filePath, actors);
			}
			else
			{
				textureNames = getTextureNamesFromMTL(filenameMTL);
				if (textureNames.size() == 0)
				{
					wxLogError(wxString("No texture inside " + filenameMTL + " loading obj without texture"));
					return getActorsFromPLYandOBJ(filePath, actors);
				}
				return getActorsFromTexturedOBJ(filePath, filenameMTL, actors);
			}
		}
		else if (extension == "wrl" || extension == "WRL")
		{
			return getActorsFromWRL(filePath, actors);
		}
		else
		{
			return getActorsFromPLYandOBJ(filePath, actors);
		}
	}
	else
	{
		wxLogError(wxString("File extension " + extension + " not supported"));
	}
	return 0;
}

bool MeshIO::exportMesh(const std::string& filePath, Mesh * mesh)
{
	if (mesh->getHasTexture())
	{
		return saveOBJ(filePath, mesh);
	}
	else
	{
		return savePLY(filePath, mesh);
	}
}

bool MeshIO::getPolyDataFromPLYandOBJ(const std::string& filePath, vtkSmartPointer<vtkPolyData>& polyData)
{
	vtkSmartPointer<vtkAbstractPolyDataReader> reader;
	if (Utils::getFileExtension(filePath) == "ply")
	{
		reader = vtkSmartPointer<vtkPLYReader>::New();
	}
	else
	{
		reader = vtkSmartPointer<vtkOBJReader>::New();
	}
	reader->SetFileName(filePath.c_str());
	reader->Update();
	if (!OutputErrorWindow::checkOutputErrorWindow())
	{
		return 0;
	}
	polyData = reader->GetOutput();
	if (!polyData)
	{
		return 0;
	}
	return 1;
}

bool MeshIO::getActorsFromPLYandOBJ(const std::string& filePath, std::vector<vtkSmartPointer<vtkActor>>& actors)
{
	vtkSmartPointer<vtkPolyData> polyData;
	if (!getPolyDataFromPLYandOBJ(filePath, polyData))
	{
		return 0;
	}
	vtkNew<vtkPolyDataMapper> mapper;
	if (polyData->GetPolys()->GetNumberOfCells() == 0)//it is a point cloud
	{
		vtkNew<vtkCellArray> vertices;
		vertices->InsertNextCell(polyData->GetPoints()->GetNumberOfPoints());
		unsigned int numberOfPoints = polyData->GetPoints()->GetNumberOfPoints();
		for (vtkIdType i = 0; i < numberOfPoints; i++)
		{
			vertices->InsertCellPoint(i);
		}
		polyData->SetVerts(vertices);
		mapper->ScalarVisibilityOn();
		mapper->SetScalarModeToUsePointData();
	}
	mapper->SetInputData(polyData);
	vtkNew<vtkActor> actor;
	actor->SetMapper(mapper);
	actors.reserve(1);
	actors.emplace_back(actor);
	return 1;
}

bool MeshIO::getActorsFromTexturedOBJ(const std::string& filePathOBJ, const std::string& filePathMTL, std::vector<vtkSmartPointer<vtkActor>>& actors)
{
	//Create the temp render window
	vtkNew<vtkRenderWindow> renderWindow;
	vtkNew<vtkOBJImporter> objImporter;
	objImporter->SetFileName(filePathOBJ.c_str());
	objImporter->SetFileNameMTL(filePathMTL.c_str());
	objImporter->SetTexturePath(Utils::getPath(filePathOBJ, false).c_str());
	objImporter->SetRenderWindow(renderWindow);
	objImporter->Update();
	if (!OutputErrorWindow::checkOutputErrorWindow())
	{
		return 0;
	}
	vtkSmartPointer<vtkRenderer> renderer = renderWindow->GetRenderers()->GetFirstRenderer();
	vtkSmartPointer<vtkActorCollection> actorCollection = renderer->GetActors();
	actorCollection->InitTraversal();
	const unsigned int numberOfItems = actorCollection->GetNumberOfItems();
	actors.reserve(numberOfItems);
	for (vtkIdType i = 0; i < numberOfItems; i++)
	{
		vtkNew<vtkActor> actor;
		actor->ShallowCopy(actorCollection->GetNextActor());
		actors.emplace_back(actor);
	}
	actorCollection->InitTraversal();
	for (vtkIdType i = 0; i < numberOfItems; i++)
	{
		renderer->RemoveActor(actorCollection->GetNextActor());
	}
	return 1;
}

std::string MeshIO::getMTLFilenameFromOBJ(const std::string& filename)
{
	ifstream myfile(filename);
	std::string line;
	std::string mtlFile = "";
	if (myfile.is_open())
	{
		while (getline(myfile, line, '\n'))
		{
			if (line.find("mtllib ", 0) != std::string::npos)
			{
				mtlFile = vtksys::SystemTools::GetFilenameName(line.substr(line.find(' ', 0) + 1));
				break;
			}
			else if (line[0] == 'v')//the vertex list started, there is no mtllib
			{
				break;
			}
		}
	}
	myfile.close();
	return mtlFile;
}

std::vector<std::string> MeshIO::getTextureNamesFromMTL(const std::string& filename)
{
	ifstream mtlFile(filename);
	std::string line;
	std::vector<std::string> textureNames;
	if (mtlFile.is_open())
	{
		while (getline(mtlFile, line, '\n'))
		{
			if (line.find("map_Kd ", 0) != std::string::npos)
			{
				textureNames.push_back(line.substr(line.find(' ', 0) + 1));
			}
		}
	}
	mtlFile.close();
	return textureNames;
}

bool MeshIO::getActorsFromWRL(const std::string& filePath, std::vector<vtkSmartPointer<vtkActor>>& actors)
{
	//Create the temp render window
	vtkNew<vtkRenderWindow> renderWindow;
	vtkNew<vtkVRMLImporter> vrlmImporter;
	vrlmImporter->SetFileName(filePath.c_str());
	vrlmImporter->SetRenderWindow(renderWindow);
	vrlmImporter->Update();
	if (!OutputErrorWindow::checkOutputErrorWindow())
	{
		return 0;
	}
	vtkSmartPointer<vtkRenderer> renderer = renderWindow->GetRenderers()->GetFirstRenderer();
	vtkSmartPointer<vtkActorCollection> actorCollection = renderer->GetActors();
	actorCollection->InitTraversal();
	const unsigned int numberOfItems = actorCollection->GetNumberOfItems();
	actors.reserve(numberOfItems);
	for (vtkIdType i = 0; i < numberOfItems; i++)
	{
		vtkNew<vtkActor> actor;
		actor->ShallowCopy(actorCollection->GetNextActor());
		actors.emplace_back(actor);
	}
	actorCollection->InitTraversal();
	for (vtkIdType i = 0; i < numberOfItems; i++)
	{
		renderer->RemoveActor(actorCollection->GetNextActor());
	}
	return 1;
}

bool MeshIO::savePLY(const std::string& filename, Mesh * mesh)
{
	vtkSmartPointer<vtkPolyData> polyData = mesh->getPolyData();
	if (!polyData)
	{
		return 0;
	}
	bool hasColor = false;
	bool hasNormals = false;
	bool hasFaces = !mesh->getIsPointCloud();
	size_t qtdFaces;
	size_t qtdPoints;
	vtkSmartPointer<vtkPoints> points = polyData->GetPoints();
	if (!points)
	{
		return 0;
	}
	qtdPoints = points->GetNumberOfPoints();
	vtkSmartPointer<vtkUnsignedCharArray> colors = vtkUnsignedCharArray::SafeDownCast(polyData->GetPointData()->GetScalars());
	if (colors)
	{
		hasColor = true;
	}
	vtkSmartPointer<vtkFloatArray> normals = vtkFloatArray::SafeDownCast(polyData->GetPointData()->GetNormals());
	if (normals)
	{
		hasNormals = true;
	}
	qtdFaces = polyData->GetNumberOfCells();
	//Write the ply file
	std::ofstream outputFile(filename, std::ios::binary);
	if (!outputFile.is_open())
	{
		return 0;
	}
	outputFile << "ply" << "\n";
	outputFile << "format binary_little_endian 1.0" << "\n";
	outputFile << "element vertex " << qtdPoints << "\n";
	outputFile << "property float x" << "\n";
	outputFile << "property float y" << "\n";
	outputFile << "property float z" << "\n";
	if (hasColor)
	{
		outputFile << "property uchar red" << "\n";
		outputFile << "property uchar green" << "\n";
		outputFile << "property uchar blue" << "\n";
	}
	if (hasNormals)
	{
		outputFile << "property float nx" << "\n";
		outputFile << "property float ny" << "\n";
		outputFile << "property float nz" << "\n";
	}
	if (hasFaces)
	{
		outputFile << "element face " << qtdFaces << "\n";
		outputFile << "property list uchar int vertex_indices" << "\n";
	}
	outputFile << "end_header" << "\n";
	double* point, *color, *normal;
	if (outputFile.is_open())
	{
		for (size_t i = 0; i < qtdPoints; i++)
		{
			point = points->GetPoint(i);
			Utils::writeBin<float>(outputFile, point[0]);
			Utils::writeBin<float>(outputFile, point[1]);
			Utils::writeBin<float>(outputFile, point[2]);
			if (hasColor)
			{
				color = colors->GetTuple3(i);
				Utils::writeBin<unsigned char>(outputFile, color[0]);
				Utils::writeBin<unsigned char>(outputFile, color[1]);
				Utils::writeBin<unsigned char>(outputFile, color[2]);
			}
			if (hasNormals)
			{
				normal = normals->GetTuple3(i);
				Utils::writeBin<float>(outputFile, normal[0]);
				Utils::writeBin<float>(outputFile, normal[1]);
				Utils::writeBin<float>(outputFile, normal[2]);
			}
		}
		unsigned int sizeIds;
		for (size_t i = 0; i < qtdFaces; i++)
		{
			vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
			polyData->GetCellPoints(i, idList);
			sizeIds = idList->GetNumberOfIds();
			Utils::writeBin<unsigned char>(outputFile, sizeIds);
			for (size_t j = 0; j < sizeIds; j++)
			{
				Utils::writeBin<int>(outputFile, idList->GetId(j));
			}
		}
	}
	outputFile.close();
	return 1;
}

bool MeshIO::saveOBJ(const std::string& filename, Mesh * mesh)
{
	vtkNew<vtkRenderer> rendererAux;
	vtkNew<vtkRenderWindow> renWinAux;
	renWinAux->AddRenderer(rendererAux);
	for (auto actor : mesh->actors)
	{
		rendererAux->AddActor(actor);
	}
	vtkNew<vtkOBJExporter> objExport;
	objExport->SetRenderWindow(renWinAux);
	std::string baseFilename = filename.substr(0, filename.size() - 4);
	objExport->SetFilePrefix(baseFilename.c_str());
	objExport->Write();
	for (auto actor : mesh->actors)
	{
		rendererAux->RemoveActor(actor);
	}
	if (!Utils::exists(baseFilename + ".mtl"))
	{
		wxLogError("No mtl generated");
		return 0;
	}
	addTextureNamesToMTL(baseFilename + ".mtl", mesh->textureNames);
	//Copy the textures to the new location
	std::string originalPath = Utils::getPath(mesh->filePath);
	std::string newPath = Utils::getPath(baseFilename);
	//Only copy the textured if we are in a new path
	if (originalPath != newPath)
	{
		for (auto textureFileName : mesh->textureNames)
		{
			std::wstring stemp = Utils::s2ws(originalPath + textureFileName);
			LPCWSTR original = stemp.c_str();
			std::wstring stemp2 = Utils::s2ws(newPath + textureFileName);
			LPCWSTR newFile = stemp2.c_str();
			if (!CopyFileW(original, newFile, false))
			{
				wxLogError("CopyFile failed (%d)", GetLastError());
				return 0;
			}
		}
	}
	return 1;
}

void MeshIO::addTextureNamesToMTL(const std::string& mtlFilename, std::vector<std::string> textureNames)
{
	ifstream mtlFile(mtlFilename);
	std::string line;
	std::string newFile;
	unsigned int idxTexture = 0;
	if (mtlFile.is_open())
	{
		while (getline(mtlFile, line, '\n'))
		{
			newFile += line + "\n";
			if (line.find("Tr", 0) != std::string::npos)
			{
				if (idxTexture > (textureNames.size() - 1))
				{
					newFile += "map_Kd " + textureNames.back() + "\n";
				}
				else
				{
					newFile += "map_Kd " + textureNames.at(idxTexture) + "\n";
					idxTexture++;
				}
			}
		}
		mtlFile.close();
		ofstream newMtlFile(mtlFilename);
		if (newMtlFile.is_open())
		{
			newMtlFile << newFile;
		}
		newMtlFile.close();
	}
}
