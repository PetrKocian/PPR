#include <string>
#include <fstream>
#include <iostream>
#include <array>
#include <filesystem>
#include <immintrin.h>
#include "sequential.h"
#include "../utils/stats.h"
#include "../utils/my_timer.h"
#include "../utils/utils.h"

//function passed to a cpu manager thread
void cpu_manager(std::vector<std::vector<char>>& cpu_buffer, Stats& result, std::atomic<bool>& finished, Watchdog& dog, Distribution &distribution)
{
	//local variables
	std::vector<char> buffer;
	Stats stats;
	bool get_data = true;

	//Stats result;
	while (get_data)
	{
		cpu_buffer_mutex.lock();
		if (!cpu_buffer.empty())
		{
			//pop data from buffer if its not empty
			buffer = cpu_buffer.back();
			cpu_buffer.pop_back();
			cpu_buffer_mutex.unlock();
			//cast char buffer to doubles
			double* double_values = (double*)buffer.data();
			//compute and update result stats
			stats = compute_stats_v(buffer);
			result.add_stats(stats);
			distribution.push_distribution(static_cast<distr_type>(stats.get_distribution_s()));
			//kick watchdog
			dog.kick(stats.get_n());
		}
		else
		{
			//break loop, release mutex and finish thread
			if (finished)
			{
				get_data = false;
			}
			cpu_buffer_mutex.unlock();
		}
	}
}


Stats compute_stats_v(std::vector<char> buffer)
{
	//cast char buffer to doubles
	const double* double_values = (double*)buffer.data();
	bool doubles_valid = true;
	double only_int = 0;
	uint16_t index = 0;
	Stats stats;

	//TODO: correct if not div4
	//iterate over data in multiples of 4 - manually vectorized loop
	for (int i = 0; i < buffer.size()/sizeof(double); i += 4)
	{
		doubles_valid = true;
		//check if number is valid and compute only_int
		for (int j = 0; j < 4; j++)
		{
			double number = double_values[i];
			int double_class = std::fpclassify(number);
			only_int = number - std::floor(number);
			if (double_class != FP_NORMAL || FP_ZERO)
			{
				index = j;
				doubles_valid = false;
			}
		}

		//if some of the numbers are invalid do a non-vectorized push for each valid
		if (doubles_valid == false)
		{
			for (int j = 0; j < 4; j++)
			{
				if (j != index)
				{
					double number = double_values[i];
					stats.push(number);
				}
			}
			continue;
		}

		//vectorized push
		__m256d vec = _mm256_load_pd(double_values + i);
		stats.push_v(vec);
		stats.push_only_int(only_int);
	}

	//finalize and return stats
	stats.finalize_stats();
	return stats;
}