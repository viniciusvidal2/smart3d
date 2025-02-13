/*
Copyright (c) 2013, Michael Kazhdan
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer. Redistributions in binary form must reproduce
the above copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the distribution.

Neither the name of the Johns Hopkins University nor the names of its contributors
may be used to endorse or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.
*/

#include "PreProcessor.h"

#define DIMENSION 3

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <algorithm>
#include "FEMTree.h"
#include "MyMiscellany.h"
#include "CmdLineParser.h"
#include "MAT.h"
#include "Geometry.h"
#include "Ply.h"
#include "PointStreamData.h"
#include "SurfaceTrimmer.h"

struct VertexDataAux
{
	float point[3];
	float normal[3];
	unsigned char color[3];
	float value = -1;
};

template< typename Real>
struct VertexDataExtractor
{
	typedef PlyVertexWithData< Real, DIMENSION, MultiPointStreamData< Real, PointStreamValue< Real >, PointStreamNormal< Real, DIMENSION >, PointStreamColor< Real > > > PlyVertexWithValueNormalAndColor;
	typedef PlyVertexWithData< Real, DIMENSION, MultiPointStreamData< Real, PointStreamValue< Real >, PointStreamNormal< Real, DIMENSION > > > PlyVertexWithValueAndNormal;
	typedef PlyVertexWithData< Real, DIMENSION, MultiPointStreamData< Real, PointStreamValue< Real >, PointStreamColor< Real > > > PlyVertexWithValueAndColor;
	typedef PlyVertexWithData< Real, DIMENSION, MultiPointStreamData< Real, PointStreamValue< Real > > > PlyVertexWithValue;

	template< typename Vertex >
	static void Extract(const Vertex &v, VertexDataAux &vertex)
	{
		ERROR_OUT("Unrecognized vertex type");
	}
	template<>
	static void Extract(const PlyVertexWithValueNormalAndColor &v, VertexDataAux &vertex)
	{
		for (unsigned int i = 0; i < 3; i++)
		{
			//Point
			vertex.point[i] = v.point.coords[i];
			//Normal
			vertex.normal[i] = v.data.template data<1>().coords[i];
			//Color
			vertex.color[i] = v.data.template data<2>().coords[i];
		}
		//Value
		vertex.value = v.data.template data<0>();
	}
	template<>
	static void Extract(const PlyVertexWithValueAndNormal &v, VertexDataAux &vertex)
	{
		for (unsigned int i = 0; i < 3; i++)
		{
			//Point
			vertex.point[i] = v.point.coords[i];
			//Normal
			vertex.normal[i] = v.data.template data<1>().coords[i];
		}
		//Value
		vertex.value = v.data.template data<0>();
	}
	template<>
	static void Extract(const PlyVertexWithValueAndColor &v, VertexDataAux &vertex)
	{
		for (unsigned int i = 0; i < 3; i++)
		{
			//Point
			vertex.point[i] = v.point.coords[i];
			//Color
			vertex.color[i] = v.data.template data<1>().coords[i];
		}
		//Value
		vertex.value = v.data.template data<0>();
	}
	template<>
	static void Extract(const PlyVertexWithValue &v, VertexDataAux &vertex)
	{
		for (unsigned int i = 0; i < 3; i++)
		{
			//Point
			vertex.point[i] = v.point.coords[i];
		}
		//Value
		vertex.value = v.data.template data<0>();
	}
};

template< typename Real>
struct VertexDataSetter
{
	typedef PlyVertexWithData< Real, DIMENSION, MultiPointStreamData< Real, PointStreamValue< Real >, PointStreamNormal< Real, DIMENSION >, PointStreamColor< Real > > > PlyVertexWithValueNormalAndColor;
	typedef PlyVertexWithData< Real, DIMENSION, MultiPointStreamData< Real, PointStreamValue< Real >, PointStreamNormal< Real, DIMENSION > > > PlyVertexWithValueAndNormal;
	typedef PlyVertexWithData< Real, DIMENSION, MultiPointStreamData< Real, PointStreamValue< Real >, PointStreamColor< Real > > > PlyVertexWithValueAndColor;
	typedef PlyVertexWithData< Real, DIMENSION, MultiPointStreamData< Real, PointStreamValue< Real > > > PlyVertexWithValue;

	template< typename Vertex >
	static void Set(Vertex &v, VertexDataAux &vertex)
	{
		ERROR_OUT("Unrecognized vertex type");
	}
	template<>
	static void Set(PlyVertexWithValueNormalAndColor &v, VertexDataAux &vertex)
	{
		for (unsigned int i = 0; i < 3; i++)
		{
			//Point
			v.point.coords[i] = vertex.point[i];
			//Normal
			v.data.template data<1>().coords[i] = vertex.normal[i];
			//Color
			v.data.template data<2>().coords[i] = vertex.color[i];
		}
		//Value
		v.data.template data<0>() = vertex.value;
	}
	template<>
	static void Set(PlyVertexWithValueAndNormal &v, VertexDataAux &vertex)
	{
		for (unsigned int i = 0; i < 3; i++)
		{
			//Point
			v.point.coords[i] = vertex.point[i];
			//Normal
			v.data.template data<1>().coords[i] = vertex.normal[i];
		}
		//Value
		v.data.template data<0>() = vertex.value;
	}
	template<>
	static void Set(PlyVertexWithValueAndColor &v, VertexDataAux &vertex)
	{
		for (unsigned int i = 0; i < 3; i++)
		{
			//Point
			v.point.coords[i] = vertex.point[i];
			//Color
			v.data.template data<1>().coords[i] = vertex.color[i];
		}
		//Value
		v.data.template data<0>() = vertex.value;
	}
	template<>
	static void Set(PlyVertexWithValue &v, VertexDataAux &vertex)
	{
		for (unsigned int i = 0; i < 3; i++)
		{
			//Point
			v.point.coords[i] = vertex.point[i];
		}
		//Value
		v.data.template data<0>() = vertex.value;
	}
};

MessageWriter messageWriter;


cmdLineParameter< char* >
In("in"),
Out("out");
cmdLineParameter< int >
Smooth("smooth", 5);
cmdLineParameter< float >
Trim("trim"),
IslandAreaRatio("aRatio", 0.001f);
cmdLineReadable
PolygonMesh("polygonMesh"),
Long("long"),
Verbose("verbose");


cmdLineReadable* params[] =
{
	&In , &Out , &Trim , &PolygonMesh , &Smooth , &IslandAreaRatio , &Verbose , &Long ,
	NULL
};

void ShowUsage(char* ex)
{
	printf("Usage: %s\n", ex);
	printf("\t --%s <input polygon mesh>\n", In.name);
	printf("\t --%s <trimming value>\n", Trim.name);
	printf("\t[--%s <ouput polygon mesh>]\n", Out.name);
	printf("\t[--%s <smoothing iterations>=%d]\n", Smooth.name, Smooth.value);
	printf("\t[--%s <relative area of islands>=%f]\n", IslandAreaRatio.name, IslandAreaRatio.value);
	printf("\t[--%s]\n", PolygonMesh.name);
	printf("\t[--%s]\n", Long.name);
	printf("\t[--%s]\n", Verbose.name);
}

template< typename Index >
struct EdgeKey
{
	Index key1, key2;
	EdgeKey(Index k1 = 0, Index k2 = 0) : key1(k1), key2(k2) {}
	bool operator == (const EdgeKey &key) const { return key1 == key.key1 && key2 == key.key2; }
#if 1
	struct Hasher { size_t operator()(const EdgeKey &key) const { return (size_t)(key.key1 * key.key2); } };
#else
	struct Hasher { size_t operator()(const EdgeKey &key) const { return key.key1 ^ key.key2; } };
#endif
};

template< typename Real, typename ... VertexData >
PlyVertexWithData< float, DIMENSION, MultiPointStreamData< float, PointStreamValue< float >, VertexData ... > > InterpolateVertices(const PlyVertexWithData< float, DIMENSION, MultiPointStreamData< float, PointStreamValue< float >, VertexData ... > >& v1, const PlyVertexWithData< float, DIMENSION, MultiPointStreamData< float, PointStreamValue< float >, VertexData ... > >& v2, Real value)
{
	if (v1.data.template data<0>() == v2.data.template data<0>()) return (v1 + v2) / Real(2.);
	Real dx = (v1.data.template data<0>() - value) / (v1.data.template data<0>() - v2.data.template data<0>());
	return v1 * (1.f - dx) + v2 * dx;
}

template< typename Real, typename Index, typename ... VertexData >
void SmoothValues(std::vector< PlyVertexWithData< float, DIMENSION, MultiPointStreamData< float, PointStreamValue< float >, VertexData ... > > >& vertices, const std::vector< std::vector< Index > >& polygons)
{
	std::vector< int > count(vertices.size());
	std::vector< Real > sums(vertices.size(), 0);
	for (size_t i = 0; i < polygons.size(); i++)
	{
		int sz = int(polygons[i].size());
		for (int j = 0; j < sz; j++)
		{
			int j1 = j, j2 = (j + 1) % sz;
			Index v1 = polygons[i][j1], v2 = polygons[i][j2];
			count[v1]++, count[v2]++;
			sums[v1] += vertices[v2].data.template data<0>(), sums[v2] += vertices[v1].data.template data<0>();
		}
	}
	for (size_t i = 0; i < vertices.size(); i++) vertices[i].data.template data<0>() = (sums[i] + vertices[i].data.template data<0>()) / (count[i] + 1);
}

template< class Real, typename Index, typename ... VertexData >
void SplitPolygon
(
	const std::vector< Index >& polygon,
	std::vector< PlyVertexWithData< float, DIMENSION, MultiPointStreamData< float, PointStreamValue< float >, VertexData ... > > >& vertices,
	std::vector< std::vector< Index > >* ltPolygons, std::vector< std::vector< Index > >* gtPolygons,
	std::vector< bool >* ltFlags, std::vector< bool >* gtFlags,
	std::unordered_map< EdgeKey< Index >, Index, typename EdgeKey< Index >::Hasher >& vertexTable,
	Real trimValue
)
{
	int sz = int(polygon.size());
	std::vector< bool > gt(sz);
	int gtCount = 0;
	for (int j = 0; j < sz; j++)
	{
		gt[j] = (vertices[polygon[j]].data.template data<0>() > trimValue);
		if (gt[j]) gtCount++;
	}
	if (gtCount == sz) { if (gtPolygons) gtPolygons->push_back(polygon); if (gtFlags) gtFlags->push_back(false); }
	else if (gtCount == 0) { if (ltPolygons) ltPolygons->push_back(polygon); if (ltFlags) ltFlags->push_back(false); }
	else
	{
		int start;
		for (start = 0; start < sz; start++) if (gt[start] && !gt[(start + sz - 1) % sz]) break;

		bool gtFlag = true;
		std::vector< Index > poly;

		// Add the initial vertex
		{
			int j1 = (start + int(sz) - 1) % sz, j2 = start;
			Index v1 = polygon[j1], v2 = polygon[j2], vIdx;
			typename std::unordered_map< EdgeKey< Index >, Index, typename EdgeKey< Index >::Hasher >::iterator iter = vertexTable.find(EdgeKey< Index >(v1, v2));
			if (iter == vertexTable.end())
			{
				vertexTable[EdgeKey< Index >(v1, v2)] = vIdx = (Index)vertices.size();
				vertices.push_back(InterpolateVertices(vertices[v1], vertices[v2], trimValue));
			}
			else vIdx = iter->second;
			poly.push_back(vIdx);
		}

		for (int _j = 0; _j <= sz; _j++)
		{
			int j1 = (_j + start + sz - 1) % sz, j2 = (_j + start) % sz;
			Index v1 = polygon[j1], v2 = polygon[j2];
			if (gt[j2] == gtFlag) poly.push_back(v2);
			else
			{
				Index vIdx;
				typename std::unordered_map< EdgeKey< Index >, Index, typename EdgeKey< Index >::Hasher >::iterator iter = vertexTable.find(EdgeKey< Index >(v1, v2));
				if (iter == vertexTable.end())
				{
					vertexTable[EdgeKey< Index >(v1, v2)] = vIdx = (Index)vertices.size();
					vertices.push_back(InterpolateVertices(vertices[v1], vertices[v2], trimValue));
				}
				else vIdx = iter->second;
				poly.push_back(vIdx);
				if (gtFlag) { if (gtPolygons) gtPolygons->push_back(poly); if (ltFlags) ltFlags->push_back(true); }
				else { if (ltPolygons) ltPolygons->push_back(poly); if (gtFlags) gtFlags->push_back(true); }
				poly.clear(), poly.push_back(vIdx), poly.push_back(v2);
				gtFlag = !gtFlag;
			}
		}
	}
}

template< class Real, typename Index, class Vertex >
void Triangulate(const std::vector< Vertex >& vertices, const std::vector< std::vector< Index > >& polygons, std::vector< std::vector< Index > >& triangles)
{
	triangles.clear();
	for (size_t i = 0; i < polygons.size(); i++)
		if (polygons.size() > 3)
		{
			std::vector< Point< Real, DIMENSION > > _vertices(polygons[i].size());
			for (int j = 0; j<int(polygons[i].size()); j++) _vertices[j] = vertices[polygons[i][j]].point;
			std::vector< TriangleIndex< Index > > _triangles = MinimalAreaTriangulation< Index, Real, DIMENSION >((ConstPointer(Point< Real, DIMENSION >))GetPointer(_vertices), _vertices.size());

			// Add the triangles to the mesh
			size_t idx = triangles.size();
			triangles.resize(idx + _triangles.size());
			for (int j = 0; j<int(_triangles.size()); j++)
			{
				triangles[idx + j].resize(3);
				for (int k = 0; k < 3; k++) triangles[idx + j][k] = polygons[i][_triangles[j].idx[k]];
			}
		}
		else if (polygons[i].size() == 3) triangles.push_back(polygons[i]);
}

template< class Real, typename Index, class Vertex >
double PolygonArea(const std::vector< Vertex >& vertices, const std::vector< Index >& polygon)
{
	if (polygon.size() < 3) return 0.;
	else if (polygon.size() == 3) return Area(vertices[polygon[0]].point, vertices[polygon[1]].point, vertices[polygon[2]].point);
	else
	{
		Point< Real, DIMENSION > center;
		for (size_t i = 0; i < polygon.size(); i++) center += vertices[polygon[i]].point;
		center /= Real(polygon.size());
		double area = 0;
		for (size_t i = 0; i < polygon.size(); i++) area += Area(center, vertices[polygon[i]].point, vertices[polygon[(i + 1) % polygon.size()]].point);
		return area;
	}
}

template< typename Index, class Vertex >
void RemoveHangingVertices(std::vector< Vertex >& vertices, std::vector< std::vector< Index > >& polygons)
{
	std::unordered_map< Index, Index > vMap;
	std::vector< bool > vertexFlags(vertices.size(), false);
	for (size_t i = 0; i < polygons.size(); i++) for (size_t j = 0; j < polygons[i].size(); j++) vertexFlags[polygons[i][j]] = true;
	Index vCount = 0;
	for (Index i = 0; i < (Index)vertices.size(); i++) if (vertexFlags[i]) vMap[i] = vCount++;
	for (size_t i = 0; i < polygons.size(); i++) for (size_t j = 0; j < polygons[i].size(); j++) polygons[i][j] = vMap[polygons[i][j]];

	std::vector< Vertex > _vertices(vCount);
	for (Index i = 0; i < (Index)vertices.size(); i++) if (vertexFlags[i]) _vertices[vMap[i]] = vertices[i];
	vertices = _vertices;
}

template< typename Index >
void SetConnectedComponents(const std::vector< std::vector< Index > >& polygons, std::vector< std::vector< Index > >& components)
{
	std::vector< Index > polygonRoots(polygons.size());
	for (size_t i = 0; i < polygons.size(); i++) polygonRoots[i] = (Index)i;
	std::unordered_map< EdgeKey< Index >, Index, typename EdgeKey< Index >::Hasher > edgeTable;
	for (size_t i = 0; i < polygons.size(); i++)
	{
		int sz = int(polygons[i].size());
		for (int j = 0; j < sz; j++)
		{
			int j1 = j, j2 = (j + 1) % sz;
			Index v1 = polygons[i][j1], v2 = polygons[i][j2];
			EdgeKey< Index > eKey = EdgeKey< Index >(v1, v2);
			typename std::unordered_map< EdgeKey< Index >, Index, typename EdgeKey< Index >::Hasher >::iterator iter = edgeTable.find(eKey);
			if (iter == edgeTable.end()) edgeTable[eKey] = (Index)i;
			else
			{
				Index p = iter->second;
				while (polygonRoots[p] != p)
				{
					Index temp = polygonRoots[p];
					polygonRoots[p] = (Index)i;
					p = temp;
				}
				polygonRoots[p] = (Index)i;
			}
		}
	}
	for (size_t i = 0; i < polygonRoots.size(); i++)
	{
		Index p = (Index)i;
		while (polygonRoots[p] != p) p = polygonRoots[p];
		Index root = p;
		p = (Index)i;
		while (polygonRoots[p] != p)
		{
			Index temp = polygonRoots[p];
			polygonRoots[p] = root;
			p = temp;
		}
	}
	int cCount = 0;
	std::unordered_map< Index, Index > vMap;
	for (Index i = 0; i < (Index)polygonRoots.size(); i++) if (polygonRoots[i] == i) vMap[i] = cCount++;
	components.resize(cCount);
	for (Index i = 0; i < (Index)polygonRoots.size(); i++) components[vMap[polygonRoots[i]]].push_back(i);
}

template< typename Index, typename ... VertexData >
vtkSmartPointer<vtkPolyData> Execute(vtkSmartPointer<vtkPolyData> inputPolyData)
{
	typedef PlyVertexWithData< float, DIMENSION, MultiPointStreamData< float, PointStreamValue< float >, VertexData ... > > Vertex;
	std::vector< Vertex > vertices;
	std::vector< std::vector< Index > > polygons;

	//Load
	//Points
	vtkSmartPointer<vtkPoints> inputPoints = inputPolyData->GetPoints();
	//Normals
	vtkSmartPointer<vtkFloatArray> inputNormals = nullptr;
	if (inputPolyData->GetPointData()->GetNormals())
	{
		inputNormals = vtkFloatArray::SafeDownCast(inputPolyData->GetPointData()->GetNormals());
	}
	//Colors
	vtkSmartPointer<vtkUnsignedCharArray> inputColors = nullptr;
	bool hasAlpha = false;
	if (inputPolyData->GetPointData()->GetScalars())
	{
		inputColors = vtkUnsignedCharArray::SafeDownCast(inputPolyData->GetPointData()->GetScalars());
		if (inputColors->GetNumberOfComponents() == 4)
		{
			hasAlpha = true;
		}
	}
	//Values
	vtkSmartPointer<vtkFloatArray> inputValues = vtkFloatArray::SafeDownCast(inputPolyData->GetPointData()->GetArray("Values"));

	size_t numberOfPoints = inputPoints->GetNumberOfPoints();
	vertices.resize(numberOfPoints);
#pragma omp parallel for
	for (int i = 0; i < numberOfPoints; i++)
	{
		double pointAux[3];
		inputPoints->GetPoint(i, pointAux);
		double normalAux[3];
		if (inputNormals)
		{
			inputNormals->GetTuple(i, normalAux);
		}
		double colorAux4[4];
		double colorAux3[3];
		if (inputColors)
		{
			if (hasAlpha)
			{
				inputColors->GetTuple(i, colorAux4);
			}
			else
			{
				inputColors->GetTuple(i, colorAux3);
			}
		}
		VertexDataAux vertex;
		for (size_t j = 0; j < 3; j++)
		{
			vertex.point[j] = pointAux[j];
			if (inputNormals)
			{
				vertex.normal[j] = normalAux[j];
			}
			if (inputColors)
			{
				if (hasAlpha)
				{
					vertex.color[j] = colorAux4[j];
				}
				else
				{
					vertex.color[j] = colorAux3[j];
				}
			}
		}
		double valueAux;
		inputValues->GetTuple(i, &valueAux);
		vertex.value = valueAux;
		Vertex v;
		VertexDataSetter<float>::Set(v, vertex);
		vertices[i] = v;
	}

	vtkSmartPointer<vtkCellArray> inputPolygons = inputPolyData->GetPolys();
	int numberOfPolygons = inputPolygons->GetNumberOfCells();
	polygons.resize(numberOfPolygons);
	inputPolygons->InitTraversal();
	vtkNew<vtkIdList> idList;
	for (int i = 0; i < numberOfPolygons; i++)
	{
		inputPolygons->GetNextCell(idList);
		std::vector< Index > indexes;
		for (int j = 0; j < idList->GetNumberOfIds(); j++)
		{
			indexes.push_back(idList->GetId(j));
		}
		polygons[i] = indexes;
	}

	for (int i = 0; i < Smooth.value; i++) SmoothValues< float, Index >(vertices, polygons);


	std::unordered_map< EdgeKey< Index >, Index, typename EdgeKey< Index >::Hasher > vertexTable;
	std::vector< std::vector< Index > > ltPolygons, gtPolygons;
	std::vector< bool > ltFlags, gtFlags;

	double t = Time();
	for (size_t i = 0; i < polygons.size(); i++) SplitPolygon(polygons[i], vertices, &ltPolygons, &gtPolygons, &ltFlags, &gtFlags, vertexTable, Trim.value);
	if (IslandAreaRatio.value > 0)
	{
		std::vector< std::vector< Index > > _ltPolygons, _gtPolygons;
		std::vector< std::vector< Index > > ltComponents, gtComponents;
		SetConnectedComponents(ltPolygons, ltComponents);
		SetConnectedComponents(gtPolygons, gtComponents);
		std::vector< double > ltAreas(ltComponents.size(), 0.), gtAreas(gtComponents.size(), 0.);
		std::vector< bool > ltComponentFlags(ltComponents.size(), false), gtComponentFlags(gtComponents.size(), false);
		double area = 0.;
		for (size_t i = 0; i < ltComponents.size(); i++)
		{
			for (size_t j = 0; j < ltComponents[i].size(); j++)
			{
				ltAreas[i] += PolygonArea< float, Index, Vertex >(vertices, ltPolygons[ltComponents[i][j]]);
				ltComponentFlags[i] = (ltComponentFlags[i] || ltFlags[ltComponents[i][j]]);
			}
			area += ltAreas[i];
		}
		for (size_t i = 0; i < gtComponents.size(); i++)
		{
			for (size_t j = 0; j < gtComponents[i].size(); j++)
			{
				gtAreas[i] += PolygonArea< float, Index, Vertex >(vertices, gtPolygons[gtComponents[i][j]]);
				gtComponentFlags[i] = (gtComponentFlags[i] || gtFlags[gtComponents[i][j]]);
			}
			area += gtAreas[i];
		}
		for (size_t i = 0; i < ltComponents.size(); i++)
		{
			if (ltAreas[i] < area*IslandAreaRatio.value && ltComponentFlags[i]) for (size_t j = 0; j < ltComponents[i].size(); j++) _gtPolygons.push_back(ltPolygons[ltComponents[i][j]]);
			else                                                               for (size_t j = 0; j < ltComponents[i].size(); j++) _ltPolygons.push_back(ltPolygons[ltComponents[i][j]]);
		}
		for (size_t i = 0; i < gtComponents.size(); i++)
		{
			if (gtAreas[i] < area*IslandAreaRatio.value && gtComponentFlags[i]) for (size_t j = 0; j < gtComponents[i].size(); j++) _ltPolygons.push_back(gtPolygons[gtComponents[i][j]]);
			else                                                               for (size_t j = 0; j < gtComponents[i].size(); j++) _gtPolygons.push_back(gtPolygons[gtComponents[i][j]]);
		}
		ltPolygons = _ltPolygons, gtPolygons = _gtPolygons;
	}
	if (!PolygonMesh.set)
	{
		{
			std::vector< std::vector< Index > > polys = ltPolygons;
			Triangulate< float, Index, Vertex >(vertices, ltPolygons, polys), ltPolygons = polys;
		}
		{
			std::vector< std::vector< Index > > polys = gtPolygons;
			Triangulate< float, Index, Vertex >(vertices, gtPolygons, polys), gtPolygons = polys;
		}
	}

	RemoveHangingVertices(vertices, gtPolygons);
	//Output
	size_t numberOfVertices = vertices.size();
	vtkNew<vtkPoints> outputPoints;
	outputPoints->SetNumberOfPoints(numberOfVertices);
	vtkNew<vtkFloatArray> outputValues;
	outputValues->SetNumberOfComponents(1);
	outputValues->SetNumberOfTuples(numberOfVertices);
	outputValues->SetName("Values");
	vtkSmartPointer<vtkFloatArray> outputNormals = nullptr;
	if (inputNormals)
	{
		outputNormals = vtkSmartPointer<vtkFloatArray>::New();
		outputNormals->SetNumberOfComponents(3);
		outputNormals->SetNumberOfTuples(numberOfVertices);
		outputNormals->SetName("Normals");
	}
	vtkSmartPointer<vtkUnsignedCharArray> outputColors = nullptr;
	if (inputColors)
	{
		outputColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
		outputColors->SetNumberOfComponents(3);
		outputColors->SetNumberOfTuples(numberOfVertices);
		outputColors->SetName("RGB");
	}

#pragma omp parallel for
	for (int i = 0; i < numberOfVertices; i++)
	{
		VertexDataAux vertex;
		VertexDataExtractor<float>::Extract(vertices[i], vertex);
		outputPoints->InsertPoint(i, vertex.point);
		if (outputNormals)
		{
			outputNormals->InsertTuple3(i, vertex.normal[0], vertex.normal[1], vertex.normal[2]);
		}
		if (outputColors)
		{
			outputColors->InsertTuple3(i, vertex.color[0], vertex.color[1], vertex.color[2]);
		}
		outputValues->InsertTuple1(i, vertex.value);
	}

	vtkNew<vtkCellArray> outputTriangles;
	int idxTri = 0;
	for (auto poly : gtPolygons)
	{
		vtkNew<vtkTriangle> tri;
		idxTri = 0;
		for (auto idx : poly)
		{
			tri->GetPointIds()->InsertId(idxTri, idx);
			idxTri++;
		}
		outputTriangles->InsertNextCell(tri);
	}
	vtkNew<vtkPolyData> outputPolyData;
	outputPolyData->SetPoints(outputPoints);
	if (outputNormals)
	{
		outputPolyData->GetPointData()->SetNormals(outputNormals);
	}
	if (outputColors)
	{
		outputPolyData->GetPointData()->SetScalars(outputColors);
	}
	outputPolyData->GetPointData()->AddArray(outputValues);
	outputPolyData->SetPolys(outputTriangles);
	return outputPolyData;
}
vtkSmartPointer<vtkPolyData> SurfaceTrimmer::trimSurface(vtkSmartPointer<vtkPolyData> inputPolyData, OptionsTrimmer* options)
{
	messageWriter.echoSTDOUT = Verbose.set;

	if (!inputPolyData)
	{
		return nullptr;
	}
	bool hasNormal = false;
	bool hasColor = false;
	if (inputPolyData->GetPointData())
	{
		if (inputPolyData->GetPointData()->GetNormals())
		{
			hasNormal = true;
		}
		if (inputPolyData->GetPointData()->GetScalars())
		{
			hasColor = true;
		}
		if (!inputPolyData->GetPointData()->HasArray("Values"))
		{
			ERROR_OUT("No values found");
			return nullptr;
		}
	}
	else
	{
		return nullptr;
	}
	Trim.value = options->trimValue;
	vtkSmartPointer<vtkPolyData> outputPolyData = nullptr;
	if (hasColor)
	{
		if (hasNormal)
		{
			outputPolyData = Execute< int, PointStreamNormal< float, DIMENSION >, PointStreamColor< float > >(inputPolyData);
		}
		else
		{
			outputPolyData = Execute< int, PointStreamColor< float > >(inputPolyData);
		}
	}
	else
	{
		if (hasNormal)
		{
			outputPolyData = Execute< int, PointStreamNormal< float, DIMENSION >                             >(inputPolyData);
		}
		else
		{
			outputPolyData = Execute< int                                                                      >(inputPolyData);
		}
	}
	return outputPolyData;
}

SurfaceTrimmer::SurfaceTrimmer(void)
{
}
