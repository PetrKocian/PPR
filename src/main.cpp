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

#include "opencl/opencl_processing.h"


void cl_wrapper(std::vector<std::vector<char>>& cl_buffer, Stats &result,std::atomic<bool>& finished, Watchdog &dog)
{
	//Stats result;

	std::cout << "OPENCL RUNNING" << std::endl;

	Device_opencl_struct dev;

	std::vector<cl::Device> all_devices = get_all_devices();

	cl::Device default_device = all_devices[0];

	prepare_opencl_device(default_device, dev);
	std::vector<char> buffer;
	Stats s;
	bool get_data = true;

	while(get_data)
	{
		cl_buffer_mutex.lock();

		if (!cl_buffer.empty())
		{
			buffer = cl_buffer.back();
			cl_buffer.pop_back();
			cl_buffer_mutex.unlock();

			s = compute_stats_opencl(dev, buffer);
			result.add_stats(s);
			dog.kick(s.get_n());
			//std::cout << "KURTOSIS opencl " << result.kurtosis() << std::endl;

		}
		else
		{
			if (cl_buffer.empty() && finished)
			{
				get_data = false;
			}
			cl_buffer_mutex.unlock();
		}
	}
}

void cpu_wrapper(std::vector<std::vector<char>> &cpu_buffer, Stats &result, std::atomic<bool> &finished, Watchdog &dog)
{
	std::vector<char> buffer;
	std::cout << "STRT " << std::endl;
	bool get_data = true;

	//Stats result;
	while(get_data)
	{
		cpu_buffer_mutex.lock();
		if (!cpu_buffer.empty())
		{
			buffer = cpu_buffer.back();
			cpu_buffer.pop_back();
			cpu_buffer_mutex.unlock();

			double* double_values = (double*)buffer.data();


			Stats s = compute_stats_naive(buffer);

			result.add_stats(s);
			
			dog.kick(s.get_n());
			//std::cout << "KURTOSIS cpu " << result.kurtosis() << std::endl;

		}
		else
		{
			if (cpu_buffer.empty()&&finished)
			{
				get_data = false;
			}
			cpu_buffer_mutex.unlock();
		}

	}


}

#define NUMBER_OF_ELEMENTS 1000000

#define DOUBLES_BUFFER_SIZE (sizeof(double)*NUMBER_OF_ELEMENTS)

int main(int argc, char* argv[]) {
	Stats result_cl, result_cpu;
	Numbers test1, test2;
	std::string line, filename;
	std::vector<cl::Device> devices;
	Device_opencl_struct dev_struct;
	std::atomic<bool> finished = false;
	mode mode;
	std::vector<std::vector<char>> cpu_buffer;
	std::vector<std::vector<char>> opencl_buffer;
	Watchdog dog(std::chrono::milliseconds(5000));

	//parse args - returns opencl devices and desired mode or exits in case of wrong args
	parse_arguments(argc, argv, devices, filename, mode);
	
	//start reader thread
	std::thread reader(read_file, "../../ppr_data/" + filename, std::ref(opencl_buffer), std::ref(cpu_buffer), std::ref(finished), mode);

	//start watchdog
	dog.start();

	if (mode == smp)
	{
		//start only smp thread
		std::cout << "smp mode" << std::endl;
		std::thread cpu_t(cpu_wrapper, std::ref(cpu_buffer), std::ref(result_cpu), std::ref(finished), std::ref(dog));
		reader.join();
		cpu_t.join();
	}
	else if(mode == opencl)
	{
		std::cout << "opencl mode" << std::endl;
		std::thread opencl_t(cl_wrapper, std::ref(opencl_buffer), std::ref(result_cl), std::ref(finished), std::ref(dog));
		opencl_t.join();

		std::thread cpu_t(cpu_wrapper, std::ref(cpu_buffer), std::ref(result_cpu), std::ref(finished), std::ref(dog));
		cpu_t.join();

		reader.join();
		result_cpu.add_stats(result_cl);
	}
	else
	{
		std::cout << "all mode" << std::endl;
		std::thread opencl_t(cl_wrapper, std::ref(opencl_buffer), std::ref(result_cl), std::ref(finished), std::ref(dog));
		std::thread cpu_t(cpu_wrapper, std::ref(cpu_buffer), std::ref(result_cpu), std::ref(finished), std::ref(dog));

		cpu_t.join();
		opencl_t.join();
		reader.join();
		result_cpu.add_stats(result_cl);
	}
	
	//stop watchdog
	dog.stop();

	std::cout << std::endl << "FINAL kurt " << result_cpu.kurtosis() << std::endl;
	std::cout << std::endl << "FINAL count " << result_cpu.get_n() << std::endl;


	test2 = read_and_analyze_file_naive("../../ppr_data/" + filename);
	//std::cout << std::endl << "FINAL KURRT " << test.kurtosis() << std::endl;



	//read_file("../../ppr_data/" + line, opencl_buffer, cpu_buffer);

	/*
	//parse arguments from input - filename and opencl devices
	parse_arguments(line, devices);

	if (devices.size() == 0)
	{
		//SMP
		std::cout << "TBB" << std::endl << std::endl;
		read_and_analyze_file_tbb("../../ppr_data/" + line);
		read_and_analyze_file_v("../../ppr_data/" + line);
	}
	else
	{
		// all/selected devices
		std::cout << "OPENCL" << std::endl << std::endl;
			t.clear();

		std::vector<char> buffer(DOUBLES_BUFFER_SIZE);
		std::ifstream input_file("../../ppr_data/" + line, std::ifstream::in | std::ifstream::binary);
		input_file.read(buffer.data(), DOUBLES_BUFFER_SIZE);

		prepare_opencl_device(devices.at(0), dev_struct);
		t.start();

		compute_stats_opencl(dev_struct, buffer);

	t.end();
	std::cout << "opencl time " << t.get_time() << std::endl;

	}
	
	std::getline(std::cin, line);

	*/

	std::getline(std::cin, line);

	//std::terminate();

	return 0;

	read_and_analyze_file_tbb("../../ppr_data/" + line);


	test1 = read_and_analyze_file_v("../../ppr_data/"+line);




	//test2 = read_and_analyze_file_naive("../../ppr_data/" + line);

	//test_vadd("../../ppr_data/" + line);


	/*Watchdog dog(std::chrono::milliseconds(5000));
	dog.start();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	dog.kick(1);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	dog.kick(1);
	dog.stop();*/

	std::wcout << "Vystup programu" << std::endl;
	//wait for enter
	std::getline(std::cin, line);

	
}