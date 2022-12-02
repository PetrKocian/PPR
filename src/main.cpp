#include <iostream>
#include <string>
#include <fstream>
#include "opencl/opencl_utils.h"
#include "utils/watchdog.h"
#include "utils/arg_parser.h"
#include "utils/my_timer.h"
#include "utils/utils.h"
#include "utils/file_reader.h"
#include "cpu/smp.h"
#include "cpu/sequential.h"
#include "opencl/opencl_processing.h"

int main(int argc, char* argv[]) {
	std::cout << "readers:" << NUMBER_OF_READERS << " | cpus:" << NUMBER_OF_CPU_T << std::endl;
	t.clear();
	t.start();

	mode mode;
	Stats result_cl, result_cpu;
	std::string filename;
	std::vector<cl::Device> devices;
	Device_opencl_struct dev_struct;
	std::atomic<int> finished = NUMBER_OF_READERS;
	std::vector<std::vector<char>> cpu_buffer;
	std::vector<std::vector<char>> opencl_buffer;
	Distribution distribution;

	//init watchdog
	Watchdog dog(std::chrono::milliseconds(5000), distribution);

	//parse args - returns opencl devices and desired mode or exits in case of wrong args
	parse_arguments(argc, argv, devices, filename, mode);

	//start watchdog
	dog.start();


	if (mode == sequential)
	{
		//start sequential computation
		result_cpu = read_and_analyze_file_v("../../ppr_data/" + filename, distribution, dog);
	}
	else if (mode == smp)
	{
		std::vector<std::thread> reader_threads;
		std::vector<std::thread> cpu_threads;

		//start reader threads
		for (int i = 0; i < NUMBER_OF_READERS; i++)
		{
			reader_threads.push_back(std::thread(read_file, "../../ppr_data/" + filename, std::ref(opencl_buffer), std::ref(cpu_buffer), std::ref(finished), mode, i));
		}

		//start cpu manager threads
		for (int i = 0; i < NUMBER_OF_CPU_T; i++)
		{
			cpu_threads.push_back(std::thread(cpu_manager, std::ref(cpu_buffer), std::ref(result_cpu), std::ref(finished), std::ref(dog), std::ref(distribution)));
		}

		//wait for all threads to finish
		for (auto& cpu_t : cpu_threads)
		{
			cpu_t.join();
		}
		for (auto& reader : reader_threads)
		{
			reader.join();
		}
	}
	else if(mode == opencl)
	{
		std::vector<std::thread> reader_threads;
		std::vector<std::thread> open_cl_threads;

		//start reader threads
		for (int i = 0; i < NUMBER_OF_READERS; i++)
		{
			reader_threads.push_back(std::thread(read_file, "../../ppr_data/" + filename, std::ref(opencl_buffer), std::ref(cpu_buffer), std::ref(finished), mode, i));
		}

		//start opencl manager thread for each device
		for (cl::Device& dev : devices)
		{
			open_cl_threads.push_back(std::thread(cl_manager, std::ref(opencl_buffer), std::ref(result_cl), std::ref(finished), std::ref(dog), std::ref(dev),std::ref(distribution)));
		}
		for (auto &cl_thread : open_cl_threads)
		{
			cl_thread.join();
		}

		//start cpu manager thread to compute "leftover" data chunk which wasn't big enough for opencl
		std::thread cpu_t(cpu_manager, std::ref(cpu_buffer), std::ref(result_cpu), std::ref(finished), std::ref(dog), std::ref(distribution));
		cpu_t.join();

		for (auto& reader : reader_threads)
		{
			reader.join();
		}

		//merge results
		result_cpu.add_stats(result_cl);
	}
	else
	{
		std::vector<std::thread> reader_threads;
		std::vector<std::thread> cpu_threads;
		std::vector<std::thread> open_cl_threads;

		//start reader threads
		for (int i = 0; i < NUMBER_OF_READERS; i++)
		{
			reader_threads.push_back(std::thread(read_file, "../../ppr_data/" + filename, std::ref(opencl_buffer), std::ref(cpu_buffer), std::ref(finished), mode, i));
		}
		
		//start cpu manager threads
		for (int i = 0; i < NUMBER_OF_CPU_T; i++)
		{
			cpu_threads.push_back(std::thread(cpu_manager, std::ref(cpu_buffer), std::ref(result_cpu), std::ref(finished), std::ref(dog), std::ref(distribution)));
		}
		
		//start opencl manager thread for each device and a cpu thread
		for (cl::Device &dev: devices)
		{
			open_cl_threads.push_back(std::thread(cl_manager, std::ref(opencl_buffer), std::ref(result_cl), std::ref(finished), std::ref(dog), std::ref(dev), std::ref(distribution)));
		}

		//wait for all threads to finish before finishing computation
		for (auto& cl_thread : open_cl_threads)
		{
			cl_thread.join();
		}
		for (auto& cpu_t : cpu_threads)
		{
			cpu_t.join();
		}
		for (auto& reader : reader_threads)
		{
			reader.join();
		}

		//merge stats
		result_cpu.add_stats(result_cl);
	}

	//stop watchdog
	dog.stop();

	//decide distribution and print decision
	distribution.make_distribution_decision();
	distribution.print_distribution_decision();

	t.end();
	t.print();

	if (mode == debug)
	{
		std::getline(std::cin, filename);
	}

	return 0;
}