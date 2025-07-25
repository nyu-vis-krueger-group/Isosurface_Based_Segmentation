#ifndef SIMILARITY_H
#define SIMILARITY_H

#include <string>

class Similarity {
public:
	Similarity(std::string executable);

public:
	double calculate(std::string mesh1, std::string mesh2);

private:
	std::string p_exe;
};

#endif