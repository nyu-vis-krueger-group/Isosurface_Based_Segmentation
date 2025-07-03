#include <IsosurfaceExtraction/FlyingEdges3D.h>

FlyingEdges3D::FlyingEdges3D(vtkAlgorithmOutput* inputConnection) {
	p_surfaceExtractor = vtkFlyingEdges3D::New();
	p_surfaceExtractor->SetInputConnection(inputConnection);
	p_surfaceExtractor->Update();
	p_surfaceExtractor->Modified();
}

void FlyingEdges3D::setIsoValue(int index, double isoValue) {
	p_surfaceExtractor->SetValue(index, isoValue);
	p_surfaceExtractor->Update();
	p_surfaceExtractor->Modified();
}

vtkPolyData* FlyingEdges3D::getOutput() {
	return p_surfaceExtractor->GetOutput();
}