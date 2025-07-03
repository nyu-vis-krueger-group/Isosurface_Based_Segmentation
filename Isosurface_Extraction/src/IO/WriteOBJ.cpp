#include <IO/WriteOBJ.h>
#include <iostream>

WriteOBJ::WriteOBJ(std::string outputFile) {
	p_OBJWriter = vtkOBJWriter::New();
	p_outFile = outputFile;
}

bool WriteOBJ::write(vtkPolyData* inputData) {
	p_OBJWriter->SetInputData(inputData);
	p_OBJWriter->SetFileName(p_outFile.c_str());
	p_OBJWriter->Update();

	std::cout << "WriteOBJ: OBJ File " << p_outFile << " written successfully." << std::endl;

	return true;
}
