#include <IO/WritePLY.h>
#include <iostream>

WritePLY::WritePLY(std::string outputFile) {
	p_PLYWriter = vtkPLYWriter::New();
	p_outFile = outputFile;
}

bool WritePLY::write(vtkPolyData* inputData) {
	p_PLYWriter->SetFileTypeToBinary();
	p_PLYWriter->SetInputData(inputData);
	p_PLYWriter->SetFileName(p_outFile.c_str());
	p_PLYWriter->Update();

	std::cout << "WritePLY: PLY File " << p_outFile << " written successfully." << std::endl;

	return true;
}
