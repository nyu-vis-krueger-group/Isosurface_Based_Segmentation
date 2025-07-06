#ifndef WRITE_VTK_H
#define WRITE_VTK_H

#include <vtkPolyDataWriter.h>
#include <vtkPolyData.h>
#include <string>

class WriteVTK {
public:
	WriteVTK(std::string outputFile);
public:
	bool write(vtkPolyData* inputData);

private:
	vtkPolyDataWriter* p_VTKWriter;
	std::string p_outFile;
};

#endif