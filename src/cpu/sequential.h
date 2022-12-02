#pragma once
#include <vector>
#include <atomic>
#include "../utils/stats.h"
#include "../utils/watchdog.h"
#include "smp.h"

void cpu_manager(std::vector<std::vector<char>>& cpu_buffer, Stats& result, std::atomic<int>& finished, Watchdog& dog, Distribution &distribution);
Stats compute_stats_v(std::vector<char> buffer);
Stats read_and_analyze_file_v(std::string filename, Distribution& distribution, Watchdog &dog);
