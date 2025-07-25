#include <Similarity.h>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

Similarity::Similarity(std::string executable) {
    p_exe = executable;
}

double Similarity::calculate(std::string mesh1, std::string mesh2) {
    std::string cmd = p_exe + " -mesh1 " + mesh1 + " -mesh2 " + mesh2;
    std::cout << cmd << std::endl;
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return std::stod(result.substr(0, result.size()-1));
}