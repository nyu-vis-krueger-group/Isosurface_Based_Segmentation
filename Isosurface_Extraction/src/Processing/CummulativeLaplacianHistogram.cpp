#include <Processing/CummulativeLaplacianHistogram.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

CummulativeLaplacianHistogram::CummulativeLaplacianHistogram(vtkAlgorithmOutput* inputConnection, vtkImageData* imageData) {
	p_laplacian = vtkImageLaplacian::New();
	p_laplacian->SetInputConnection(inputConnection);
	p_laplacian->SetDimensionality(3);
	p_laplacian->Update();

	p_intensityArr = imageData->GetPointData()->GetScalars();
	p_intensityArr->Modified();
	double range[2];
	imageData->GetScalarRange(range);
	p_scalarMin = static_cast<int>(std::floor(range[0]));;
	p_scalarMax = static_cast<int>(std::ceil(range[1]));
}

std::unordered_map<std::string, std::vector<double>> CummulativeLaplacianHistogram::calculate() {
	// calculate laplacian
	p_laplacianArr = p_laplacian->GetOutput()->GetPointData()->GetScalars();
	p_laplacianArr->Modified();

	// create histogram for all intensities
	std::unordered_map<std::string, std::vector<double>> histogram;
	histogram["Value"] = std::vector<double>(p_scalarMax - p_scalarMin + 1, 0.0);
	histogram["Intensity"] = std::vector<double>(p_scalarMax - p_scalarMin + 1, 0.0);

	// laplacian histogram
	for (int i = 0; i < p_laplacianArr->GetNumberOfTuples(); ++i) {
		double rawVal = p_intensityArr->GetComponent(i, 0);
		int bin = static_cast<int>(std::round(rawVal)) - p_scalarMin;
		if (bin >= 0 && bin < histogram["Value"].size()) {
			histogram["Intensity"][bin] = bin + p_scalarMin;
			histogram["Value"][bin] -= p_laplacianArr->GetComponent(i, 0);
		}
	}
	// aggregating from the end
	for (size_t i = histogram["Value"].size() - 1; i > 0; --i)
		histogram["Value"][i - 1] += histogram["Value"][i];

	// calculating the knee isovalue
	size_t N = histogram["Value"].size();
	double x0 = p_scalarMin, y0 = histogram["Value"][0];
	double x1 = p_scalarMax, y1 = histogram["Value"][N - 1];
	double dx = x1 - x0, dy = y1 - y0;
	double norm = std::hypot(dy, dx);


	int bestIdx = 0;
	double bestDist = -1;
	for (int i = 0; i < N; ++i) {
		double xi = p_scalarMin + i;
		double yi = histogram["Value"][i];
		double num = std::abs(dy * xi - dx * yi + x1 * y0 - y1 * x0);
		double d = num / norm;
		if (d > bestDist) {
			bestDist = d;
			bestIdx = i;
		}
	}

	p_kneeIsovalue = p_scalarMin + bestIdx;

	return histogram;
}

int CummulativeLaplacianHistogram::getKneeIsovalue() {
	return p_kneeIsovalue;
}