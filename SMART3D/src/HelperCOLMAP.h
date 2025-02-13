#pragma once
#include <string>

class HelperCOLMAP
{
public:
	HelperCOLMAP() {};
	~HelperCOLMAP() {};

	static bool modelConverter(std::string inputPath, std::string outputPath, std::string outputType = "nvm");

	//Create the file used in the model_aligner
	static bool createRefImagesFile(const std::string& inputNVM, const std::string& outputPath);

	static bool modelAligner(std::string inputPath, std::string outputPath, std::string refImagesPath, double robustAlignmentMaxError);

	static bool imageUndistorter(std::string imagePath, std::string inputPath, std::string outputPath, int maxImageSize);

	static bool patchMatchStereo(std::string workspacePath);

	static bool stereoFusion(std::string workspacePath, std::string outputPath);

	static bool executeSparse(std::string imagesPath, std::string nvmPath);

	static bool executeDense(std::string imagesPath, std::string nvmPath);

private:

};