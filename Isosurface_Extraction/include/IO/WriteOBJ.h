#ifndef WRITE_OBJ_H
#define WRITE_OBJ_H

#include <vtkOBJWriter.h>
#include <vtkPolyData.h>
#include <string>

class WriteOBJ {
public:
	WriteOBJ(std::string outputFile);

public:
	bool write(vtkPolyData* inputData);

private:
	vtkOBJWriter* p_OBJWriter;
	std::string p_outFile;
};

#endif