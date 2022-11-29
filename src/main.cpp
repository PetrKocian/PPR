#include <iostream>
#include <string>
#include <fstream>
#include "opencl/opencl_utils.h"
#include "naive/naive.h"
#include "utils/watchdog.h"
#include "utils/arg_parser.h"
#include "utils/my_timer.h"
#include "utils/utils.h"
#include "utils/file_reader.h"
#include "cpu/smp.h"
#include "cpu/sequential.h"
#include "opencl/opencl_processing.h"

int main(int argc, char* argv[]) {
	mode mode;
	Stats result_cl, result_cpu;
	std::string filename;
	std::vector<cl::Device> devices;
	Device_opencl_struct dev_struct;
	std::atomic<bool> finished = false;
	std::vector<std::vector<char>> cpu_buffer;
	std::vector<std::vector<char>> opencl_buffer;

	//init watchdog
	Watchdog dog(std::chrono::milliseconds(5000));

	//parse args - returns opencl devices and desired mode or exits in case of wrong args
	parse_arguments(argc, argv, devices, filename, mode);

	//start watchdog
	dog.start();

	if (mode == smp)
	{
		//start smp computation
		result_cpu = tbb_read_and_analyze_file("../../ppr_data/" + filename, dog);
	}
	else if(mode == opencl)
	{
		//start reader thread
		std::thread reader(read_file, "../../ppr_data/" + filename, std::ref(opencl_buffer), std::ref(cpu_buffer), std::ref(finished), mode);

		//start opencl manager thread for each device
		std::vector<std::thread> open_cl_threads;
		for (cl::Device& dev : devices)
		{
			open_cl_threads.push_back(std::thread(cl_manager, std::ref(opencl_buffer), std::ref(result_cl), std::ref(finished), std::ref(dog), std::ref(dev)));
		}
		for (auto &thread : open_cl_threads)
		{
			thread.join();
		}

		//start cpu manager thread to compute "leftover" data chunk which wasn't big enough for opencl
		std::thread cpu_t(cpu_manager, std::ref(cpu_buffer), std::ref(result_cpu), std::ref(finished), std::ref(dog));
		cpu_t.join();

		reader.join();

		//merge results
		result_cpu.add_stats(result_cl);

	}
	else
	{
		//start reader thread
		std::thread reader(read_file, "../../ppr_data/" + filename, std::ref(opencl_buffer), std::ref(cpu_buffer), std::ref(finished), mode);

		std::vector<std::thread> open_cl_threads;

		//start opencl manager thread for each device and a cpu thread
		std::thread cpu_t(cpu_manager, std::ref(cpu_buffer), std::ref(result_cpu), std::ref(finished), std::ref(dog));
		for (cl::Device &dev: devices)
		{
			open_cl_threads.push_back(std::thread(cl_manager, std::ref(opencl_buffer), std::ref(result_cl), std::ref(finished), std::ref(dog), std::ref(dev)));
		}
		for (auto& thread : open_cl_threads)
		{
			thread.join();
		}
		cpu_t.join();
		reader.join();
		//merge stats
		result_cpu.add_stats(result_cl);
	}
	
	//stop watchdog
	dog.stop();

	std::cout << std::endl << "FINAL ints " << result_cpu.get_only_ints() << std::endl;
	std::cout << std::endl << "FINAL kurt " << result_cpu.kurtosis() << std::endl;
	std::cout << std::endl << "FINAL count " << result_cpu.get_n() << std::endl;

	return 0;
}