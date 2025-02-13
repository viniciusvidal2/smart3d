#include "HelperMeshRecon.h"

#include <wx/log.h>

#include "Utils.h"

HelperMeshRecon::HelperMeshRecon()
{
}

HelperMeshRecon::~HelperMeshRecon()
{
}

int HelperMeshRecon::testNumberOfVertexPLY(const std::string& filename)
{
	std::ifstream plyFile(filename);
	std::string line;
	if (plyFile.is_open())
	{
		std::size_t found;
		while (getline(plyFile, line, '\n'))
		{
			found = line.find_last_of(" ");
			if (line.substr(0, found) == "element vertex")
			{
				return std::atoi(line.substr(found + 1).c_str());
			}
		}
		plyFile.close();
	}
	return 0;
}

bool HelperMeshRecon::executeNVM2SFM(std::string nvmPath, std::string sfmPath, const std::string& boundingBoxType)
{
	std::string parameters(Utils::preparePath(Utils::getExecutionPath() + "/meshrecon/nvm2sfm.exe") + " " + Utils::preparePath(nvmPath) + " " + Utils::preparePath(sfmPath) + " " + boundingBoxType);
	if (!Utils::startProcess(parameters))
	{
		wxLogError("Error with NVM2SFM");
		return 0;
	}
	if (!Utils::exists(sfmPath))
	{
		wxLogError("No .sfm was generated");
		return 0;
	}
	return 1;
}

bool HelperMeshRecon::executeMeshRecon(std::string sfmPath, std::string plyPath, const std::string& levelOfDetails)
{
	std::string plyInit = plyPath.substr(0, plyPath.size() - 4) + "_init.ply";
	std::string paramInit(Utils::preparePath(Utils::getExecutionPath() + "/meshrecon/MeshRecon_init.exe") + " "
		+ Utils::preparePath(sfmPath) + " " + Utils::preparePath(plyInit));
	if (!Utils::startProcess(paramInit))
	{
		wxLogError("Error with MeshRecon_init");
		return 0;
	}
	if (!Utils::exists(plyInit))
	{
		wxLogError("No initial mesh was generated");
		return 0;
	}
	if (testNumberOfVertexPLY(plyInit) == 0)
	{
		wxLogError("No mesh was generated with MeshRecon_init");
		return 0;
	}
	std::string plyRefine = plyPath.substr(0, plyPath.size() - 4) + "_refine.ply";
	std::string paramRefine(Utils::preparePath(Utils::getExecutionPath() + "/meshrecon/MeshRecon_refine.exe") + " "
		+ Utils::preparePath(sfmPath) + " " + Utils::preparePath(plyInit) + " " + Utils::preparePath(plyRefine) + " " + levelOfDetails);
	if (!Utils::startProcess(paramRefine))
	{
		wxLogError("Error with MeshRecon_refine");
		return 0;
	}
	if (!Utils::exists(plyRefine))
	{
		wxLogError("No refine mesh was generated, you probably don't have enought VRAM, try to use CMPMVS in the Configurations");
		return 0;
	}
	return 1;
}
