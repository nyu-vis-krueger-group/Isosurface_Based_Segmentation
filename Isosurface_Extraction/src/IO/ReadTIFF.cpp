#include <IO/ReadTIFF.h>

ReadTIFF::ReadTIFF(std::string file) {
	p_vtkTIFFReader = vtkTIFFReader::New();
	p_file = file;
}

bool ReadTIFF::read() {
	p_vtkTIFFReader->SetFileName(p_file.c_str());
	p_vtkTIFFReader->Update();
	return true;
}

vtkImageData* ReadTIFF::getOutput() {
	return p_vtkTIFFReader->GetOutput();
}

vtkAlgorithmOutput* ReadTIFF::getOutputPort() {
	return p_vtkTIFFReader->GetOutputPort();
}