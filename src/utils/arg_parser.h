#pragma once
#include <string>
#include <CL/cl.hpp>

void parse_arguments(int argc, char* argv[], std::vector<cl::Device> &devices);