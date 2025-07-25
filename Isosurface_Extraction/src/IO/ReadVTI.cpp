#include <IO/ReadVTI.h>

ReadVTI::ReadVTI(std::string file) {
	p_vtkVTIReader = vtkXMLImageDataReader::New();
	p_file = file;
}

bool ReadVTI::read() {
	p_vtkVTIReader->SetFileName(p_file.c_str());
	p_vtkVTIReader->Update();
	return true;
}

vtkImageData* ReadVTI::getOutput() {
	return p_vtkVTIReader->GetOutput();
}

vtkAlgorithmOutput* ReadVTI::getOutputPort() {
	return p_vtkVTIReader->GetOutputPort();
}