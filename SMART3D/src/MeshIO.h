#pragma once

#include <vtkSmartPointer.h>
#include <vector>

class vtkPolyData;
class vtkActor;
class Mesh;

class MeshIO
{
public:

	//Get poly data from a file: .ply, .obj
	static bool getPolyDataFromFile(const std::string& filePath, vtkSmartPointer<vtkPolyData> &polyData);
	
	//Get actors from a file: .ply, .obj, .obj + .mtl and .wrl
	static bool getActorsFromFile(const std::string& filePath, std::vector<vtkSmartPointer<vtkActor>> &actors, std::vector<std::string> &textureNames);

	//Export mesh
	static bool exportMesh(const std::string& filePath, Mesh* mesh);

private:

	//Input

	//PLY and OBJ without texture
	static bool getPolyDataFromPLYandOBJ(const std::string& filePath, vtkSmartPointer<vtkPolyData>& polyData);

	//PLY and OBJ without texture
	static bool getActorsFromPLYandOBJ(const std::string& filePath, std::vector<vtkSmartPointer<vtkActor>> &actors);

	//OBJ with texture
	static bool getActorsFromTexturedOBJ(const std::string& filePathOBJ, const std::string& filePathMTL, std::vector<vtkSmartPointer<vtkActor>> &actors);
	static std::string getMTLFilenameFromOBJ(const std::string& filename);
	static std::vector<std::string> getTextureNamesFromMTL(const std::string& filename);

	//WRL
	static bool getActorsFromWRL(const std::string& filePath, std::vector<vtkSmartPointer<vtkActor>> &actors);

	//Output

	//PLY
	static bool savePLY(const std::string& filename, Mesh* mesh);

	//OBJ
	static bool saveOBJ(const std::string& filename, Mesh* mesh);
	//Add the texture names in the MTL file
	static void addTextureNamesToMTL(const std::string& mtlFilename, std::vector<std::string> textureNames);

};