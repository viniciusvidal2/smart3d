#ifdef SURFACETRIMMER_EXPORTS
#define SURFACETRIMMER_API __declspec(dllexport)
#else
#define SURFACETRIMMER_API __declspec(dllimport)
#endif

#ifndef __SURFACETRIMMER__H__
#define __SURFACETRIMMER__H__

#include <vector>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkCellArray.h>
#include <vtkTriangle.h>

//Used to define the reconstruction options
struct OptionsTrimmer
{
	OptionsTrimmer() {}
	OptionsTrimmer(float trimValue)
	{
		this->trimValue = trimValue;
	}
	float trimValue = 7;
};


class SURFACETRIMMER_API SurfaceTrimmer {
public:
	SurfaceTrimmer(void);
	vtkSmartPointer<vtkPolyData> trimSurface(vtkSmartPointer<vtkPolyData> inputPolyData, OptionsTrimmer* options);
private:
};

#endif