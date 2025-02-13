#pragma once
#include <tuple>
#include "PointStreamData.h"

struct VertexData
{
	float point[3];
	float normal[3];
	unsigned char color[3];
	float value = -1;
	//Used in the Extract method
	bool hasColor = 0;
};


template< typename Real, unsigned int Dim >
struct VertexDataExtractor
{
	typedef PlyVertexWithData< Real, Dim, MultiPointStreamData< Real, PointStreamNormal< Real, Dim >, PointStreamValue< Real >, MultiPointStreamData< Real, PointStreamColor< Real > > > > PlyVertexWithNormalValueAndColor;
	typedef PlyVertexWithData< Real, Dim, MultiPointStreamData< Real, PointStreamNormal< Real, Dim >, PointStreamValue< Real >, MultiPointStreamData< Real > > > PlyVertexWithNormalAndValue;
	typedef PlyVertexWithData< Real, Dim, MultiPointStreamData< Real, PointStreamNormal< Real, Dim >, MultiPointStreamData< Real, PointStreamColor< Real > > > > PlyVertexWithNormalAndColor;
	typedef PlyVertexWithData< Real, Dim, MultiPointStreamData< Real, PointStreamNormal< Real, Dim >, MultiPointStreamData< Real > > > PlyVertexWithNormal;

	template< typename Vertex >
	static void Extract(const Vertex &v, VertexData &vertex)
	{
		ERROR_OUT("Unrecognized vertex type");
	}
	template<>
	static void Extract(const PlyVertexWithNormalValueAndColor &v, VertexData &vertex)
	{
		//Point
		vertex.point[0] = v.point.coords[0];
		vertex.point[1] = v.point.coords[1];
		vertex.point[2] = v.point.coords[2];
		//Normal
		vertex.normal[0] = v.data.template data<0>().coords[0];
		vertex.normal[1] = v.data.template data<0>().coords[1];
		vertex.normal[2] = v.data.template data<0>().coords[2];
		//Color
		vertex.color[0] = std::get< 0 >(v.data.template data<2>()).data().coords[0];
		vertex.color[1] = std::get< 0 >(v.data.template data<2>()).data().coords[1];
		vertex.color[2] = std::get< 0 >(v.data.template data<2>()).data().coords[2];
		vertex.hasColor = true;
		//Value
		vertex.value = v.data.template data<1>();
	}
	template<>
	static void Extract(const PlyVertexWithNormalAndValue &v, VertexData &vertex)
	{
		//Point
		vertex.point[0] = v.point.coords[0];
		vertex.point[1] = v.point.coords[1];
		vertex.point[2] = v.point.coords[2];
		//Normal
		vertex.normal[0] = v.data.template data<0>().coords[0];
		vertex.normal[1] = v.data.template data<0>().coords[1];
		vertex.normal[2] = v.data.template data<0>().coords[2];
		//Value
		vertex.value = v.data.template data<1>();
	}
	template<>
	static void Extract(const PlyVertexWithNormalAndColor &v, VertexData &vertex)
	{
		//Point
		vertex.point[0] = v.point.coords[0];
		vertex.point[1] = v.point.coords[1];
		vertex.point[2] = v.point.coords[2];
		//Normal
		vertex.normal[0] = v.data.template data<0>().coords[0];
		vertex.normal[1] = v.data.template data<0>().coords[1];
		vertex.normal[2] = v.data.template data<0>().coords[2];
		//Color
		vertex.color[0] = std::get< 0 >(v.data.template data<1>()).data().coords[0];
		vertex.color[1] = std::get< 0 >(v.data.template data<1>()).data().coords[1];
		vertex.color[2] = std::get< 0 >(v.data.template data<1>()).data().coords[2];
		vertex.hasColor = true;
	}
	template<>
	static void Extract(const PlyVertexWithNormal &v, VertexData &vertex)
	{
		//Point
		vertex.point[0] = v.point.coords[0];
		vertex.point[1] = v.point.coords[1];
		vertex.point[2] = v.point.coords[2];
		//Normal
		vertex.normal[0] = v.data.template data<0>().coords[0];
		vertex.normal[1] = v.data.template data<0>().coords[1];
		vertex.normal[2] = v.data.template data<0>().coords[2];
	}
};

template< typename Real, unsigned int Dim >
struct VertexDataSetter
{
	typedef PlyVertexWithData< Real, Dim, MultiPointStreamData< Real, PointStreamNormal< Real, Dim >, MultiPointStreamData< Real, PointStreamColor< Real > > > > PlyVertexWithNormalAndColor;
	typedef PlyVertexWithData< Real, Dim, MultiPointStreamData< Real, PointStreamNormal< Real, Dim >, MultiPointStreamData< Real > > > PlyVertexWithNormal;

	template< typename Vertex >
	static void Set(Vertex &v, VertexData &vertex)
	{
		ERROR_OUT("Unrecognized vertex type");
	}
	template<>
	static void Set(PlyVertexWithNormalAndColor &v, VertexData &vertex)
	{
		//Point
		v.point.coords[0] = vertex.point[0];
		v.point.coords[1] = vertex.point[1];
		v.point.coords[2] = vertex.point[2];
		//Normal
		v.data.template data<0>().coords[0] = vertex.normal[0];
		v.data.template data<0>().coords[1] = vertex.normal[1];
		v.data.template data<0>().coords[2] = vertex.normal[2];
		//Color
		std::get< 0 >(v.data.template data<1>()).data().coords[0] = vertex.color[0];
		std::get< 0 >(v.data.template data<1>()).data().coords[1] = vertex.color[1];
		std::get< 0 >(v.data.template data<1>()).data().coords[2] = vertex.color[2];
	}
	template<>
	static void Set(PlyVertexWithNormal &v, VertexData &vertex)
	{
		//Point
		v.point.coords[0] = vertex.point[0];
		v.point.coords[1] = vertex.point[1];
		v.point.coords[2] = vertex.point[2];
		//Normal
		v.data.template data<0>().coords[0] = vertex.normal[0];
		v.data.template data<0>().coords[1] = vertex.normal[1];
		v.data.template data<0>().coords[2] = vertex.normal[2];
	}
};