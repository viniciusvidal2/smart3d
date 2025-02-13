#ifdef SSDRECON_EXPORTS
#define SSDRECON_API __declspec(dllexport)
#else
#define SSDRECON_API __declspec(dllimport)
#endif

#ifndef __SSDRECON__H__
#define __SSDRECON__H__

#include <vector>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkCellArray.h>
#include <vtkTriangle.h>

//Used to define the reconstruction options
struct OptionsSSD
{
	OptionsSSD() {}
	OptionsSSD(int depth, int boundary)
	{
		this->depth = depth;
		this->boundary = boundary;
	}
	int depth = 8;
	int boundary = 1;
	int valueWeight = 1;
	int gradientWeight = 1;
	int biLapWeight = 1;
};

class SSDRECON_API SSDRecon {
public:
	SSDRecon(void);
	vtkSmartPointer<vtkPolyData> createSSD(vtkSmartPointer<vtkPolyData> inputPolyData, OptionsSSD* options);
private:
};

#endif