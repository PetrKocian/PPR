#pragma once
#include <string>
#include <CL/cl.hpp>

void parse_arguments(std::string &filename, std::vector<cl::Device> &devices);