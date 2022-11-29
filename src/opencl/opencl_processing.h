#pragma once

#include <string>
#include <vector>
#include <atomic>
#include "../utils/stats.h"
#include "opencl_utils.h"
#include "../utils/watchdog.h"

void cl_manager(std::vector<std::vector<char>>& cl_buffer, Stats& result, std::atomic<bool>& finished, Watchdog& dog, cl::Device& device, Distribution &distribution);
void prepare_opencl_device(cl::Device device, Device_opencl_struct& device_struct);
Stats compute_stats_opencl(Device_opencl_struct& dev, std::vector<char> buffer);
