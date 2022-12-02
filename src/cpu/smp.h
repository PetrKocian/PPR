#pragma once

#include "../utils/stats.h"
#include "../utils/watchdog.h"

Stats tbb_read_and_analyze_file(std::string filename, Watchdog &dog, Distribution& distribution);
