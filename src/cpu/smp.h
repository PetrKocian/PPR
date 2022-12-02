#pragma once

#include "../utils/stats.h"
#include "../utils/watchdog.h"

Stats compute_stats_tbb(std::vector<char> buffer);
Stats tbb_read_and_analyze_file(std::string filename, Watchdog &dog, Distribution& distribution);
