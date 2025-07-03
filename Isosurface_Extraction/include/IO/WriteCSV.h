#ifndef WRITE_CSV_H
#define WRITE_CSV_H

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>


template <class T>
class WriteCSV {
public:
	WriteCSV(std::string outputFile);

public:
	bool write(const std::unordered_map<std::string, std::vector<T>>& values);

private:
	std::ofstream p_os;
	std::string p_outFile;
};

#endif