#include <string>
#include <fstream>
#include <iostream>
#include <array>
#include <filesystem>
#include <immintrin.h>
#include <oneapi/tbb/flow_graph.h>
#include "naive.h"
#include "../utils/stats.h"
#include "../utils/my_timer.h"
#include "../opencl/opencl_processing.h"

#define NUMBER_OF_DOUBLES 10000
static const size_t buffer_size = sizeof(double) * NUMBER_OF_DOUBLES;

//Naive implementation, used for testing
void read_and_analyze_file_naive(std::string filename)
{
	t.clear();
	t.start();

	std::ifstream input_file(filename, std::ifstream::in | std::ifstream::binary);
	bool eof = false;
	size_t buffer_size = sizeof(double) * NUMBER_OF_DOUBLES;
	std::vector<char> buffer(buffer_size);

	Stats stats;

	//check if file opened
	if (!input_file)
	{
		std::wcout << "File failed to open" << std::endl;
		eof = true;
	}

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
			}
		}
	}

	t.end();

	std::cout 
		<< "stats mean " << stats.mean() <<  " n " << stats.get_n() <<" kurtosis " << stats.kurtosis() 
		<< " only ints " << stats.get_only_ints() << " " << stats.only_integers()
		<< " time: " << t.get_time() << "us time " << t.get_time_ms() << "ms" << std::endl;

	t.clear();
}

//Naive implementation, used for testing
Stats compute_stats_naive(std::vector<char> buffer)
{
	Stats stats;

	double* double_values = (double*)buffer.data();

	//check if doubles are normal/zero and add them to result structure
	for (int i = 0; i < buffer.size()/sizeof(double); i++)
	{

		double number = double_values[i];
		auto double_class = std::fpclassify(number);
		if (double_class == FP_NORMAL || FP_ZERO)
		{
			stats.push(number);
		}
	}

	return stats;
}