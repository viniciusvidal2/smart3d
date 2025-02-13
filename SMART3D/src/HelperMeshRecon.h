#pragma once
#include <string>

class HelperMeshRecon
{
public:
	HelperMeshRecon();
	~HelperMeshRecon();

	static int testNumberOfVertexPLY(const std::string& filename);

	static bool executeNVM2SFM(std::string nvmPath, std::string sfmPath, const std::string& boundingBoxType);

	static bool executeMeshRecon(std::string sfmPath, std::string plyPath, const std::string& levelOfDetails);

private:

};