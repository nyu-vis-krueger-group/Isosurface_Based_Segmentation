#ifndef ISO_EXTRACTOR_FLYING_EDGES
#define ISO_EXTRACTOR_FLYING_EDGES

#include <vtkAlgorithmOutput.h>
#include <vtkFlyingEdges3D.h>
#include <vtkPolyData.h>

class FlyingEdges3D {
public:
	FlyingEdges3D(vtkAlgorithmOutput* inputConnection);
public:
	void setIsoValue(int index, double isoValue);
	vtkPolyData* getOutput();

private:
	vtkFlyingEdges3D* p_surfaceExtractor;
};

#endif