#include <IO/ReadTIFF.h>
#include <Processing/CummulativeLaplacianHistogram.h>
#include <windows.h>

int main(int argc, char* argv[]) {
	std::string inputVTI = argv[1];

	ReadTIFF tiffReader(inputVTI);
	tiffReader.read();

	CummulativeLaplacianHistogram histogramCalculator(tiffReader.getOutputPort(), tiffReader.getOutput());
	std::unordered_map<std::string, std::vector<double>> histogram = histogramCalculator.calculate();
	int isovalue = histogramCalculator.getKneeIsovalue();

	std::cout << isovalue;

	return 0;
}