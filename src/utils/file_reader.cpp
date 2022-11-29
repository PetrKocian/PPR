#include "file_reader.h"
#include <vector>
#include <fstream>
#include <iostream>

//mutexes to protect shared buffers
std::mutex cpu_buffer_mutex;
std::mutex cl_buffer_mutex;

void read_file(std::string filename, std::vector<std::vector<char>> &opencl_v, std::vector<std::vector<char>> &cpu_v, std::atomic<bool> &finished, mode mode)
{
	//open file
	std::ifstream input_file(filename, std::ifstream::in | std::ifstream::binary);

	//local declarations
	size_t buffer_size_cl = sizeof(double) * NUMBER_OF_DOUBLES_CL;
	size_t buffer_size_cpu = sizeof(double) * NUMBER_OF_DOUBLES_CPU;
	std::vector<char> buffer_cpu(buffer_size_cpu);
	std::vector<char> buffer_cl(buffer_size_cl);

	//check if file opened
	if (!input_file)
	{
		std::cout << "File failed to open, exiting" << std::endl;
		std::terminate();
	}

	while (true)
	{
		//control memory usage
		while (opencl_v.size() + cpu_v.size() > 30)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			if (opencl_v.size() + cpu_v.size() < 20)
			{
				break;
			}
		}
		//don't load data into cpu buffer if using only opencl devices, and load data only in cpu buffer if in SMP mode
		if (mode != opencl && opencl_v.size() > cpu_v.size() || mode == smp)
		{
			input_file.read(buffer_cpu.data(), buffer_size_cpu);

			//stop reading if end of file
			if (input_file.gcount() == 0)
			{
				break;
			}

			//resize vector for last read elements
			if (input_file.gcount() != buffer_size_cpu)
			{
				buffer_cpu.resize(input_file.gcount());
			}

			//push buffer in cpu buffer vector
			cpu_buffer_mutex.lock();
			cpu_v.push_back(buffer_cpu);
			cpu_buffer_mutex.unlock();
		}
		else
		{
			input_file.read(buffer_cl.data(), buffer_size_cl);

			//if last read count is smaller then opencl buffer size, load into cpu buffer
			if (input_file.gcount() < buffer_size_cl)
			{
				buffer_cl.resize(input_file.gcount());
				cpu_buffer_mutex.lock();
				cpu_v.push_back(buffer_cl);
				cpu_buffer_mutex.unlock();
				break;
			}

			//push buffer in opencl buffer vector
			cl_buffer_mutex.lock();
			opencl_v.push_back(buffer_cl);
			cl_buffer_mutex.unlock();
		}
		
		//eof
		if (input_file.gcount() == 0)
		{
			break;
		}	
	}
	//set finished flag
	finished = true;
}