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
void cpu_manager(std::vector<std::vector<char>>& cpu_buffer, Stats& result, std::atomic<int>& finished, Watchdog& dog, Distribution &distribution)
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
			if (finished == 0)
			{
				get_data = false;
			}
			cpu_buffer_mutex.unlock();
			//std::this_thread::sleep_for(std::chrono::milliseconds(100));
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


Stats read_and_analyze_file_v(std::string filename, Distribution& distribution, Watchdog &dog)
{
	std::ifstream input_file(filename, std::ifstream::in | std::ifstream::binary);
	bool eof = false;
	size_t buffer_size = sizeof(double) * NUMBER_OF_DOUBLES;
	std::vector<char> buffer(buffer_size);
	double combined_mean = 0;
	double only_int = 0;

	Stats stats;
	Stats stats_non_v;
	Stats stats_p;

	//check if file opened
	if (!input_file)
	{
		std::wcout << "File failed to open" << std::endl;
		eof = true;
	}

	const uintmax_t filesize = std::filesystem::file_size(filename);

	for (int i = 0; i < filesize / buffer_size; i++)
	{
		input_file.read(buffer.data(), buffer_size);
		const size_t end = input_file.gcount() / sizeof(double);

		stats_p = compute_stats_v(buffer);
		stats.add_stats(stats_p);
		distribution.push_distribution(static_cast<distr_type>(stats.get_distribution_s()));
		dog.kick(stats_p.get_n());
	}

	input_file.read(buffer.data(), buffer_size);

	if (input_file.gcount() != 0)
	{
		double* double_values = (double*)buffer.data();

		for (int i = 0; i < input_file.gcount() / sizeof(double); i++)
		{
			double number = double_values[i];

			auto double_class = std::fpclassify(number);
			if (double_class == FP_NORMAL || FP_ZERO)
			{
				stats.push(number);
			}
		}
	}

	stats.finalize_stats();
	return stats;
}