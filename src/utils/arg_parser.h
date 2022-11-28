#pragma once
#include <string>
#include <CL/cl.hpp>
#include "utils.h"

void parse_arguments(int argc, char* argv[], std::vector<cl::Device> &devices, std::string &filename, mode &mode_ret);