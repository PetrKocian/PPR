#include <string>
#include <fstream>
#include <iostream>
#include <array>
#include <filesystem>
#include <immintrin.h>

#include "naive.h"
#include "../utils/stats.h"
#include "../utils/timer.h"

#define NUMBER_OF_DOUBLES 40

Numbers read_and_analyze_file_v(std::string filename)
{
	Numbers result;
	std::ifstream input_file(filename, std::ifstream::in | std::ifstream::binary);
	bool eof = false;
	uint16_t buffer_size = sizeof(double) * NUMBER_OF_DOUBLES;
	std::vector<char> buffer(buffer_size);
	double combined_mean;

	Stats stats;
	Stats stats_non_v;
	stats_non_v.clear();
	stats.clear();
	t.clear();

	//check if file opened
	if (!input_file)
	{
		std::wcout << "File failed to open" << std::endl;
		eof = true;
	}


	t.start();
	const uint32_t filesize = std::filesystem::file_size(filename);


	for (int i = 0; i < filesize / buffer_size; i++)
	{
		input_file.read(buffer.data(), buffer_size);
		const int end = input_file.gcount() / sizeof(double);

		const double* double_values = (double*)buffer.data();

		for (int i = 0; i < end; i += 4)
		{
			__m256d vec = _mm256_load_pd(double_values+i);
			stats.push_v(vec);
			//auto double_class = std::fpclassify(number);
			//result.doubles.push_back(number);
			//result.valid_doubles++;
			/*if (double_class == FP_NORMAL || FP_ZERO)
			{

				result.doubles.push_back(number);
				stats.push(number);
				result.valid_doubles++;
			}
			else
			{
				result.invalid++;
			}*/
		}
	}
	if (input_file.gcount() == 0)
	{
		;
	}
	else
	{
		input_file.read(buffer.data(), buffer_size);

		double* double_values = (double*)buffer.data();

		for (int i = 0; i < input_file.gcount() / sizeof(double); i++)
		{
			double number = double_values[i];

			auto double_class = std::fpclassify(number);
			if (double_class == FP_NORMAL || FP_ZERO)
			{
				stats_non_v.push(number);

				result.doubles.push_back(number);
				result.valid_doubles++;
			}
			else
			{
				result.invalid++;
			}
		}

		//TODO: make nicer
		combined_mean = (stats_non_v.mean() * result.valid_doubles + stats.mean_v() * stats.n_of_v()) / (stats.n_of_v() + result.valid_doubles);
	}

	t.end();

	std::cout << "stats mean " << combined_mean << " kurtosis " << stats.kurtosis() << " only ints " << stats.only_integers()
		<< " time: " << t.get_time() << " us time " << t.get_time_ms() <<std::endl;
	t.clear();
	return result;
}


Numbers read_and_analyze_file_naive(std::string filename)
{
	Numbers result;
	std::ifstream input_file(filename, std::ifstream::in | std::ifstream::binary);
	bool eof = false;
	uint16_t buffer_size = sizeof(double) * NUMBER_OF_DOUBLES;
	std::vector<char> buffer(buffer_size);

	Stats stats;
	stats.clear();
	t.clear();

	//check if file opened
	if (!input_file)
	{
		std::wcout << "File failed to open" << std::endl;
		eof = true;
	}


	t.start();

	while (!eof)
	{
		input_file.read(buffer.data(), buffer_size);

		//eof
		if (input_file.gcount() == 0)
		{
			break;
		}


		double* double_values = (double*)buffer.data();

		//check if doubles are normal/zero and add them to result structure
		for (int i = 0; i < input_file.gcount() / sizeof(double); i++)
		{
			double number = double_values[i];
			auto double_class = std::fpclassify(number);
			if (double_class == FP_NORMAL || FP_ZERO)
			{
				stats.push(number);

				result.doubles.push_back(number);
				result.valid_doubles++;
			}
			else
			{
				result.invalid++;
			}
		}


	}
	t.end();
	
	std::cout << "stats mean " << stats.mean() << " kurtosis " << stats.kurtosis() << " only ints " << stats.only_integers()
		<< " time: " << t.get_time() << " us time " << t.get_time_ms() << std::endl;
	t.clear();
	return result;
}