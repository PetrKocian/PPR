#pragma once
#include "../utils/stats.h"
#include <vector>


Stats read_and_analyze_file_naive(std::string filename);
Stats compute_stats_naive(std::vector<char> buffer);
