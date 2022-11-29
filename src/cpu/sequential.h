#pragma once
#include <vector>
#include <atomic>
#include "../utils/stats.h"
#include "../utils/watchdog.h"

void cpu_manager(std::vector<std::vector<char>>& cpu_buffer, Stats& result, std::atomic<bool>& finished, Watchdog& dog);
Stats compute_stats_v(std::vector<char> buffer);
