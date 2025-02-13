#include "HelperCOLMAP.h"

#include <vtksys/SystemTools.hxx>

#include <wx/log.h>

#include "ConfigurationDialog.h"
#include "ImageIO.h"
#include "GPSData.h"
#include "Camera.h"
#include "Utils.h"

bool HelperCOLMAP::modelConverter(std::string inputPath, std::string outputPath, std::string outputType)
{
	std::string colmapParameters(Utils::preparePath(Utils::getExecutionPath() + "/COLMAP/COLMAP.bat") +
		" model_converter --input_path=" + Utils::preparePath(inputPath) +
		" --output_path=" + Utils::preparePath(outputPath) +
		" --output_type=" + Utils::preparePath(outputType)
	);
	if (!Utils::startProcess(colmapParameters))
	{
		wxLogError("Error with COLMAP model converter");
		return 0;
	}
	if (!Utils::exists(outputPath))
	{
		wxLogError("No file was generated");
		return 0;
	}
	return 1;
}

bool HelperCOLMAP::createRefImagesFile(const std::string& inputNVM, const std::string& outputPath)
{
	std::vector<Camera*> cameras;
	if (!ImageIO::loadCameraParameters(inputNVM, cameras))
	{
		return 0;
	}
	if (!cameras[0]->gpsData)
	{
		for (auto camera : cameras)
		{
			delete camera;
		}
		cameras.clear();
		return 0;
	}
	std::ofstream refImagesFile(outputPath);
	if (refImagesFile.is_open())
	{
		for (auto camera : cameras)
		{
			if (camera->gpsData)
			{
				refImagesFile << vtksys::SystemTools::GetFilenameName(camera->filePath) << " " <<
					std::fixed << camera->gpsData->getLatitude() << " " <<
					std::fixed << camera->gpsData->getLongitude() << " " <<
					std::fixed << camera->gpsData->getAltitude() << "\n";
			}
			delete camera;
		}
		cameras.clear();
		refImagesFile.close();
	}
	else
	{
		for (auto camera : cameras)
		{
			delete camera;
		}
		cameras.clear();
		return 0;
	}
	return 1;
}

bool HelperCOLMAP::modelAligner(std::string inputPath, std::string outputPath, std::string refImagesPath, double robustAlignmentMaxError)
{
	std::string colmapParameters(Utils::preparePath(Utils::getExecutionPath() + "/COLMAP/COLMAP.bat") +
		" model_aligner --input_path=" + Utils::preparePath(inputPath) +
		" --output_path=" + Utils::preparePath(outputPath) +
		" --ref_images_path=" + Utils::preparePath(refImagesPath) +
		" --robust_alignment 1" +
		" --robust_alignment_max_error=" + std::to_string(robustAlignmentMaxError)
	);
	if (!Utils::startProcess(colmapParameters))
	{
		wxLogError("Error with COLMAP model aligner");
		return 0;
	}
	return 1;
}

bool HelperCOLMAP::imageUndistorter(std::string imagePath, std::string inputPath, std::string outputPath, int maxImageSize)
{
	std::string colmapParameters(Utils::preparePath(Utils::getExecutionPath() + "/COLMAP/COLMAP.bat") +
		" image_undistorter --image_path=" + Utils::preparePath(imagePath) +
		" --input_path=" + Utils::preparePath(inputPath) +
		" --output_path=" + Utils::preparePath(outputPath) +
		" --max_image_size=" + std::to_string(maxImageSize)
	);
	if (!Utils::startProcess(colmapParameters))
	{
		wxLogError("Error with COLMAP image undistorter");
		return 0;
	}
	return 1;
}

bool HelperCOLMAP::patchMatchStereo(std::string workspacePath)
{
	std::string colmapParameters(Utils::preparePath(Utils::getExecutionPath() + "/COLMAP/COLMAP.bat") +
		" patch_match_stereo --workspace_path=" + Utils::preparePath(workspacePath)
	);
	if (!Utils::startProcess(colmapParameters))
	{
		wxLogError("Error with COLMAP patch match stereo");
		return 0;
	}
	return 1;
}

bool HelperCOLMAP::stereoFusion(std::string workspacePath, std::string outputPath)
{
	std::string colmapParameters(Utils::preparePath(Utils::getExecutionPath() + "/COLMAP/COLMAP.bat") +
		" stereo_fusion --workspace_path=" + Utils::preparePath(workspacePath) +
		" --output_path=" + Utils::preparePath(outputPath)
	);
	if (!Utils::startProcess(colmapParameters))
	{
		wxLogError("Error with COLMAP stereo fusion");
		return 0;
	}
	return 1;
}

bool HelperCOLMAP::executeSparse(std::string imagesPath, std::string nvmPath)
{
	std::string colmapParameters(Utils::preparePath(Utils::getExecutionPath() + "/COLMAP/COLMAP.bat") +
		" automatic_reconstructor --image_path=" + Utils::preparePath(imagesPath) +
		" --workspace_path=" + Utils::preparePath(vtksys::SystemTools::GetFilenamePath(nvmPath)) +
		" --quality=" + ConfigurationDialog::getSparseQuality() +
		" --use_gpu=" + ConfigurationDialog::getUseGPU() +
		" --dense=0"
	);
	if (!Utils::startProcess(colmapParameters))
	{
		wxLogError("Error with COLMAP sparse");
		return 0;
	}
	std::string camerasBinPath = vtksys::SystemTools::GetFilenamePath(nvmPath);
	camerasBinPath += "/sparse/0/cameras.bin";
	if (!Utils::exists(camerasBinPath))
	{
		wxLogError("No camera was generated");
		return 0;
	}
	//Convert cameras.bin to .nvm
	if (!modelConverter(vtksys::SystemTools::GetFilenamePath(nvmPath) + "/sparse/0", nvmPath))
	{
		return 0;
	}
	ImageIO::replaceCamerasFileImageDir(nvmPath, imagesPath);
	return 1;
}

bool HelperCOLMAP::executeDense(std::string imagesPath, std::string nvmPath)
{
	std::string colmapParameters(Utils::preparePath(Utils::getExecutionPath() + "/COLMAP/COLMAP.bat") +
		" automatic_reconstructor --image_path=" + Utils::preparePath(imagesPath) +
		" --workspace_path=" + Utils::preparePath(vtksys::SystemTools::GetFilenamePath(nvmPath)) +
		" --quality=" + ConfigurationDialog::getDenseQuality() +
		" --mesher=" + ConfigurationDialog::getMesher() +
		" --use_gpu=" + ConfigurationDialog::getUseGPU()
	);
	if (!Utils::startProcess(colmapParameters))
	{
		wxLogError("Error with COLMAP dense");
		return 0;
	}
	return 1;
}
