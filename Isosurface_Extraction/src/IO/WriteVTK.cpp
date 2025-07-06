#include <IO/WriteVTK.h>
#include <iostream>

WriteVTK::WriteVTK(std::string outputFile) {
	p_VTKWriter = vtkPolyDataWriter::New();
	p_outFile = outputFile;
}

bool WriteVTK::write(vtkPolyData* inputData) {
	p_VTKWriter->SetInputData(inputData);
	p_VTKWriter->SetFileName(p_outFile.c_str());
	p_VTKWriter->Update();

	std::cout << "WriteVTK: VTK File " << p_outFile << " written successfully." << std::endl;

	return true;
}