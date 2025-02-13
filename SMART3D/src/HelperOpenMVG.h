#pragma once

#include "Utils.h"
#include <wx/log.h>
#include <openMVG/geometry/rigid_transformation3D_srt.hpp>
#include <openMVG/geometry/Similarity3.hpp>
#include <openMVG/geometry/Similarity3_Kernel.hpp>
#include <openMVG/geodesy/geodesy.hpp>
//Do not delete this
using namespace std;


class HelperOpenMVG
{
public:
	HelperOpenMVG();
	~HelperOpenMVG();

	//imagesPath = "../images"; outputDir="../matches"; focalDistance=focal distance in pixels
	static bool imageListing(std::string imagesPath, std::string outputDir, float focalDistance)
	{
		std::string openMVGParameters(Utils::preparePath(Utils::getExecutionPath() + "/OpenMVG/openMVG_main_SfMInit_ImageListing.exe") +
			" --imageDirectory=" + Utils::preparePath(imagesPath) +
			" --outputDirectory=" + Utils::preparePath(outputDir) +
			" --focal=" + std::to_string(focalDistance)
		);
		if (!Utils::startProcess(openMVGParameters))
		{
			wxLogError("Error with OpenMVG image listing");
			return 0;
		}
		return 1;
	}

	//inputPath = sfm.json; outputDir="../matches"; describerPreset ="NORMAL", "HIGH", "ULTRA"
	static bool computeFeatures(std::string inputPath, std::string outputDir, std::string describerPreset = "")
	{
		std::string openMVGParameters(Utils::preparePath(Utils::getExecutionPath() + "/OpenMVG/openMVG_main_computeFeatures.exe") +
			" --input_file=" + Utils::preparePath(inputPath) +
			" --outdir path=" + Utils::preparePath(outputDir)
		);
		if (describerPreset != "")
		{
			openMVGParameters += " --describerPreset=" + describerPreset;
		}
		if (!Utils::startProcess(openMVGParameters))
		{
			wxLogError("Error with OpenMVG compute features");
			return 0;
		}
		return 1;
	}

	//inputPath = sfm.json; outputDir="../matches"
	static bool computeMatches(std::string inputPath, std::string outputDir)
	{
		std::string openMVGParameters(Utils::preparePath(Utils::getExecutionPath() + "/OpenMVG/openMVG_main_computeMatches.exe") +
			" --input_file=" + Utils::preparePath(inputPath) +
			" --outdir path=" + Utils::preparePath(outputDir)
		);
		if (!Utils::startProcess(openMVGParameters))
		{
			wxLogError("Error with OpenMVG compute matches");
			return 0;
		}
		return 1;
	}

	//inputPath = sfm.json; matchDir="../matches"; outputDir="../matches/incrementalSFM"
	static bool incrementalSFM(std::string inputPath, std::string matchDir, std::string outputDir)
	{
		std::string openMVGParameters(Utils::preparePath(Utils::getExecutionPath() + "/OpenMVG/openMVG_main_incrementalSFM.exe") +
			" --input_file=" + Utils::preparePath(inputPath) +
			" --matchdir=" + Utils::preparePath(matchDir) +
			" --outdir=" + Utils::preparePath(outputDir)
		);
		if (!Utils::startProcess(openMVGParameters))
		{
			wxLogError("Error with OpenMVG incremental SFM");
			return 0;
		}
		return 1;
	}

	static bool findGeoRegistration(std::vector<openMVG::Vec3> vec_sfm_center, std::vector<openMVG::Vec3> vec_gps_center, openMVG::Vec3* t, openMVG::Mat3* R, double* S)
	{
		// Convert positions to the appropriate data container
		const openMVG::Mat X_SfM = Eigen::Map<openMVG::Mat>(vec_sfm_center[0].data(), 3, vec_sfm_center.size());
		const openMVG::Mat X_GPS = Eigen::Map<openMVG::Mat>(vec_gps_center[0].data(), 3, vec_gps_center.size());
		if (!openMVG::geometry::FindRTS(X_SfM, X_GPS, S, t, R))
		{
			wxLogError("Failed to compute the registration");
			return 0;
		}
		return 1;
	}


private:

};