#include <IO/WriteCSV.h>
#include <IO/ReadTIFF.h>
#include <IsosurfaceExtraction/FlyingEdges3D.h>
#include <IO/WriteOBJ.h>
#include <Processing/CummulativeLaplacianHistogram.h>
#include <chrono>

int main(int argc, char* argv[]) {
	std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

	std::cout << "Isosurface Extractor: Finds an optimal isovalue, extracts that contour, and saves it as an OBJ.\n" << std::endl;
	std::cout << "Arg 1: input TIFF file, Arg 2 (Optional): 1, if you save the laplacian histogram as csv.\n" << std::endl;

	std::string inputTIFF = argv[1];

	// read input TIFF file
	std::cout << "Reading TIFF " << inputTIFF <<"." << std::endl;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	ReadTIFF tiffReader(inputTIFF);
	tiffReader.read();
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Finished in " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0 << "seconds.\n" << std::endl;

	// laplacian histogram 
	std::cout << "Creating end-cummulative laplacian histogram." << std::endl;
	begin = std::chrono::steady_clock::now();
	CummulativeLaplacianHistogram histogramCalculator(tiffReader.getOutputPort(), tiffReader.getOutput());
	std::unordered_map<std::string, std::vector<double>> histogram = histogramCalculator.calculate();
	int isovalue = histogramCalculator.getKneeIsovalue();
	end = std::chrono::steady_clock::now();
	std::cout << "Finished in " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0 << "seconds.\n" << std::endl;

	// writing laplacian histogram as csv
	if (argc > 2 && std::stoi(argv[2])==1) {
		std::cout << "Writing laplacian histogram as CSV." << std::endl;
		begin = std::chrono::steady_clock::now();
		WriteCSV<double> writer("hisotgram.csv");
		writer.write(histogram);
		end = std::chrono::steady_clock::now();
		std::cout << "Finished in " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0 << "seconds.\n" << std::endl;
	}

	// flying edges 3d to extract contour
	std::cout << "Extracting contours using FlyingEdges3D." << std::endl;
	begin = std::chrono::steady_clock::now();
	FlyingEdges3D contourExtractor(tiffReader.getOutputPort());
	std::cout << "Detected isovalue: " << isovalue << std::endl;
	contourExtractor.setIsoValue(0, isovalue);
	vtkPolyData* contour = contourExtractor.getOutput();
	end = std::chrono::steady_clock::now();
	std::cout << "Finished in " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0 << "seconds.\n" << std::endl;

	// Write the extracted contour as OBJ
	std::cout << "Extracting contours using FlyingEdges3D." << std::endl;
	begin = std::chrono::steady_clock::now();
	WriteOBJ objWriter(inputTIFF+"_output.obj");
	objWriter.write(contourExtractor.getOutput());
	end = std::chrono::steady_clock::now();
	std::cout << "Finished in " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0 << "seconds.\n" << std::endl;

	std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();

	std::cout << "Isosurface Extractor: Finished.\n" << std::endl;
	std::cout << "Total Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() / 1000000.0 << "seconds.\n" << std::endl;

	return 0;
}