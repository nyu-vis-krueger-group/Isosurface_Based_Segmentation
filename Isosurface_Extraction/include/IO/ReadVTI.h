#ifndef VTI_READER_H
#define VTI_READER_H

#include <vtkXMLImageDataReader.h>
#include <vtkImageData.h>
#include <vtkAlgorithmOutput.h>
#include <string>

class ReadVTI {
public:
	ReadVTI(std::string file);

public:
	bool read();
	vtkImageData* getOutput();
	vtkAlgorithmOutput* getOutputPort();

private:
	vtkXMLImageDataReader* p_vtkVTIReader;
	std::string p_file;
};

#endif