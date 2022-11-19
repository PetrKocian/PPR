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

#define NUMBER_OF_DOUBLES 100
static const size_t buffer_size = sizeof(double) * NUMBER_OF_DOUBLES;

void read_and_analyze_file_tbb(std::string filename)
{
	t.clear();
	t.start();
	tbb::flow::graph g;
	std::vector<char> buffer(buffer_size);
	Stats final_stats;




	std::ifstream input_file(filename, std::ifstream::in | std::ifstream::binary);
	if (!input_file)
	{
		std::wcout << "File failed to open" << std::endl;
	}

	tbb::flow::input_node<std::vector<char>> input_node(g, [&](tbb::flow_control & fc) {

		input_file.read(buffer.data(), buffer_size);

		if(input_file.gcount() == 0)
		{
			//std::cout << "INPUT NODE END" << std::endl;
			fc.stop();
			return std::vector<char>();
		}
		return std::vector<char>(buffer.begin(), buffer.begin() + input_file.gcount());

		});
	tbb::flow::function_node<std::vector<char>,Stats> push_node(g, tbb::flow::unlimited, [&](std::vector<char> data) {

		Stats stats;
		const double* double_values = (double*)data.data();

		if (data.size() == buffer_size)
		{
			const size_t end = data.size() / sizeof(double);
			for (int i = 0; i < end; i += 4)
			{
				__m256d vec = _mm256_load_pd(double_values + i);
				/*
				std::cout << "number_v1 " << *(double_values+i )<< std::endl;
				std::cout << "number_v2 " << *(double_values + i+1) << std::endl;
				std::cout << "number_v3 " << *(double_values + i+2 )<< std::endl;
				std::cout << "number_v4 " << *(double_values + i +3)<< std::endl;
				*/
				stats.push_v(vec);
			}
			//std::cout << "Kurt_v " << stats.kurtosis_v() << std::endl;
			stats.finalize_stats();
			return stats;
		}
		else
		{
			const size_t end = data.size() / sizeof(double);
			for (int i = 0; i < end; i++)
			{
				double number = double_values[i];
				//std::cout << "number " << number << std::endl;
				stats.push(number);
			}
			//std::cout << "Kurt " << stats.kurtosis() << std::endl;

			stats.finalize_stats();
			return stats;
		}
		});
	tbb::flow::function_node<Stats, Stats> combine_node(g, 1, [&](Stats stats) {
		final_stats.add_stats(stats);
		return final_stats;
		});
	tbb::flow::make_edge(input_node, push_node);
	tbb::flow::make_edge(push_node, combine_node);

	input_node.activate();
	g.wait_for_all();
	final_stats.finalize_stats();

	t.end();

	std::cout << "final stats k " << final_stats.kurtosis() << " mean " << final_stats.mean() << " n " <<final_stats.get_n()<<" time: " << t.get_time() << "us time: " << t.get_time_ms() << "ms" << std::endl;
}

Numbers read_and_analyze_file_v(std::string filename)
{
	t.clear();
	t.start();

	Numbers result;
	std::ifstream input_file(filename, std::ifstream::in | std::ifstream::binary);
	bool eof = false;
	uint16_t buffer_size = sizeof(double) * NUMBER_OF_DOUBLES;
	std::vector<char> buffer(buffer_size);
	double combined_mean = 0;
	//TODO: move this inside push_v
	double only_int = 0;

	Stats stats;
	Stats stats_non_v;

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

		const double* double_values = (double*)buffer.data();
		bool doubles_valid = true;

		for (int i = 0; i < end; i += 4)
		{
			doubles_valid = true;
			for (int j = 0; j < 4; j++)
			{
				double number = double_values[i];
				int double_class = std::fpclassify(number);
				only_int += number - std::floor(number);
				if (double_class != FP_NORMAL || FP_ZERO)
				{
					doubles_valid = false;
				}
			}

			if (doubles_valid == false)
			{
				for (int j = 0; j < 4; j++)
				{
					double number = double_values[i];
					stats.push(number);
				}
				continue;
			}

			__m256d vec = _mm256_load_pd(double_values + i);
			stats.push_v(vec);
		}
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

				result.doubles.push_back(number);
				result.valid_doubles++;
			}
			else
			{
				result.invalid++;
			}
		}

		//TODO: make a function
		combined_mean = (stats.mean() * result.valid_doubles + stats.mean_v() * stats.n_of_v()) / (stats.n_of_v() + result.valid_doubles);
	}

	stats.finalize_stats();

	t.end();

	std::cout << "stats mean " << stats.mean_v() << " " << combined_mean << " n " << stats.get_n()<<" kurtosis " << stats.kurtosis() << " only ints "<< only_int<< " " << stats.only_integers()
		<< " time: " << t.get_time() << "us time " << t.get_time_ms() << "ms" << std::endl;
	t.clear();
	return result;
}


Numbers read_and_analyze_file_naive(std::string filename)
{
	t.clear();
	t.start();

	Numbers result;
	std::ifstream input_file(filename, std::ifstream::in | std::ifstream::binary);
	bool eof = false;
	uint16_t buffer_size = sizeof(double) * NUMBER_OF_DOUBLES;
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

	std::cout << "stats mean " << stats.mean() <<  " n " << stats.get_n() <<" kurtosis " << stats.kurtosis() << " only ints " << stats.only_integers()
		<< " time: " << t.get_time() << "us time " << t.get_time_ms() << "ms" << std::endl;
	t.clear();
	return result;
}