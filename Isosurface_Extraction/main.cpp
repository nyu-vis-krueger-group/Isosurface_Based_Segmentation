#include <IO/ReadTIFF.h>
#include <IsosurfaceExtraction/FlyingEdges3D.h>
#include <Processing/CummulativeLaplacianHistogram.h>
#include <IO/WritePLY.h>

/*
* Usage: ./IsosurfaceExtraction.exe inputTiff.tiff mode[DETECT/EXTRACT]
* Run in two modes: 
* Mode 1: (DETECT) (default) Only detect isovalue and output (right now)
* Mode 2: (EXTRACT) Detect isovalue and extract isosurface (https://github.com/Chahat08/BioVis25/blob/7a4f0026de65947eb99365f1d88b03311ebd3d1e/Isosurface_Extraction/main.cpp)
*/
int main(int argc, char* argv[]) {

	std::string inputTIFF = argv[1];

	// read input tiff
	ReadTIFF tiffReader(inputTIFF);
	tiffReader.read();


	// detect isovalue
	CummulativeLaplacianHistogram histogramCalculator(tiffReader.getOutputPort(), tiffReader.getOutput());
	std::unordered_map<std::string, std::vector<double>> histogram = histogramCalculator.calculate();
	int isovalue = histogramCalculator.getKneeIsovalue();

	std::cout << isovalue;

	std::string mode = argc > 2 ? argv[2] : "DETECT";
	if (!mode.compare("EXTRACT")) {
		// extract contour
		FlyingEdges3D contourExtractor(tiffReader.getOutputPort());
		contourExtractor.setIsoValue(0, isovalue);
		vtkPolyData* contour = contourExtractor.getOutput();

		// Write the extracted contour as OBJ
		std::cout << "Writing the extracted contours to " + inputTIFF + "_output.ply" << std::endl;
		WritePLY plyWriter(inputTIFF + "_output.ply");
		plyWriter.write(contourExtractor.getOutput());
	}

	return 0;
}