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

Numbers read_and_analyze_file(std::string filename)
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
			
			double mean_temp = 0;
			__m128d a = _mm256_extractf128_pd(vec, 0);
			__m128d b = _mm256_extractf128_pd(vec, 1);

			double means[4];
			double* p = means;
			_mm_storel_pd(p, a);
			_mm_storeh_pd(p + 1, a);
			_mm_storel_pd(p + 2, b);
			_mm_storeh_pd(p + 3, b);

			stats.push(means[0]);
			stats.push(means[1]);
			stats.push(means[2]);
			stats.push(means[3]);

			//_mm256_stream_pd(means, m1_v);
			for (int j = 0; j < 4; j++)
			{
				std::wcout << "vec" << j << ": " << means[j] <<"   num: "<< double_values[i+j] << std::endl;
				mean_temp += means[j];
			}






			const double number = double_values[i];
			//test[i] = number * 2;
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
	std::cout << "mean " << stats.mean() << std::endl;
	std::cout << "mean_v " << stats.mean_v() << std::endl;
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

			std::cout << "num " << number << std::endl;
			auto double_class = std::fpclassify(number);
			if (double_class == FP_NORMAL || FP_ZERO)
			{

				result.doubles.push_back(number);
				stats.push(number);
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

				result.doubles.push_back(number);
				stats.push(number);
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