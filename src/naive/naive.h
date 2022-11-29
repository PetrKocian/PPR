#pragma once
#include "../utils/stats.h"
#include <vector>


void read_and_analyze_file_v(std::string filename);
void read_and_analyze_file_naive(std::string filename);
Stats compute_stats_naive(std::vector<char> buffer);
