#include <IO/ReadTIFF.h>
#include <Processing/CummulativeLaplacianHistogram.h>
#include <windows.h>

/*
* TODO:
* Run in two modes: 
* Mode 1: Only detect isovalue and output (right now)
* Mode 2: Detect isovalue and extract isosurface (https://github.com/Chahat08/BioVis25/blob/7a4f0026de65947eb99365f1d88b03311ebd3d1e/Isosurface_Extraction/main.cpp)
*/
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