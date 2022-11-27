#pragma once

#include <string>
#include <vector>
#include "../utils/stats.h"
#include "opencl_utils.h"

void prepare_opencl_device(cl::Device device, Device_opencl_struct& device_struct);
Stats compute_stats_opencl(Device_opencl_struct& dev, std::vector<char> buffer);
void test_vadd(std::string filename);