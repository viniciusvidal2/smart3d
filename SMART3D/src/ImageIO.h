#pragma once

#include <string>
#include <vector>

class Camera;
namespace easyexif
{
	class EXIFInfo;
};

class ImageIO
{
public:
	static bool getImageSize(const std::string& imagePath, int &width, int &height);


	//Test if image paths exist
	static bool getImagePathsExist(std::vector<std::string> &imagePaths, const std::string& newImageDir = "");
	//Load EXIF data from an image
	static bool loadEXIFData(const std::string& imagePath, easyexif::EXIFInfo &EXIFData);

	//Camera
	//Load the camera parameters
	static bool loadCameraParameters(const std::string& filePath, std::vector<Camera*> &cameras);

	static bool getCamerasFileImagePaths(const std::string& camerasFilePath, std::vector<std::string> &imagePaths);

	static bool replaceCamerasFileImageDir(const std::string& camerasFilePath, const std::string& newImgDir);


private:

	//NVM
	//Get the image paths from a NVM file
	static bool getNVMImagePaths(const std::string& nvmPath, std::vector<std::string> &imagePaths);
	//Replace the image dir of the nvm
	static bool replaceNVMImageDir(const std::string& nvmPath, const std::string& newImgDir);

	//SFM
	//Get the image paths from a SFM file
	static bool getSFMImagePaths(const std::string& sfmPath, std::vector<std::string> &imagePaths);
	//Replace the image dir of the sfm
	static bool replaceSFMImageDir(const std::string& sfmPath, const std::string& newImgDir);

};