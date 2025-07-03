#ifndef TIFF_READER_H
#define TIFF_READER_H

#include <vtkTIFFReader.h>
#include <vtkImageData.h>
#include <vtkAlgorithmOutput.h>
#include <string>

class ReadTIFF {
public:
	ReadTIFF(std::string file);

public:
	bool read();
	vtkImageData* getOutput();
	vtkAlgorithmOutput* getOutputPort();

private:
	vtkTIFFReader* p_vtkTIFFReader;
	std::string p_file;
};

#endif