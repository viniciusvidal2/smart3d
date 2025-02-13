#include "ImageIO.h"

#include <sstream>
#include <iostream>

#include <windows.h>
//  Define min max macros required by GDI+ headers.
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#else
#error max macro is already defined
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#else
#error min macro is already defined
#endif

#include <gdiplus.h>

//  Undefine min max macros so they won't collide with <limits> header content.
#undef min
#undef max

#include <vtkRenderer.h>

#include "exif.h"
#include "Utils.h"
#include "Camera.h"


bool ImageIO::getImageSize(const std::string& imagePath, int & width, int & height)
{
	if (!Utils::exists(imagePath))
	{
		return 0;
	}
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	std::wstring wide_string = std::wstring(imagePath.begin(), imagePath.end());
	Gdiplus::Image* img = new Gdiplus::Image(wide_string.c_str());
	if (img)
	{
		height = img->GetHeight();
		width = img->GetWidth();
		delete img;
		Gdiplus::GdiplusShutdown(gdiplusToken);
		return 1;
	}
	delete img;
	Gdiplus::GdiplusShutdown(gdiplusToken);
	return 0;
}

bool ImageIO::getImagePathsExist(std::vector<std::string>& imagePaths, const std::string& newImageDir)
{
	if (imagePaths.size() == 0)
	{
		return 0;
	}
	for (auto imagePath : imagePaths)
	{
		if (newImageDir != "")
		{
			std::string imageName = "";
			if (imagePath.find_last_of('/') != std::string::npos)
			{
				imageName = imagePath.substr(imagePath.find_last_of('/') + 1, imagePath.size());
				imagePath = newImageDir + "/" + imageName;
			}
			else
			{
				imageName = imagePath.substr(imagePath.find_last_of('\\') + 1, imagePath.size());
				imagePath = newImageDir + "\\" + imageName;
			}
		}
		if (!Utils::exists(imagePath))
		{
			return 0;
		}
	}
	return 1;
}

bool ImageIO::loadEXIFData(const std::string& imagePath, easyexif::EXIFInfo & EXIFData)
{
	FILE *fp = fopen(imagePath.c_str(), "rb");
	if (!fp)
	{
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	unsigned long fsize = ftell(fp);
	rewind(fp);
	unsigned char *buf = new unsigned char[fsize];
	if (fread(buf, 1, fsize, fp) != fsize)
	{
		delete[] buf;
		return 0;
	}
	fclose(fp);
	// Parse EXIF
	int code = EXIFData.parseFrom(buf, fsize);
	delete[] buf;
	if (code)
	{
		return 0;
	}
	return 1;
}

bool ImageIO::loadCameraParameters(const std::string& filePath, std::vector<Camera*>& cameras)
{
	std::ifstream parametersFile(filePath);
	if (!parametersFile.good())
	{
		return 0;
	}
	std::string line;
	unsigned int qtdCameras = 0;
	//SFM
	if (Utils::getFileExtension(filePath) == "sfm")
	{
		std::getline(parametersFile, line, '\n');
		qtdCameras = std::stoi(line);
		std::getline(parametersFile, line, '\n');
	}
	else //NVM
	{
		std::getline(parametersFile, line, '\n');
		std::getline(parametersFile, line, '\n');
		std::getline(parametersFile, line, '\n');
		qtdCameras = std::stoi(line);
	}
	cameras.reserve(qtdCameras);
	for (unsigned int i = 0; i < qtdCameras; i++)
	{
		std::getline(parametersFile, line, '\n');
		Camera* cam = new Camera(nullptr, line);
		if (cam->isLoaded())
		{
			cameras.emplace_back(cam);
		}
		else
		{
			for (auto cam : cameras)
			{
				delete cam;
			}
			delete cam;
			return 0;
		}
	}
	parametersFile.close();
	return 1;
}

bool ImageIO::getCamerasFileImagePaths(const std::string& camerasFilePath, std::vector<std::string>& imagePaths)
{
	if (Utils::getFileExtension(camerasFilePath) == "sfm")
	{
		return ImageIO::getSFMImagePaths(camerasFilePath, imagePaths);
	}
	else if (Utils::getFileExtension(camerasFilePath) == "nvm")
	{
		return ImageIO::getNVMImagePaths(camerasFilePath, imagePaths);
	}
	return 0;
}

bool ImageIO::replaceCamerasFileImageDir(const std::string& camerasFilePath, const std::string& newImgDir)
{
	if (Utils::getFileExtension(camerasFilePath) == "sfm")
	{
		return ImageIO::replaceSFMImageDir(camerasFilePath, newImgDir);
	}
	else if (Utils::getFileExtension(camerasFilePath) == "nvm")
	{
		return ImageIO::replaceNVMImageDir(camerasFilePath, newImgDir);
	}
	return 0;
}

bool ImageIO::getNVMImagePaths(const std::string& nvmPath, std::vector<std::string>& imagePaths)
{
	std::ifstream in(nvmPath.c_str());
	if (!in.good())
	{
		return 0;
	}
	//Check NVM file signature
	std::string signature;
	in >> signature;
	if (signature != "NVM_V3")
	{
		return 0;
	}
	//Discard the rest of the line
	std::getline(in, signature);
	//Read number of views
	int qtdCameras = 0;
	in >> qtdCameras;
	if (qtdCameras < 0 || qtdCameras > 10000)
	{
		return 0;
	}
	//Read views
	std::string filePath;
	imagePaths.reserve(qtdCameras);
	for (int i = 0; i < qtdCameras; ++i)
	{
		//get the filePath
		in >> filePath;
		imagePaths.emplace_back(filePath);
		//Used to jump to the next line
		std::getline(in, signature);
	}
	in.close();
	return 1;
}

bool ImageIO::replaceNVMImageDir(const std::string& nvmPath, const std::string& newImgDir)
{
	std::ifstream in(nvmPath.c_str());
	std::stringstream out;
	if (!in.good())
	{
		return 0;
	}
	//NVM_V3 line
	std::string line;
	std::getline(in, line, '\n');
	out << line << "\n\n";
	/* Read number of views. */
	int num_views = 0;
	in >> num_views;
	out << num_views << "\n";
	if (num_views < 0 || num_views > 10000)
	{
		return 0;
	}
	std::string imagePath;
	std::string imgName;
	for (int i = 0; i < num_views; ++i)
	{
		/* Filename*/
		in >> imagePath;
		imgName = Utils::getFileName(imagePath);
		if (imgName != "")
		{
			out << newImgDir << "\\" << imgName << " ";
		}
		else
		{
			out << newImgDir << "\\" << imagePath << " ";
		}
		double temp;
		for (int j = 0; j < 10; j++)
		{
			in >> temp;
			if (j != 9)
			{
				out << temp << " ";
			}
			else
			{
				out << temp;
			}
		}
		//Avoid double space when we finish the cameras
		if (i < num_views - 1)
		{
			out << "\n";
		}
		in.eof();
	}
	while (getline(in, line, '\n'))
	{
		out << line << "\n";
	}
	in.close();
	//Overwrite the file
	std::ofstream outFile(nvmPath.c_str());
	if (!outFile.good())
	{
		return 0;
	}
	outFile << out.rdbuf();
	outFile.close();
	return 1;
}

bool ImageIO::getSFMImagePaths(const std::string& sfmPath, std::vector<std::string>& imagePaths)
{
	std::ifstream in(sfmPath.c_str());
	if (!in.good())
	{
		return 0;
	}
	//Read number of views
	int qtdCameras = 0;
	in >> qtdCameras;
	std::string temp;
	//Discard the rest of the line
	std::getline(in, temp);
	if (qtdCameras < 0 || qtdCameras > 10000)
	{
		return 0;
	}
	//Read views
	std::string filePath;
	imagePaths.reserve(qtdCameras);
	for (int i = 0; i < qtdCameras; ++i)
	{
		//get the filePath
		in >> filePath;
		imagePaths.emplace_back(filePath);
		//Used to jump to the next line
		std::getline(in, temp);
	}
	in.close();
	return 1;
}

bool ImageIO::replaceSFMImageDir(const std::string& sfmPath, const std::string& newImgDir)
{
	std::ifstream in(sfmPath.c_str());
	std::stringstream out;
	if (!in.good())
	{
		return 0;
	}
	// Read number of views.
	int num_views = 0;
	in >> num_views;
	out << num_views << "\n";
	//Empty line
	std::string line;
	std::getline(in, line, '\n');
	out << line << "\n";
	if (num_views < 0 || num_views > 10000)
	{
		return 0;
	}
	std::string imagePath;
	std::string imgName;
	for (int i = 0; i < num_views; ++i)
	{
		/* Filename*/
		in >> imagePath;
		imgName = Utils::getFileName(imagePath);
		if (imgName != "")
		{
			out << newImgDir << "\\" << imgName << " ";
		}
		else
		{
			out << newImgDir << "\\" << imagePath << " ";
		}
		double temp;
		for (int j = 0; j < 16; j++)
		{
			in >> temp;
			if (j != 15)
			{
				out << temp << " ";
			}
			else
			{
				out << temp;
			}
		}
		//Avoid double space when we finish the cameras
		if (i < num_views - 1)
		{
			out << "\n";
		}
		in.eof();
	}
	while (getline(in, line, '\n'))
	{
		out << line << "\n";
	}
	in.close();
	//Overwrite the file
	std::ofstream outFile(sfmPath.c_str());
	if (!outFile.good())
	{
		return 0;
	}
	outFile << out.rdbuf();
	outFile.close();
	return 1;
}
