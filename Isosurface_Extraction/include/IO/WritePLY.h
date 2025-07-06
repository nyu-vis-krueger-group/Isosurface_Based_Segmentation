#ifndef WRITE_PLY_H
#define WRITE_PLY_H

#include <vtkPlyWriter.h>
#include <vtkPolyData.h>
#include <string>

class WritePLY {
public:
	WritePLY(std::string outputFile);
public:
	bool write(vtkPolyData* inputData);

private:
	vtkPLYWriter* p_PLYWriter;
	std::string p_outFile;
};

#endif