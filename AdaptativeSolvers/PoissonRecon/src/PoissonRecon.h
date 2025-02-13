#ifdef POISSONRECON_EXPORTS
#define POISSONRECON_API __declspec(dllexport)
#else
#define POISSONRECON_API __declspec(dllimport)
#endif

#ifndef __POISSONRECON__H__
#define __POISSONRECON__H__

#include <vector>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkCellArray.h>
#include <vtkTriangle.h>

//Used to define the reconstruction options
struct OptionsPoisson
{
	OptionsPoisson() {}
	OptionsPoisson(int depth, int boundary)
	{
		this->depth = depth;
		this->boundary = boundary;
	}
	int depth = 8;
	int boundary = 2;
};

class POISSONRECON_API PoissonRecon {
public:
	PoissonRecon(void);
	vtkSmartPointer<vtkPolyData> createPoisson(vtkSmartPointer<vtkPolyData> inputPolyData, OptionsPoisson* options);
private:
};

#endif