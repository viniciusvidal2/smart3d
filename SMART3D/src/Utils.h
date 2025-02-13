#pragma once

#include <vtkSmartPointer.h>

class vtkRenderer;
class vtkRenderWindow;
class vtkMatrix4x4;
class vtkAbstractPicker;
class vtkActor;
class vtkTransform;
class vtkPolyData;
class vtkPoints;
class vtkImageData;

class wxImage;

class Camera;

class Utils
{
public:
	Utils();
	~Utils();

	//Update the renderer's active camera to see directly to the focal point
	static void updateCamera(vtkSmartPointer<vtkRenderer> renderer, const Camera* cam);

	static void takeSnapshot(const std::string& path, int magnification, bool getAlpha, vtkRenderWindow* renderWindow);

	//P1---Midpoint---P2
	static void getMidpoint(const double p1[3], const double p2[3], double midPoint[3])
	{
		for (int i = 0; i < 3; i++)
		{
			midPoint[i] = (p1[i] + p2[i]) / 2;
		}
	}
	static void getMidpoint(const double p1[3], const double p2[3], const double p3[3], double midPoint[3])
	{
		for (int i = 0; i < 3; i++)
		{
			midPoint[i] = (p1[i] + p2[i] + p3[i]) / 3;
		}
	}

	static void transformPoint(double point[3], vtkSmartPointer<vtkMatrix4x4> matrixRT);
	//p[0] = x; p[1] = y; p[2] = z;
	static void createDoubleVector(double x, double y, double z, double vector[3])
	{
		if (!vector)
		{
			vector = new double[3];
		}
		vector[0] = x;
		vector[1] = y;
		vector[2] = z;
	}
	static void createDoubleVector(const double xyz[3], double vector[3])
	{
		if (!vector)
		{
			vector = new double[3];
		}
		std::memcpy(vector, xyz, sizeof(double) * 3);
	}


	static void getNormal(const double pointA[3], const double pointB[3], const double pointC[3], double n[3]);
	/*
	A
	*
	*
	*
	B * * * C, compute the normal and return the (normal+pointB) nearest to pointTest, the pointTest is used to define the correct direction of the vector
	*/
	static void getNormal(const double pointA[3], const double pointB[3], const double pointC[3], const double pointTest[3], double n[3]);

	//PointC is in the line defined by A and B?
	static bool isInLine(const double pointA[3], const double pointB[3], const double pointC[3])
	{
		return (abs((pointB[1] - pointA[1]) * pointC[0] - (pointB[0] - pointA[0]) * pointC[1] + pointB[0] * pointA[1] - pointB[1] * pointA[0]) / sqrt(pow(pointB[1] - pointA[1], 2) + pow(pointB[0] - pointA[0], 2))) < 0.01;
	}

	static vtkSmartPointer<vtkImageData> wxImage2ImageData(const wxImage& img);

	static void deletePoint(vtkSmartPointer<vtkPoints> points, vtkIdType id);

	static double distanceBetween2DisplayPoints(const double p0[3], const double p1[3])
	{
		return sqrt(pow(p0[0] - p1[0], 2) + pow(p0[1] - p1[1], 2));
	}

	static bool isSamePoint(const double p0[3], const double p1[3])
	{
		return p0[0] == p1[0] && p0[1] == p1[1] && p0[2] == p1[2];
	}

	static void getSkewSym(const double v[3], vtkSmartPointer<vtkMatrix4x4> result);

	static void multiplyMatrix4x4ByScalar(vtkSmartPointer<vtkMatrix4x4> mat, double scalar, vtkSmartPointer<vtkMatrix4x4> result);

	static void sumMatrix4x4(vtkSmartPointer<vtkMatrix4x4> mat1, vtkSmartPointer<vtkMatrix4x4> mat2, vtkSmartPointer<vtkMatrix4x4> result);

	static void getDisplayPosition(vtkSmartPointer<vtkRenderer> renderer, const double point[3], double displayPosition[3]);

	static bool pickPosition(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkAbstractPicker> picker,
		const double displayPosition[3], double point[3]);

	static vtkSmartPointer<vtkActor> duplicateActor(vtkSmartPointer<vtkActor> actor);

	static double getDistanceBetweenGeographicCoordinate(double lat1, double long1, double alt1, double lat2, double long2, double alt2);

	//This transform is used to align the vector A with the orientation of vector B
	static vtkSmartPointer<vtkTransform> getTransformToAlignVectors(double a[3], double b[3]);

	static void getPlaneCoef(const double p1[3], const double p2[3], const double p3[3], double planeCoef[4])
	{
		double a1 = p2[0] - p1[0];
		double b1 = p2[1] - p1[1];
		double c1 = p2[2] - p1[2];
		double a2 = p3[0] - p1[0];
		double b2 = p3[1] - p1[1];
		double c2 = p3[2] - p1[2];
		double a = b1 * c2 - b2 * c1;
		double b = a2 * c1 - a1 * c2;
		double c = a1 * b2 - b1 * a2;
		double d = (-a * p1[0] - b * p1[1] - c * p1[2]);
		planeCoef[0] = a;
		planeCoef[1] = b;
		planeCoef[2] = c;
		planeCoef[3] = d;
	}

	//Get the shortest distance between a plane and a point
	static double getDistancePlaneToPoint(const double planeCoef[4], const double point[3])
	{
		double dist = 0;
		for (size_t i = 0; i < 3; i++)
		{
			dist += planeCoef[i] * point[i];
		}

		dist += abs(planeCoef[3]);
		double sum = 0;
		for (size_t i = 0; i < 3; i++)
		{
			sum += planeCoef[i] * planeCoef[i];
		}

		dist /= sqrt(sum);

		return dist;
	}

	//Get the PolyData of a specific actor
	static vtkSmartPointer<vtkPolyData> getActorPolyData(vtkSmartPointer<vtkActor> actor);

	static std::wstring s2ws(const std::string& s);

	static int startProcess(const std::string& path_with_command);
	static int startProcess(const std::string& path_with_command, const std::string& workingDirectory);
	//startProcess and ask for admin permission
	static int startProcess(const std::string& path_exec, const std::string& parameters, const std::string& workingDirectory);

	static std::string getExecutionPath();

	static int getNumberOfCamerasSFM(const std::string& filename);

	static int getNumberOfCamerasNVM(const std::string& filename);

	static void removeExtraModelsFromNVM(const std::string& filename);

	static vtkSmartPointer<vtkImageData> loadImage(const std::string& filePath);

	//Compute the intersection point between a line and a sphere, return 1 if success
	static bool intersectLineSphere(const double linePoint0[3], const double linePoint1[3],
		const double sphereCenter[3], double sphereRadius, double intersectionPoint[3]);

	//Convert WGS84 lon,lat,alt data to ECEF data (Earth Centered Earth Fixed)
	static void lla_to_ecef(double lat, double lon, double alt, double output[3]);


	//True if the file exists, false otherwise
	static bool exists(const std::string& name);

	//Adjust the path to be used with external processes
	static std::string preparePath(std::string& path);

	//Get file extesion without the dot
	static std::string getFileExtension(const std::string & filePath);

	static std::string getFileName(const std::string & filePath, bool withExtension = true);

	static std::string getPath(const std::string & filePath, bool returnWithLastSlash = true);

	//Write some T value inside a binary file
	template <class T>
	static void writeBin(std::ofstream &out, T val)
	{
		out.write(reinterpret_cast<char*>(&val), sizeof(T));
	}

	static vtkSmartPointer<vtkPoints> getPointsInsideBB(vtkSmartPointer<vtkPoints> points, const double boudingBox[6]);

};