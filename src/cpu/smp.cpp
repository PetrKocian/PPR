#include <string>
#include <fstream>
#include <iostream>
#include <array>
#include <filesystem>
#include <immintrin.h>
#include <oneapi/tbb/flow_graph.h>
#include "smp.h"
#include "../utils/utils.h"
#include "../utils/my_timer.h"
#include "../opencl/opencl_processing.h"

static const size_t buffer_size = sizeof(double) * NUMBER_OF_DOUBLES_CPU;

Stats tbb_read_and_analyze_file(std::string filename, Watchdog& dog, Distribution& distribution)
{
	tbb::flow::graph g;
	std::vector<char> buffer(buffer_size);
	Stats final_stats;

	//open file
	std::ifstream input_file(filename, std::ifstream::in | std::ifstream::binary);
	if (!input_file)
	{
		std::wcout << "File failed to open" << std::endl;
		exit(1);
	}

	//reader input node
	tbb::flow::input_node<std::vector<char>> input_node(g, [&](tbb::flow_control & fc) {

		input_file.read(buffer.data(), buffer_size);

		if (input_file.gcount() == 0)
		{
			fc.stop();
			return std::vector<char>();
		}
		return std::vector<char>(buffer.begin(), buffer.begin() + input_file.gcount());

		});
	//push function node
	tbb::flow::function_node<std::vector<char>, Stats> push_node(g, tbb::flow::unlimited, [&](std::vector<char> data) {

		Stats stats;
		const double* double_values = (double*)data.data();

		if (data.size() == buffer_size)
		{
			const size_t end = data.size() / sizeof(double);
			for (int i = 0; i < end; i += 4)
			{
				__m256d vec = _mm256_load_pd(double_values + i);
				stats.push_v(vec);
			}
			//add AVX2 vector doubles together to finalize stats
			stats.finalize_stats();
			distribution.push_distribution(static_cast<distr_type>(stats.get_distribution_s()));
			dog.kick(stats.get_n());
			return stats;
		}
		else
		//end of file -> might not be divisible by 4, use push instead of push_v
		{
			const size_t end = data.size() / sizeof(double);
			for (int i = 0; i < end; i++)
			{
				double number = double_values[i];
				stats.push(number);
			}

			stats.finalize_stats();
			return stats;
		}
		});

	//merger function node, 1 instance since it's modifying global final_stats
	tbb::flow::function_node<Stats, Stats> combine_node(g, 1, [&](Stats stats) {
		final_stats.add_stats(stats);
		return final_stats;
		});
	//connect nodes
	tbb::flow::make_edge(input_node, push_node);
	tbb::flow::make_edge(push_node, combine_node);

	//activate and wait for graph to finish, return result
	input_node.activate();
	g.wait_for_all();
	final_stats.finalize_stats();

	return final_stats;
}
