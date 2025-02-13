#include "Draw.h"

#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkImageActor.h>
#include <vtkActor2D.h>
#include <vtkProperty2D.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkCaptionActor2D.h>
#include <vtkAlgorithmOutput.h>

#include <vtkDataSet.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkImageData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkFloatArray.h>
#include <vtkSelection.h>
#include <vtkLine.h>
#include <vtkPolyLine.h>
#include <vtkPolygon.h>
#include <vtkTexture.h>
#include <vtkImageFlip.h>
#include <vtkExtractSelectedIds.h>
#include <vtkPointData.h>
#include <vtkOutlineFilter.h>

#include <vtkLineSource.h>
#include <vtkSphereSource.h>
#include <vtkPlaneSource.h>

#include <vtkOpenGLPolyDataMapper.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkImageMapper3D.h>

#include "captionActor2D.h"

Draw::Draw()
{
}

Draw::~Draw()
{
}

vtkSmartPointer<vtkActor> Draw::create3DLine(vtkSmartPointer<vtkRenderer> renderer, double * point1, double * point2)
{
	vtkNew<vtkPoints> points;
	points->InsertNextPoint(point1);
	points->InsertNextPoint(point2);
	return create3DLine(renderer, points);
}

vtkSmartPointer<vtkActor> Draw::create3DLine(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkPoints> points)
{
	vtkNew<vtkLineSource> lineSource;
	lineSource->SetPoints(points);
	//Create a mapper and actor
	vtkNew<vtkPolyDataMapper> mapper;
	mapper->SetInputConnection(lineSource->GetOutputPort());
	vtkNew<vtkActor> actor;
	actor->SetMapper(mapper);
	//avoid picking this actor, to make the "set focus" command better
	actor->SetPickable(false);
	renderer->AddActor(actor);
	return actor;
}

vtkSmartPointer<captionActor2D> Draw::createNumericIndicator(vtkSmartPointer<vtkRenderer> renderer, double * point3DPosition, std::string text, double fontSize, double r, double g, double b)
{
	vtkSmartPointer<captionActor2D> actor = createNumericIndicator(point3DPosition, text, fontSize, r, g, b);
	renderer->AddActor(actor);
	return actor;
}

vtkSmartPointer<captionActor2D> Draw::createNumericIndicator(double * point3DPosition, std::string text, double fontSize, double r, double g, double b)
{
	vtkNew<captionActor2D> actor;
	actor->SetCaption(text.c_str());
	actor->SetPosition(50, 50);
	actor->SetAttachmentPoint(point3DPosition);
	actor->BorderOff();
	actor->LeaderOn();
	actor->SetThreeDimensionalLeader(false);
	//Add Sphere
	vtkNew<vtkSphereSource> sphereSource;
	sphereSource->SetRadius(3);
	actor->SetLeaderGlyphConnection(sphereSource->GetOutputPort());
	actor->SetMaximumLeaderGlyphSize(10);
	actor->SetMinimumLeaderGlyphSize(10);

	actor->SetPadding(0);
	actor->GetTextActor()->SetTextScaleModeToNone();
	actor->GetCaptionTextProperty()->SetFontSize(fontSize);
	actor->GetCaptionTextProperty()->SetFontFamilyToArial();
	actor->GetCaptionTextProperty()->SetColor(r, g, b);
	actor->GetCaptionTextProperty()->SetShadow(1);
	return actor;
}

vtkSmartPointer<vtkActor> Draw::createSphere(vtkSmartPointer<vtkRenderer> renderer, double * pointCenter, double radius, double r, double g, double b)
{
	return createSphere(renderer, pointCenter, radius, r, g, b, 8);
}

vtkSmartPointer<vtkActor> Draw::createSphere(vtkSmartPointer<vtkRenderer> renderer, double * pointCenter, double radius, double r, double g, double b, int resolution)
{
	vtkNew<vtkSphereSource> sphereSource;
	sphereSource->SetRadius(radius);
	sphereSource->SetPhiResolution(resolution);
	sphereSource->SetThetaResolution(resolution);
	//Create a mapper and actor
	vtkNew<vtkPolyDataMapper> mapper;
	mapper->SetInputConnection(sphereSource->GetOutputPort());
	vtkNew<vtkActor> actor;
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(r, g, b);
	actor->SetPosition(pointCenter);
	renderer->AddActor(actor);
	return actor;
}

vtkSmartPointer<vtkActor2D> Draw::create2DLine(vtkSmartPointer<vtkRenderer> renderer, double * point1, double * point2, vtkSmartPointer<vtkPolyData> polyData2D, double lineWidth, double r, double g, double b)
{
	vtkNew<vtkPoints> pointsLine2D;
	pointsLine2D->SetNumberOfPoints(2);
	pointsLine2D->Allocate(2);
	pointsLine2D->InsertPoint(0, point1);
	pointsLine2D->InsertPoint(1, point2);
	vtkNew<vtkCellArray> cellsLine2D;
	cellsLine2D->Initialize();
	vtkNew<vtkLine> line2D;
	line2D->GetPointIds()->SetId(0, 0);
	line2D->GetPointIds()->SetId(1, 1);
	cellsLine2D->InsertNextCell(line2D);
	polyData2D->Initialize();
	polyData2D->SetPoints(pointsLine2D);
	polyData2D->SetLines(cellsLine2D);
	polyData2D->Modified();
	vtkNew<vtkCoordinate> coordinateLine2D;
	coordinateLine2D->SetCoordinateSystemToWorld();
	vtkNew<vtkPolyDataMapper2D> mapperLine2D;
	mapperLine2D->SetInputData(polyData2D);
	mapperLine2D->SetTransformCoordinate(coordinateLine2D);
	mapperLine2D->ScalarVisibilityOn();
	mapperLine2D->SetScalarModeToUsePointData();
	mapperLine2D->Update();
	vtkNew<vtkActor2D> actorLine;
	actorLine->SetMapper(mapperLine2D);
	actorLine->GetProperty()->SetLineWidth(lineWidth);
	actorLine->GetProperty()->SetColor(r, g, b);
	renderer->AddActor2D(actorLine);
	return actorLine;
}

vtkSmartPointer<vtkActor2D> Draw::create2DLine(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkPoints> points, double lineWidth, double r, double g, double b)
{
	vtkNew<vtkPolyLine> polyLine;
	polyLine->GetPointIds()->SetNumberOfIds(points->GetNumberOfPoints());
	for (unsigned int i = 0; i < points->GetNumberOfPoints(); i++)
	{
		polyLine->GetPointIds()->SetId(i, i);
	}
	// Create a cell array to store the lines in and add the lines to it
	vtkNew<vtkCellArray> cells;
	cells->InsertNextCell(polyLine);

	// Create a polydata to store everything in
	vtkNew<vtkPolyData> polyData2D;
	polyData2D->Initialize();
	polyData2D->SetPoints(points);
	polyData2D->SetLines(cells);
	polyData2D->Modified();
	vtkNew<vtkCoordinate> coordinateLine2D;
	coordinateLine2D->SetCoordinateSystemToWorld();
	vtkNew<vtkPolyDataMapper2D> mapperLine2D;
	mapperLine2D->SetInputData(polyData2D);
	mapperLine2D->SetTransformCoordinate(coordinateLine2D);
	mapperLine2D->ScalarVisibilityOn();
	mapperLine2D->SetScalarModeToUsePointData();
	mapperLine2D->Update();
	vtkNew<vtkActor2D> actorLine;
	actorLine->SetMapper(mapperLine2D);
	actorLine->GetProperty()->SetLineWidth(lineWidth);
	actorLine->GetProperty()->SetColor(r, g, b);
	renderer->AddActor2D(actorLine);
	return actorLine;
}

vtkSmartPointer<vtkActor2D> Draw::create2DLineAngle(vtkSmartPointer<vtkRenderer> renderer, double * point1, double * point2, double * point3, vtkSmartPointer<vtkPolyData> polyData2D, double lineWidth, double r, double g, double b)
{
	vtkNew<vtkPoints> pointsLine2D;
	pointsLine2D->SetNumberOfPoints(3);
	pointsLine2D->Allocate(3);
	pointsLine2D->InsertPoint(0, point1);
	pointsLine2D->InsertPoint(1, point2);
	pointsLine2D->InsertPoint(2, point3);
	vtkNew<vtkCellArray> cellsLine2D;
	cellsLine2D->Initialize();
	vtkNew<vtkLine> line2D;
	line2D->GetPointIds()->SetId(0, 0);
	line2D->GetPointIds()->SetId(1, 1);
	vtkNew<vtkLine> line2D_2;
	line2D_2->GetPointIds()->SetId(0, 1);
	line2D_2->GetPointIds()->SetId(1, 2);
	cellsLine2D->InsertNextCell(line2D);
	cellsLine2D->InsertNextCell(line2D_2);
	polyData2D->Initialize();
	polyData2D->SetPoints(pointsLine2D);
	polyData2D->SetLines(cellsLine2D);
	polyData2D->Modified();
	vtkNew<vtkCoordinate> coordinateLine2D;
	coordinateLine2D->SetCoordinateSystemToWorld();
	vtkNew<vtkPolyDataMapper2D> mapperLine2D;
	mapperLine2D->SetInputData(polyData2D);
	mapperLine2D->SetTransformCoordinate(coordinateLine2D);
	mapperLine2D->ScalarVisibilityOn();
	mapperLine2D->SetScalarModeToUsePointData();
	mapperLine2D->Update();
	vtkNew<vtkActor2D> actorLine;
	actorLine->SetMapper(mapperLine2D);
	actorLine->GetProperty()->SetLineWidth(lineWidth);
	actorLine->GetProperty()->SetColor(r, g, b);
	renderer->AddActor2D(actorLine);
	return actorLine;
}

vtkSmartPointer<vtkActor2D> Draw::create2DRectangle(vtkSmartPointer<vtkRenderer> renderer, double * point1, double * point2, double * point3, double * point4, vtkSmartPointer<vtkPolyData> polyData2D, double lineWidth, double r, double g, double b)
{
	vtkNew<vtkPoints> pointsLine2D;
	pointsLine2D->SetNumberOfPoints(3);
	pointsLine2D->Allocate(3);
	pointsLine2D->InsertPoint(0, point1);
	pointsLine2D->InsertPoint(1, point2);
	pointsLine2D->InsertPoint(2, point3);
	pointsLine2D->InsertPoint(3, point4);
	vtkNew<vtkCellArray> cellsLine2D;
	cellsLine2D->Initialize();
	vtkNew<vtkLine> line2D_0;
	line2D_0->GetPointIds()->SetId(0, 0);
	line2D_0->GetPointIds()->SetId(1, 1);
	vtkNew<vtkLine> line2D_1;
	line2D_1->GetPointIds()->SetId(0, 1);
	line2D_1->GetPointIds()->SetId(1, 2);
	vtkNew<vtkLine> line2D_2;
	line2D_2->GetPointIds()->SetId(0, 2);
	line2D_2->GetPointIds()->SetId(1, 3);
	vtkNew<vtkLine> line2D_3;
	line2D_3->GetPointIds()->SetId(0, 3);
	line2D_3->GetPointIds()->SetId(1, 0);
	cellsLine2D->InsertNextCell(line2D_0);
	cellsLine2D->InsertNextCell(line2D_1);
	cellsLine2D->InsertNextCell(line2D_2);
	cellsLine2D->InsertNextCell(line2D_3);
	polyData2D->Initialize();
	polyData2D->SetPoints(pointsLine2D);
	polyData2D->SetLines(cellsLine2D);
	polyData2D->Modified();
	vtkNew<vtkCoordinate> coordinateLine2D;
	coordinateLine2D->SetCoordinateSystemToDisplay();
	vtkNew<vtkPolyDataMapper2D> mapperLine2D;
	mapperLine2D->SetInputData(polyData2D);
	mapperLine2D->SetTransformCoordinate(coordinateLine2D);
	mapperLine2D->ScalarVisibilityOn();
	mapperLine2D->SetScalarModeToUsePointData();
	mapperLine2D->Update();
	vtkNew<vtkActor2D> actorLine;
	actorLine->SetMapper(mapperLine2D);
	actorLine->GetProperty()->SetLineWidth(lineWidth);
	actorLine->GetProperty()->SetColor(r, g, b);
	//try to avoid some erros
	actorLine->SetPickable(false);
	renderer->AddActor2D(actorLine);
	return actorLine;
}

vtkSmartPointer<vtkCaptionActor2D> Draw::createText(vtkSmartPointer<vtkRenderer> renderer, double * point3DPosition, std::string text, double fontSize, double r, double g, double b)
{
	vtkNew<vtkCaptionActor2D> actor;
	actor->SetCaption(text.c_str());
	actor->SetAttachmentPoint(point3DPosition);
	actor->BorderOff();
	actor->LeaderOff();
	actor->SetPadding(0);
	actor->GetTextActor()->SetTextScaleModeToNone();
	actor->GetCaptionTextProperty()->SetFontSize(fontSize);
	actor->GetCaptionTextProperty()->SetFontFamilyToArial();
	actor->GetCaptionTextProperty()->SetColor(r, g, b);
	actor->GetCaptionTextProperty()->SetShadow(1);
	renderer->AddActor(actor);
	return actor;
}

vtkSmartPointer<vtkActor> Draw::createPlane(vtkSmartPointer<vtkRenderer> renderer, double * origin, double * point1, double * point2, double scale, double r, double g, double b)
{
	if (scale <= 0)
	{
		scale = 1;
	}
	double* center = new double[3];
	for (int i = 0; i < 3; i++)
	{
		center[i] = (origin[i] + point1[i] + point2[i]) / 3;
	}
	double* vetor1 = new double[3];
	vtkMath::Subtract(point1, center, vetor1);
	vtkMath::Normalize(vetor1);
	vtkMath::MultiplyScalar(vetor1, scale);
	vtkMath::Add(center, vetor1, vetor1);
	double* vetor2 = new double[3];
	vtkMath::Subtract(origin, center, vetor2);
	vtkMath::Normalize(vetor2);
	vtkMath::MultiplyScalar(vetor2, scale);
	vtkMath::Add(center, vetor2, vetor2);
	double* vetor3 = new double[3];
	vtkMath::Subtract(point2, center, vetor3);
	vtkMath::Normalize(vetor3);
	vtkMath::MultiplyScalar(vetor3, scale);
	vtkMath::Add(center, vetor3, vetor3);

	vtkNew<vtkPlaneSource> planeSource;
	planeSource->SetOrigin(vetor2);
	planeSource->SetPoint1(vetor1);
	planeSource->SetPoint2(vetor3);
	//Create a mapper and actor
	vtkNew<vtkPolyDataMapper> mapper;
	mapper->SetInputConnection(planeSource->GetOutputPort());
	vtkNew<vtkActor> actorPlane;
	actorPlane->SetMapper(mapper);
	actorPlane->GetProperty()->SetColor(r, g, b);
	renderer->AddActor(actorPlane);
	return actorPlane;
}

vtkSmartPointer<vtkActor> Draw::createPlane(vtkSmartPointer<vtkRenderer> renderer, double * center, double * normal, double r, double g, double b)
{
	vtkNew<vtkPlaneSource> planeSource;
	planeSource->SetCenter(center);
	planeSource->SetNormal(normal);
	//Create a mapper and actor
	vtkNew<vtkPolyDataMapper> mapper;
	mapper->SetInputConnection(planeSource->GetOutputPort());
	vtkNew<vtkActor> actorPlane;
	actorPlane->SetMapper(mapper);
	actorPlane->GetProperty()->SetColor(r, g, b);
	renderer->AddActor(actorPlane);
	return actorPlane;
}

vtkSmartPointer<vtkActor> Draw::createPolygon(vtkSmartPointer<vtkRenderer> renderer, const std::vector<double*>& polygonPoints, double r, double g, double b)
{
	vtkNew<vtkPoints> points;
	for (const auto& point : polygonPoints)
	{
		points->InsertNextPoint(point);
	}
	// Create the polygon
	vtkNew<vtkPolygon> polygon;
	polygon->GetPointIds()->SetNumberOfIds(points->GetNumberOfPoints());
	const unsigned int numberOfPoints = points->GetNumberOfPoints();
	for (unsigned int i = 0; i < numberOfPoints; i++)
	{
		polygon->GetPointIds()->SetId(i, i);
	}
	// Add the polygon to a list of polygons
	vtkNew<vtkCellArray> polygons;
	polygons->InsertNextCell(polygon);
	// Create a PolyData
	vtkNew<vtkPolyData> polygonPolyData;
	polygonPolyData->SetPoints(points);
	polygonPolyData->SetPolys(polygons);
	vtkNew<vtkPolyDataMapper> mapper;
	mapper->SetInputData(polygonPolyData);
	vtkNew<vtkActor> actorPolygon;
	actorPolygon->SetMapper(mapper);
	actorPolygon->GetProperty()->SetColor(r, g, b);
	renderer->AddActor(actorPolygon);
	return actorPolygon;
}

vtkSmartPointer<vtkImageActor> Draw::createImageActor(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkImageData> imgData)
{
	if (!imgData)
	{
		return nullptr;
	}
	vtkNew<vtkImageActor> actor;
	actor->GetMapper()->SetInputData(imgData);
	renderer->AddActor(actor);
	return actor;
}

vtkSmartPointer<vtkActor> Draw::createImage360Actor(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkImageData> imgData, double radius)
{
	if (!imgData)
	{
		return nullptr;
	}
	vtkNew<vtkSphereSource> sphere;
	sphere->SetRadius(radius);
	sphere->SetPhiResolution(100);
	sphere->SetThetaResolution(100);

	vtkNew<vtkOpenGLPolyDataMapper> mapper;
	mapper->SetInputConnection(sphere->GetOutputPort());
	mapper->AddShaderReplacement(
		vtkShader::Vertex,
		"//VTK::PositionVC::Dec", // replace
		true, // before the standard replacements
		"//VTK::PositionVC::Dec\n" // we still want the default
		"out vec3 TexCoords;\n",
		false // only do it once
	);
	mapper->AddShaderReplacement(
		vtkShader::Vertex,
		"//VTK::PositionVC::Impl", // replace
		true, // before the standard replacements
		"//VTK::PositionVC::Impl\n" // we still want the default
		"vec3 camPos = -MCVCMatrix[3].xyz * mat3(MCVCMatrix);\n"
		"TexCoords.xyz = reflect(vertexMC.xyz - camPos, normalize(normalMC));\n",
		false // only do it once
	);
	mapper->AddShaderReplacement(
		vtkShader::Fragment,
		"//VTK::Light::Dec", // replace
		true, // before the standard replacements
		"//VTK::Light::Dec\n" // we still want the default
		"in vec3 TexCoords;\n",
		false // only do it once
	);
	mapper->AddShaderReplacement(
		vtkShader::Fragment,
		"//VTK::Light::Impl", // replace
		true, // before the standard replacements
		"//VTK::Light::Impl\n"
		"  float phix = length(vec2(TexCoords.x, TexCoords.z));\n"
		"  vec3 skyColor = texture(actortexture, vec2(0.5*atan(TexCoords.z, TexCoords.x)/3.1415927 + 0.5, atan(TexCoords.y,phix)/3.1415927 + 0.5)).xyz;\n"
		"  gl_FragData[0] = vec4(skyColor, opacity);\n"
		, // we still want the default
		false // only do it once
	);

	vtkNew<vtkTexture> texture;
	texture->InterpolateOn();

	vtkNew<vtkImageFlip> flip;
	flip->SetInputData(imgData);
	flip->SetFilteredAxis(0); // flip x axis

	texture->SetInputConnection(flip->GetOutputPort());
	//texture->SetInputData(imgData);
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetTexture(texture);
	actor->SetMapper(mapper);

	//actor->RotateY(90);

	renderer->AddActor(actor);
	return actor;
}

vtkSmartPointer<vtkActor> Draw::createPolyData(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkPolyData> polyData, double r, double g, double b)
{
	return createDataSet(renderer, polyData, r, g, b);
}

vtkSmartPointer<vtkActor> Draw::createPolyData(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkPolyData> polyData)
{
	//Create a mapper and actor
	vtkNew<vtkPolyDataMapper> mapper;
	mapper->ScalarVisibilityOn();
	mapper->SetScalarModeToUsePointData();
	mapper->SetInputData(polyData);
	vtkNew<vtkActor> actor;
	actor->SetMapper(mapper);
	renderer->AddActor(actor);
	return actor;
}

vtkSmartPointer<vtkActor> Draw::createDataSet(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkDataSet> dataSet, double r, double g, double b)
{
	//Create a mapper and actor
	vtkNew<vtkDataSetMapper> mapper;
	mapper->SetInputData(dataSet);
	vtkNew<vtkActor> actor;
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(r, g, b);
	renderer->AddActor(actor);
	return actor;
}

vtkSmartPointer<vtkActor> Draw::createFrustum(vtkSmartPointer<vtkRenderer> renderer, std::vector<double*> cameraPoints)
{
	vtkNew<vtkLineSource> lineSource;
	vtkNew<vtkPoints> points;
	points->InsertNextPoint(cameraPoints.at(0));
	points->InsertNextPoint(cameraPoints.at(1));
	points->InsertNextPoint(cameraPoints.at(4));
	points->InsertNextPoint(cameraPoints.at(0));
	points->InsertNextPoint(cameraPoints.at(3));
	points->InsertNextPoint(cameraPoints.at(4));
	points->InsertNextPoint(cameraPoints.at(1));
	points->InsertNextPoint(cameraPoints.at(2));
	points->InsertNextPoint(cameraPoints.at(0));
	points->InsertNextPoint(cameraPoints.at(2));
	points->InsertNextPoint(cameraPoints.at(3));
	lineSource->SetPoints(points);
	//Create a mapper and actor
	vtkNew<vtkPolyDataMapper> mapper;
	mapper->SetInputConnection(lineSource->GetOutputPort());
	vtkNew<vtkActor> actorFrustum;
	actorFrustum->SetMapper(mapper);
	//avoid picking this actor, to make the "set focus" command better
	actorFrustum->SetPickable(false);
	renderer->AddActor(actorFrustum);
	return actorFrustum;
}

vtkSmartPointer<vtkActor> Draw::createPointCloud(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkPolyData> meshPolyData, vtkSmartPointer<vtkSelection> selection, double r, double g, double b)
{
	vtkNew<vtkExtractSelectedIds> extractSelectedIds;
	extractSelectedIds->SetInputData(0, meshPolyData);
	extractSelectedIds->SetInputData(1, selection);
	extractSelectedIds->Update();

	vtkNew<vtkDataSetMapper> mapper;
	mapper->SetInputConnection(extractSelectedIds->GetOutputPort());
	mapper->SetScalarVisibility(false);

	vtkNew<vtkActor> actor;
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(1.0, 0.0, 0.0);
	actor->GetProperty()->SetPointSize(2);
	renderer->AddActor(actor);
	return actor;
}

vtkSmartPointer<vtkActor> Draw::createPointCloud(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkPoints> points, double r, double g, double b)
{
	vtkNew<vtkCellArray> vertices;
	const unsigned int numberOfPoints = points->GetNumberOfPoints();
	vertices->InsertNextCell(numberOfPoints);
	for (unsigned int i = 0; i < numberOfPoints; i++)
	{
		vertices->InsertCellPoint(i);
	}
	vtkNew<vtkPolyData> polyData;
	polyData->SetPoints(points);
	polyData->SetVerts(vertices);
	return createPolyData(renderer, polyData, r, g, b);
}

vtkSmartPointer<vtkActor> Draw::createPointCloud(vtkSmartPointer<vtkPoints> points, double r, double g, double b)
{
	vtkNew<vtkCellArray> vertices;
	const unsigned int numberOfPoints = points->GetNumberOfPoints();
	vertices->InsertNextCell(numberOfPoints);
	for (unsigned int i = 0; i < numberOfPoints; i++)
	{
		vertices->InsertCellPoint(i);
	}
	vtkNew<vtkPolyData> polyData;
	polyData->SetPoints(points);
	polyData->SetVerts(vertices);
	vtkNew<vtkPolyDataMapper> mapper;
	mapper->SetInputData(polyData);
	vtkNew<vtkActor> actor;
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(r, g, b);
	return actor;
}

vtkSmartPointer<vtkActor> Draw::createPointCloud(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkUnsignedCharArray> colors, vtkSmartPointer<vtkFloatArray> normals)
{
	vtkNew<vtkCellArray> vertices;
	const unsigned int numberOfPoints = points->GetNumberOfPoints();
	vertices->InsertNextCell(numberOfPoints);
	for (unsigned int i = 0; i < numberOfPoints; i++)
	{
		vertices->InsertCellPoint(i);
	}
	vtkNew<vtkPolyData> polyData;
	polyData->SetPoints(points);
	if (colors)
	{
		polyData->GetPointData()->SetScalars(colors);
	}
	if (normals)
	{
		polyData->GetPointData()->SetNormals(normals);
	}
	polyData->SetVerts(vertices);
	vtkNew<vtkPolyDataMapper> mapper;
	mapper->SetInputData(polyData);
	vtkNew<vtkActor> actor;
	actor->SetMapper(mapper);
	renderer->AddActor(actor);
	return actor;
}

vtkSmartPointer<vtkPolyData> Draw::createPointCloud(vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkUnsignedCharArray> colors, vtkSmartPointer<vtkFloatArray> normals)
{
	vtkNew<vtkCellArray> vertices;
	const unsigned int numberOfPoints = points->GetNumberOfPoints();
	vertices->InsertNextCell(numberOfPoints);
	for (unsigned int i = 0; i < numberOfPoints; i++)
	{
		vertices->InsertCellPoint(i);
	}
	vtkNew<vtkPolyData> polyData;
	polyData->SetPoints(points);
	if (colors)
	{
		polyData->GetPointData()->SetScalars(colors);
	}
	if (normals)
	{
		polyData->GetPointData()->SetNormals(normals);
	}
	polyData->SetVerts(vertices);
	return polyData;
}

vtkSmartPointer<vtkActor> Draw::createBoundingBox(vtkSmartPointer<vtkRenderer> renderer, double * boundingBox, double r, double g, double b)
{
	vtkNew<vtkPoints> points;
	points->InsertNextPoint(boundingBox[0], boundingBox[2], boundingBox[4]);
	points->InsertNextPoint(boundingBox[0], boundingBox[2], boundingBox[5]);
	points->InsertNextPoint(boundingBox[0], boundingBox[3], boundingBox[4]);
	points->InsertNextPoint(boundingBox[0], boundingBox[3], boundingBox[5]);
	points->InsertNextPoint(boundingBox[1], boundingBox[2], boundingBox[4]);
	points->InsertNextPoint(boundingBox[1], boundingBox[2], boundingBox[5]);
	points->InsertNextPoint(boundingBox[1], boundingBox[3], boundingBox[4]);
	points->InsertNextPoint(boundingBox[1], boundingBox[3], boundingBox[5]);
	return createBoundingBox(renderer, points, r, g, b);
}

vtkSmartPointer<vtkActor> Draw::createBoundingBox(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkPoints> points, double r, double g, double b)
{
	vtkNew<vtkPolyData> polyData;
	polyData->SetPoints(points);
	return createBoundingBox(renderer, polyData, r, g, b);
}

vtkSmartPointer<vtkActor> Draw::createBoundingBox(vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkPolyData> polyData, double r, double g, double b)
{
	vtkNew<vtkOutlineFilter> outlineFilter;
	outlineFilter->SetInputData(polyData);
	//Create a mapper and actor
	vtkNew<vtkPolyDataMapper> mapper;
	mapper->SetInputConnection(outlineFilter->GetOutputPort());
	vtkNew<vtkActor> actor;
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(r, g, b);
	renderer->AddActor(actor);
	return actor;
}
