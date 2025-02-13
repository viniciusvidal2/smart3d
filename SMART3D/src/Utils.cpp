#include "Utils.h"

#include <vector>
#include <sstream>
#include <Windows.h>

#include <vtkMath.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkTransformFilter.h>
#include <vtkMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkTexture.h>
#include <vtkTransform.h>
#include <vtkImageImport.h>
#include <vtkImageData.h>
#include <vtkWindowToImageFilter.h>
#include <vtkImageReader2.h>
#include <vtkJPEGReader.h>
#include <vtkBMPReader.h>
#include <vtkTIFFReader.h>
#include<vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkInteractorObserver.h>
#include <vtkAbstractPicker.h>
#include <vtkMatrix4x4.h>
#include <vtksys/SystemTools.hxx>

#include <wx/log.h>
#include <wx/image.h>
#include <wx/stdpaths.h>

#include "Camera.h"

Utils::Utils()
{
}

Utils::~Utils()
{
}

void Utils::updateCamera(vtkSmartPointer<vtkRenderer> renderer, const Camera* cam)
{
	if (cam->viewUp)
	{
		//Updating camera
		if (cam->getIs360())
		{
			renderer->GetActiveCamera()->SetViewAngle(50);
		}
		else
		{
			if (cam->width > cam->height)//I want all the image, so get the bigger side
			{
				renderer->GetActiveCamera()->SetViewAngle(vtkMath::DegreesFromRadians(2.0 * atan(cam->width / (2.0*cam->getFocalX()))));
			}
			else
			{
				renderer->GetActiveCamera()->SetViewAngle(vtkMath::DegreesFromRadians(2.0 * atan(cam->height / (2.0*cam->getFocalY()))));
			}
		}
		renderer->GetActiveCamera()->SetPosition(cam->cameraPoints.at(0));
		renderer->GetActiveCamera()->SetFocalPoint(cam->cameraPoints.at(5));
		renderer->GetActiveCamera()->SetViewUp(cam->viewUp);
		//Necessary to make all actors visible
		renderer->ResetCameraClippingRange();
	}
}

void Utils::takeSnapshot(const std::string& path, int magnification, bool getAlpha, vtkRenderWindow* renderWindow)
{
	vtkNew<vtkWindowToImageFilter> windowToImageFilter;
	windowToImageFilter->SetInput(renderWindow);
	windowToImageFilter->SetScale(magnification);
	if (getAlpha)
	{
		windowToImageFilter->SetInputBufferTypeToRGBA();
	}
	else
	{
		windowToImageFilter->SetInputBufferTypeToRGB();
	}
	windowToImageFilter->ReadFrontBufferOff(); // read from the back buffer
	windowToImageFilter->Update();

	vtkNew<vtkPNGWriter> writer;
	writer->SetFileName(path.c_str());
	writer->SetInputConnection(windowToImageFilter->GetOutputPort());
	writer->Write();
	renderWindow->Render();
}

void Utils::transformPoint(double point[3], vtkSmartPointer<vtkMatrix4x4> matrixRT)
{
	double x = (matrixRT->Element[0][0] * point[0] + matrixRT->Element[0][1] * point[1] + matrixRT->Element[0][2] * point[2] + matrixRT->Element[0][3]);
	double y = (matrixRT->Element[1][0] * point[0] + matrixRT->Element[1][1] * point[1] + matrixRT->Element[1][2] * point[2] + matrixRT->Element[1][3]);
	double z = (matrixRT->Element[2][0] * point[0] + matrixRT->Element[2][1] * point[1] + matrixRT->Element[2][2] * point[2] + matrixRT->Element[2][3]);
	point[0] = x; 
	point[1] = y; 
	point[2] = z;
}

void Utils::getNormal(const double pointA[3], const double pointB[3], const double pointC[3], double n[3])
{
	double v1[3];
	double v2[3];
	vtkMath::Subtract(pointB, pointA, v1);
	vtkMath::Subtract(pointB, pointC, v2);
	vtkMath::Normalize(v1);
	vtkMath::Normalize(v2);
	vtkMath::Cross(v1, v2, n);
	vtkMath::Normalize(n);
}

void Utils::getNormal(const double pointA[3], const double pointB[3], const double pointC[3], const double pointTest[3], double n[3])
{
	double v1[3];
	double v2[3];
	vtkMath::Subtract(pointB, pointA, v1);
	vtkMath::Subtract(pointB, pointC, v2);
	vtkMath::Normalize(v1);
	vtkMath::Normalize(v2);
	vtkMath::Cross(v1, v2, n);
	vtkMath::Normalize(n);
	double n2[3] = {n[0], n[1], n[2]};
	vtkMath::MultiplyScalar(n2, -1);
	vtkMath::Add(n, pointB, v1);
	vtkMath::Add(n2, pointB, v2);
	if (vtkMath::Distance2BetweenPoints(v1, pointTest) > vtkMath::Distance2BetweenPoints(v2, pointTest))
	{
		n[0] = n2[0];
		n[1] = n2[1];
		n[2] = n2[2];
	}
}

vtkSmartPointer<vtkImageData> Utils::wxImage2ImageData(const wxImage& img)
{
	vtkNew<vtkImageImport> importer;
	vtkNew<vtkImageData> imageData;
	importer->SetOutput(imageData);
	importer->SetDataSpacing(1, 1, 1);
	importer->SetDataOrigin(0, 0, 0);
	importer->SetWholeExtent(0, img.GetWidth() - 1, 0, img.GetHeight() - 1, 0, 0);
	importer->SetDataExtentToWholeExtent();
	importer->SetDataScalarTypeToUnsignedChar();
	importer->SetNumberOfScalarComponents(3);
	importer->SetImportVoidPointer(img.GetData());
	importer->Update();
	return imageData;
}

void Utils::deletePoint(vtkSmartPointer<vtkPoints> points, vtkIdType id)
{
	vtkNew<vtkPoints> newPoints;
	const unsigned int numberOfPoints = points->GetNumberOfPoints();
	for (unsigned int i = 0; i < numberOfPoints; i++)
	{
		if (i != id)
		{
			double p[3];
			points->GetPoint(i, p);
			newPoints->InsertNextPoint(p);
		}
	}
	points->ShallowCopy(newPoints);
}

void Utils::getSkewSym(const double v[3], vtkSmartPointer<vtkMatrix4x4> result)
{
	result->SetElement(0, 0, 0);
	result->SetElement(0, 1, -v[2]);
	result->SetElement(0, 2, v[1]);
	result->SetElement(0, 3, 0);

	result->SetElement(1, 0, v[2]);
	result->SetElement(1, 1, 0);
	result->SetElement(1, 2, -v[0]);
	result->SetElement(1, 3, 0);

	result->SetElement(2, 0, -v[1]);
	result->SetElement(2, 1, v[0]);
	result->SetElement(2, 2, 0);
	result->SetElement(2, 3, 0);

	result->SetElement(3, 0, 0);
	result->SetElement(3, 1, 0);
	result->SetElement(3, 2, 0);
	result->SetElement(3, 3, 1);
}

void Utils::multiplyMatrix4x4ByScalar(vtkSmartPointer<vtkMatrix4x4> mat, double scalar, vtkSmartPointer<vtkMatrix4x4> result)
{
	for (unsigned int i = 0; i < 4; i++)
	{
		for (unsigned int j = 0; j < 4; j++)
		{
			result->SetElement(i, j, mat->GetElement(i, j)*scalar);
		}
	}
}

void Utils::sumMatrix4x4(vtkSmartPointer<vtkMatrix4x4> mat1, vtkSmartPointer<vtkMatrix4x4> mat2, vtkSmartPointer<vtkMatrix4x4> result)
{
	for (unsigned int i = 0; i < 4; i++)
	{
		for (unsigned int j = 0; j < 4; j++)
		{
			result->SetElement(i, j, mat1->GetElement(i, j) + mat2->GetElement(i, j));
		}
	}
}

void Utils::getDisplayPosition(vtkSmartPointer<vtkRenderer> renderer, const double point[3], double  displayPosition[3])
{
	vtkInteractorObserver::ComputeWorldToDisplay(renderer, point[0], point[1], point[2], displayPosition);
	displayPosition[2] = 0;
}

bool Utils::pickPosition(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkAbstractPicker> picker, const double displayPosition[3], double point[3])
{
	if (picker->Pick(displayPosition[0], displayPosition[1], 0, renderer))
	{
		picker->GetPickPosition(point);
		return true;
	}
	return false;
}

vtkSmartPointer<vtkActor> Utils::duplicateActor(vtkSmartPointer<vtkActor> actor)
{
	vtkSmartPointer<vtkPolyData> polydata = vtkPolyData::SafeDownCast(actor->GetMapper()->GetInput());
	vtkNew<vtkTransformFilter> transformFilter;
	vtkNew<vtkTransform> T;
	T->Identity();
	if (actor->GetUserMatrix())
	{
		T->SetMatrix(actor->GetUserMatrix());
	}
	else if (actor->GetMatrix())
	{
		T->SetMatrix(actor->GetMatrix());
	}
	transformFilter->SetTransform(T);
	transformFilter->SetInputData(polydata);
	transformFilter->Update();
	polydata = transformFilter->GetPolyDataOutput();
	vtkNew<vtkPolyData> polydataCopy;
	polydataCopy->DeepCopy(polydata);

	vtkNew<vtkPolyDataMapper> mapper;
	mapper->ScalarVisibilityOn();
	mapper->SetScalarModeToUsePointData();

	mapper->SetInputData(polydataCopy);

	vtkNew<vtkActor> newActor;
	newActor->SetMapper(mapper);
	if (actor->GetTexture())
	{
		vtkNew<vtkTexture> texture;
		vtkNew<vtkImageData> imgCopy;
		imgCopy->DeepCopy(actor->GetTexture()->GetInput());
		texture->SetInputData(imgCopy);
		newActor->SetTexture(texture);
		newActor->GetProperty()->SetInterpolationToPhong();
		newActor->GetProperty()->SetLighting(false);
	}
	return newActor;
}

double Utils::getDistanceBetweenGeographicCoordinate(double lat1, double long1, double alt1, double lat2, double long2, double alt2)
{
	double R = 6378.137;

	double dLat = vtkMath::RadiansFromDegrees(lat2) - vtkMath::RadiansFromDegrees(lat1);
	double dLon = vtkMath::RadiansFromDegrees(long2) - vtkMath::RadiansFromDegrees(long1);

	double a = sin(dLat / 2.0f) * sin(dLat / 2.0f) + cos(vtkMath::RadiansFromDegrees(lat1)) * cos(vtkMath::RadiansFromDegrees(lat2)) * sin(dLon / 2.0f) * sin(dLon / 2.0f);

	double c = 2 * atan2(sqrt(a), sqrt(1 - a));

	double d_xy = R * c * 1000;

	return sqrt(pow(d_xy, 2) + pow(alt2 - alt1, 2));
}

vtkSmartPointer<vtkTransform> Utils::getTransformToAlignVectors(double a[3], double b[3])
{
	vtkMath::Normalize(a);
	vtkMath::Normalize(b);

	double rot_angle = acos(vtkMath::Dot(a, b));

	double cVector[3];
	vtkMath::Cross(a, b, cVector);

	vtkMath::Normalize(cVector);

	double x = cVector[0];
	double y = cVector[1];
	double z = cVector[2];

	double c = cos(rot_angle);

	double s = sin(rot_angle);

	vtkNew<vtkMatrix4x4> mat;

	mat->SetElement(0, 0, x*x*(1 - c) + c);
	mat->SetElement(0, 1, x*y*(1 - c) - z * s);
	mat->SetElement(0, 2, x*z*(1 - c) + y * s);
	mat->SetElement(0, 3, 0);
	mat->SetElement(1, 0, y*x*(1 - c) + z * s);
	mat->SetElement(1, 1, y*y*(1 - c) + c);
	mat->SetElement(1, 2, y*z*(1 - c) - x * s);
	mat->SetElement(1, 3, 0);
	mat->SetElement(2, 0, x*z*(1 - c) - y * s);
	mat->SetElement(2, 1, y*z*(1 - c) + x * s);
	mat->SetElement(2, 2, z*z*(1 - c) + c);
	mat->SetElement(2, 3, 0);
	mat->SetElement(3, 0, 0);
	mat->SetElement(3, 1, 0);
	mat->SetElement(3, 2, 0);
	mat->SetElement(3, 3, 1);

	vtkNew<vtkTransform> T;
	T->SetMatrix(mat);

	return T;
}

vtkSmartPointer<vtkPolyData> Utils::getActorPolyData(vtkSmartPointer<vtkActor> actor)
{
	if (!actor)
	{
		return nullptr;
	}
	vtkSmartPointer<vtkPolyData> meshPolyData = vtkPolyData::SafeDownCast(actor->GetMapper()->GetInput());
	vtkNew<vtkTransformFilter> transformFilter;
	vtkNew<vtkTransform> T;
	if (actor->GetUserMatrix())
	{
		T->SetMatrix(actor->GetUserMatrix());
	}
	else if (actor->GetMatrix())
	{
		T->SetMatrix(actor->GetMatrix());
	}
	else
	{
		return meshPolyData;
	}
	transformFilter->SetTransform(T);
	transformFilter->SetInputData(meshPolyData);
	transformFilter->Update();
	meshPolyData = transformFilter->GetPolyDataOutput();
	return meshPolyData;
}

std::wstring Utils::s2ws(const std::string & s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

int Utils::startProcess(const std::string& path_with_command)
{
	std::wstring stemp = s2ws(path_with_command);
	LPWSTR path_command = const_cast<LPWSTR>(stemp.c_str());

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process. 
	if (!CreateProcess(NULL,   // No module name (use command line)
		path_command,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		wxLogError("CreateProcess failed (%d).\n", GetLastError());
		return 0;
	}

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return 1;
}

int Utils::startProcess(const std::string& path_with_command, const std::string& workingDirectory)
{
	std::wstring stemp = s2ws(path_with_command);
	LPWSTR path_command = const_cast<LPWSTR>(stemp.c_str());

	std::wstring stemp2 = s2ws(workingDirectory);
	LPCWSTR working_dir = stemp2.c_str();

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process. 
	if (!CreateProcess(NULL,   // No module name (use command line)
		path_command,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		working_dir,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		wxLogError("CreateProcess failed (%d).\n", GetLastError());
		return 0;
	}

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return 1;
}

int Utils::startProcess(const std::string& path_exec, const std::string& parameters, const std::string& workingDirectory)
{
	std::wstring stemp = s2ws(path_exec);
	LPWSTR path_exe = const_cast<LPWSTR>(stemp.c_str());

	std::wstring stemp2 = s2ws(workingDirectory);
	LPCWSTR working_dir = stemp2.c_str();

	std::wstring stemp3 = s2ws(parameters);
	LPCWSTR param = stemp3.c_str();

	SHELLEXECUTEINFO shExInfo = { 0 };
	shExInfo.cbSize = sizeof(shExInfo);
	shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	shExInfo.hwnd = 0;
	shExInfo.lpVerb = _T("runas");                // Operation to perform
	shExInfo.lpFile = path_exe;       // Application to start    
	shExInfo.lpParameters = param;                  // Additional parameters
	shExInfo.lpDirectory = working_dir;
	shExInfo.nShow = SW_SHOW;
	shExInfo.hInstApp = 0;

	if (ShellExecuteEx(&shExInfo))
	{
		WaitForSingleObject(shExInfo.hProcess, INFINITE);
		CloseHandle(shExInfo.hProcess);
	}
	else
	{
		return 0;
	}
	return 1;
}

std::string Utils::getExecutionPath()
{
	return vtksys::SystemTools::GetFilenamePath(wxStandardPaths::Get().GetExecutablePath().ToStdString());
}

int Utils::getNumberOfCamerasSFM(const std::string& filename)
{
	std::ifstream sfmFile(filename);
	std::string line;
	if (sfmFile.is_open())
	{
		std::getline(sfmFile, line, '\n');
		sfmFile.close();
		return std::atoi(line.c_str());
	}
	return 0;
}

int Utils::getNumberOfCamerasNVM(const std::string& filename)
{
	std::ifstream nvmFile(filename);
	std::string line;
	if (nvmFile.is_open())
	{
		getline(nvmFile, line, '\n');
		getline(nvmFile, line, '\n');
		getline(nvmFile, line, '\n');
		nvmFile.close();
		return std::atoi(line.c_str());
	}
	return 0;
}

void Utils::removeExtraModelsFromNVM(const std::string& filename)
{
	std::ifstream nvmFile(filename);
	std::stringstream out;
	std::string line;
	if (nvmFile.is_open())
	{
		//NVM_V3
		for (unsigned int i = 0; i < 3; i++)
		{
			getline(nvmFile, line, '\n');
			out << line << "\n";
		}
		int numberOfCameras = std::atoi(line.c_str());
		for (int i = 0; i < numberOfCameras + 2; i++)
		{
			getline(nvmFile, line, '\n');
			out << line << "\n";
		}
		int numberOfFeatures = std::atoi(line.c_str());
		for (int i = 0; i < numberOfFeatures; i++)
		{
			getline(nvmFile, line, '\n');
			out << line << "\n";
		}
	}
	nvmFile.close();
	//Overwrite the file
	std::ofstream outFile(filename.c_str());
	if (!outFile.good())
	{
		return;
	}
	outFile << out.rdbuf();
	outFile.close();
}

vtkSmartPointer<vtkImageData> Utils::loadImage(const std::string& filePath)
{
	if (!exists(filePath))
	{
		return nullptr;
	}
	std::string extension = filePath.substr(filePath.find_last_of("."));
	vtkSmartPointer<vtkImageReader2> reader;
	if (extension == ".jpg" || extension == ".JPG" || extension == ".jpeg")
	{
		reader = vtkSmartPointer<vtkJPEGReader>::New();
	}
	else if (extension == ".png" || extension == ".PNG")
	{
		reader = vtkSmartPointer<vtkPNGReader>::New();
	}
	else if (extension == ".bmp" || extension == ".BMP")
	{
		reader = vtkSmartPointer<vtkBMPReader>::New();
	}
	else if (extension == ".tiff" || extension == ".TIFF")
	{
		reader = vtkSmartPointer<vtkTIFFReader>::New();
	}
	//else if (extension == ".pnm" || extension == ".PNM")
	//{
	   // reader = vtkSmartPointer<vtkPNMReader>::New();
	//}
	else
	{
		return nullptr;
	}
	reader->SetFileName(filePath.c_str());
	reader->Update();
	if (!reader->GetOutput())
	{
		return nullptr;
	}
	return reader->GetOutput();
}

bool Utils::intersectLineSphere(const double linePoint0[3], const double linePoint1[3],
	const double sphereCenter[3], double sphereRadius, double intersectionPoint[3])
{
	//http://www.codeproject.com/Articles/19799/Simple-Ray-Tracing-in-C-Part-II-Triangles-Intersec

	double cx = sphereCenter[0];
	double cy = sphereCenter[1];
	double cz = sphereCenter[2];

	double px = linePoint0[0];
	double py = linePoint0[1];
	double pz = linePoint0[2];

	double vx = linePoint1[0] - px;
	double vy = linePoint1[1] - py;
	double vz = linePoint1[2] - pz;

	double A = vx * vx + vy * vy + vz * vz;
	double B = 2.0 * (px * vx + py * vy + pz * vz - vx * cx - vy * cy - vz * cz);
	double C = px * px - 2 * px * cx + cx * cx + py * py - 2 * py * cy + cy * cy +
		pz * pz - 2 * pz * cz + cz * cz - sphereRadius * sphereRadius;

	// discriminant
	double D = B * B - 4 * A * C;

	if (D < 0)
	{
		return 0;
	}

	double t1 = (-B - std::sqrt(D)) / (2.0 * A);

	double solution1[3];
	Utils::createDoubleVector(linePoint0[0] * (1 - t1) + t1 * linePoint1[0],
		linePoint0[1] * (1 - t1) + t1 * linePoint1[1],
		linePoint0[2] * (1 - t1) + t1 * linePoint1[2],
		solution1);
	if (D == 0)
	{
		intersectionPoint[0] = solution1[0];
		intersectionPoint[1] = solution1[1];
		intersectionPoint[2] = solution1[2];
		return 1;
	}

	double t2 = (-B + std::sqrt(D)) / (2.0 * A);
	double solution2[3];
	Utils::createDoubleVector(linePoint0[0] * (1 - t2) + t2 * linePoint1[0],
		linePoint0[1] * (1 - t2) + t2 * linePoint1[1],
		linePoint0[2] * (1 - t2) + t2 * linePoint1[2],
		solution2);

	// prefer a solution that is closer to the second sphere (linePoint1)
	if (vtkMath::Distance2BetweenPoints(solution1, linePoint1) < vtkMath::Distance2BetweenPoints(solution2, linePoint1))
	{
		intersectionPoint[0] = solution1[0];
		intersectionPoint[1] = solution1[1];
		intersectionPoint[2] = solution1[2];
		return 1;
	}
	else
	{
		intersectionPoint[0] = solution2[0];
		intersectionPoint[1] = solution2[1];
		intersectionPoint[2] = solution2[2];
		return 1;
	}
}

void Utils::lla_to_ecef(double lat, double lon, double alt, double output[3])
{
	const double clat = std::cos(vtkMath::RadiansFromDegrees(lat));
	const double slat = std::sin(vtkMath::RadiansFromDegrees(lat));
	const double clon = std::cos(vtkMath::RadiansFromDegrees(lon));
	const double slon = std::sin(vtkMath::RadiansFromDegrees(lon));

	const double a2 = std::pow(6378137.0, 2);
	const double b2 = std::pow(6356752.314245, 2);

	const double L = 1.0 / std::sqrt(a2 * std::pow(clat, 2) + b2 * std::pow(slat, 2));
	const double x = (a2 * L + alt) * clat * clon;
	const double y = (a2 * L + alt) * clat * slon;
	const double z = (b2 * L + alt) * slat;

	output[0] = x;
	output[1] = y;
	output[2] = z;
}

bool Utils::exists(const std::string & name)
{
	return wxFileExists(name);
}

std::string Utils::preparePath(std::string& path)
{
	std::replace(path.begin(), path.end(), '\\', '/');
	return "\"" + path + "\"";
}

std::string Utils::getFileExtension(const std::string & filePath)
{
	if (filePath.find_last_of('.') != std::string::npos)
	{
		return filePath.substr(filePath.find_last_of('.') + 1, filePath.size());
	}
	return "";
}

std::string Utils::getFileName(const std::string & filePath, bool withExtension)
{
	std::string fileName = "";
	if (filePath.find_last_of('/') != std::string::npos)
	{
		fileName = filePath.substr(filePath.find_last_of('/') + 1, filePath.size());
	}
	else if (filePath.find_last_of('\\') != std::string::npos)
	{
		fileName = filePath.substr(filePath.find_last_of('\\') + 1, filePath.size());
	}
	if (fileName != "" && !withExtension)
	{
		return fileName.substr(0, fileName.find_last_of('.'));
	}
	return fileName;
}

std::string Utils::getPath(const std::string & filePath, bool returnWithLastSlash)
{
	if (filePath.find_last_of('/') != std::string::npos)
	{
		if (returnWithLastSlash)
		{
			return filePath.substr(0, filePath.find_last_of('/') + 1);
		}
		else
		{
			return filePath.substr(0, filePath.find_last_of('/'));
		}
	}
	else if (filePath.find_last_of('\\') != std::string::npos)
	{
		if (returnWithLastSlash)
		{
			return filePath.substr(0, filePath.find_last_of('\\') + 1);
		}
		else
		{
			return filePath.substr(0, filePath.find_last_of('\\'));
		}
	}
	return "";
}

vtkSmartPointer<vtkPoints> Utils::getPointsInsideBB(vtkSmartPointer<vtkPoints> points, const double boudingBox[6])
{
	int numberOfPoints = points->GetNumberOfPoints();
	std::vector<bool> ids;
	ids.resize(numberOfPoints, false);
#pragma omp for
	for (int i = 0; i < numberOfPoints; i++)
	{
		double x[3];
		points->GetPoint(i, x);
		if (x[0] > boudingBox[0] && x[0] < boudingBox[1] &&
			x[1] > boudingBox[2] && x[1] < boudingBox[3] &&
			x[2] > boudingBox[4] && x[2] < boudingBox[5])
		{
			ids[i] = true;
		}
	}
	vtkNew<vtkPoints> pointsOut;
	for (int i = 0; i < numberOfPoints; i++)
	{
		if (ids[i])
		{
			pointsOut->InsertNextPoint(points->GetPoint(i));
		}
	}
	return pointsOut;
}
