#pragma once

#include <string>
#include <vector>
#include <atomic>

void read_file(std::string filename, std::vector<std::vector<char>>& opencl_v, std::vector<std::vector<char>>& cpu_v, std::atomic<bool> &finished);
