#pragma once
#include <vtkSmartPointer.h>
#include <vector>

class vtkRenderer;
class vtkActor;
class vtkImageActor;
class vtkActor2D;
class vtkCaptionActor2D;
class vtkAlgorithmOutput;
class captionActor2D;

class vtkDataSet;
class vtkPolyData;
class vtkPoints;
class vtkImageData;
class vtkUnsignedCharArray;
class vtkFloatArray;

class vtkSelection;

class Draw 
{
public:
	Draw();
	~Draw();

	static vtkSmartPointer<vtkActor> create3DLine(vtkSmartPointer<vtkRenderer> renderer,
		double* point1, double* point2);
	static vtkSmartPointer<vtkActor> create3DLine(vtkSmartPointer<vtkRenderer> renderer,
		vtkSmartPointer<vtkPoints> points);
	static vtkSmartPointer<captionActor2D> createNumericIndicator(vtkSmartPointer<vtkRenderer> renderer,
		double* point3DPosition, std::string text, double fontSize, double r, double g, double b);
	static vtkSmartPointer<captionActor2D> createNumericIndicator(double* point3DPosition,
		std::string text, double fontSize, double r, double g, double b);
	static vtkSmartPointer<vtkActor> createSphere(vtkSmartPointer<vtkRenderer> renderer,
		double* pointCenter, double radius, double r, double g, double b);
	static vtkSmartPointer<vtkActor> createSphere(vtkSmartPointer<vtkRenderer> renderer,
		double* pointCenter, double radius, double r, double g, double b, int resolution);
	static vtkSmartPointer<vtkActor2D> create2DLine(vtkSmartPointer<vtkRenderer> renderer,
		double* point1, double* point2, vtkSmartPointer<vtkPolyData> polyData2D, double lineWidth, double r, double g, double b);
	static vtkSmartPointer<vtkActor2D> create2DLine(vtkSmartPointer<vtkRenderer> renderer,
		vtkSmartPointer<vtkPoints> points, double lineWidth, double r, double g, double b);
	static vtkSmartPointer<vtkActor2D> create2DLineAngle(vtkSmartPointer<vtkRenderer> renderer,
		double* point1, double* point2, double* point3, vtkSmartPointer<vtkPolyData> polyData2D, double lineWidth, double r, double g, double b);
	/*
	This method is used to create a 2D rectangle using display coordinates!
	*/
	static vtkSmartPointer<vtkActor2D> create2DRectangle(vtkSmartPointer<vtkRenderer> renderer,
		double* point1, double* point2, double* point3, double* point4, vtkSmartPointer<vtkPolyData> polyData2D, double lineWidth, double r, double g, double b);
	static vtkSmartPointer<vtkCaptionActor2D> createText(vtkSmartPointer<vtkRenderer> renderer,
		double* point3DPosition, std::string text, double fontSize, double r, double g, double b);
	static vtkSmartPointer<vtkActor> createPlane(vtkSmartPointer<vtkRenderer> renderer,
		double* origin, double* point1, double* point2, double scale, double r, double g, double b);
	static vtkSmartPointer<vtkActor> createPlane(vtkSmartPointer<vtkRenderer> renderer,
		double* center, double* normal, double r, double g, double b);
	static vtkSmartPointer<vtkActor> createPolygon(vtkSmartPointer<vtkRenderer> renderer,
		const std::vector<double*>& polygonPoints, double r, double g, double b);
	static vtkSmartPointer<vtkImageActor> createImageActor(vtkSmartPointer<vtkRenderer> renderer,
		vtkSmartPointer<vtkImageData> imgData);
	static vtkSmartPointer<vtkActor> createImage360Actor(vtkSmartPointer<vtkRenderer> renderer,
		vtkSmartPointer<vtkImageData> imgData, double radius);
	static vtkSmartPointer<vtkActor> createPolyData(vtkSmartPointer<vtkRenderer> renderer,
		vtkSmartPointer<vtkPolyData> polyData, double r, double g, double b);
	static vtkSmartPointer<vtkActor> createPolyData(vtkSmartPointer<vtkRenderer> renderer,
		vtkSmartPointer<vtkPolyData> polyData);
	static vtkSmartPointer<vtkActor> createDataSet(vtkSmartPointer<vtkRenderer> renderer,
		vtkSmartPointer<vtkDataSet> dataSet, double r, double g, double b);
	/*
	origin of the camera = p0
	p1--------p2
	|		       |
	|  pCenter |<--- Looking from p0 to pCenter
	|          |
	p4--------p3
	*/
	static vtkSmartPointer<vtkActor> createFrustum(vtkSmartPointer<vtkRenderer> renderer,
		std::vector<double*> cameraPoints);
	static vtkSmartPointer<vtkActor> createPointCloud(vtkSmartPointer<vtkRenderer> renderer,
		vtkSmartPointer<vtkPolyData> meshPolyData, vtkSmartPointer<vtkSelection> selection, double r, double g, double b);
	static vtkSmartPointer<vtkActor> createPointCloud(vtkSmartPointer<vtkRenderer> renderer,
		vtkSmartPointer<vtkPoints> points, double r, double g, double b);
	static vtkSmartPointer<vtkActor> createPointCloud(vtkSmartPointer<vtkPoints> points,
		double r, double g, double b);
	static vtkSmartPointer<vtkActor> createPointCloud(vtkSmartPointer<vtkRenderer> renderer,
		vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkUnsignedCharArray> colors, vtkSmartPointer<vtkFloatArray> normals);

	static vtkSmartPointer<vtkPolyData> createPointCloud(vtkSmartPointer<vtkPoints> points,
		vtkSmartPointer<vtkUnsignedCharArray> colors, vtkSmartPointer<vtkFloatArray> normals);

	static vtkSmartPointer<vtkActor> createBoundingBox(vtkSmartPointer<vtkRenderer> renderer,
		double* boundingBox, double r, double g, double b);
	static vtkSmartPointer<vtkActor> createBoundingBox(vtkSmartPointer<vtkRenderer> renderer,
		vtkSmartPointer<vtkPoints> points, double r, double g, double b);
	static vtkSmartPointer<vtkActor> createBoundingBox(vtkSmartPointer<vtkRenderer> renderer,
		vtkSmartPointer<vtkPolyData> polyData, double r, double g, double b);
};