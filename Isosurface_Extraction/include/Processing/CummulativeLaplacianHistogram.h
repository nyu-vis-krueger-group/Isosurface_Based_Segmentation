#ifndef LAPLACIAN_HISTOGRAM_H
#define LAPLACIAN_HISTOGRAM_H

#include <vtkImageLaplacian.h>
#include <vtkAlgorithmOutput.h>
#include <vtkImageData.h>
#include <vtkDataArray.h>


class CummulativeLaplacianHistogram {
public:
	CummulativeLaplacianHistogram(vtkAlgorithmOutput* inputConnection, vtkImageData* imageData);
public:
	std::unordered_map<std::string, std::vector<double>> calculate();
	int getKneeIsovalue();

private:
	vtkImageLaplacian* p_laplacian;
	int p_scalarMin, p_scalarMax;
	vtkDataArray* p_intensityArr;
	vtkDataArray* p_laplacianArr;

	int p_kneeIsovalue = 0;
};

#endif