#include "file_reader.h"
#include "utils.h"
#include <vector>
#include <fstream>
#include <iostream>

#define NUMBER_OF_DOUBLES 10000
#define NUMBER_OF_DOUBLES_CL 1000000

std::mutex cpu_buffer_mutex;
std::mutex cl_buffer_mutex;
void read_file(std::string filename, std::vector<std::vector<char>> &opencl_v, std::vector<std::vector<char>> &cpu_v, std::atomic<bool> &finished)
{

	std::ifstream input_file(filename, std::ifstream::in | std::ifstream::binary);
	bool eof = false;
	size_t buffer_size_cl = sizeof(double) * NUMBER_OF_DOUBLES_CL;
	size_t buffer_size_cpu = sizeof(double) * NUMBER_OF_DOUBLES;

	std::vector<char> buffer_cpu(buffer_size_cpu);
	std::vector<char> buffer_cl(buffer_size_cl);


	//check if file opened
	if (!input_file)
	{
		std::cout << "File failed to open" << std::endl;
		eof = true;
	}

	while (!eof)
	{

		while (opencl_v.size() + cpu_v.size() > 30)
		{
			//std::cout << "CL: " << opencl_v.size() << "| CPU: " << cpu_v.size() << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			if (opencl_v.size() + cpu_v.size() < 20)
			{
				//std::cout << "BREAK" << std::endl;
				break;
			}
		}
		if (opencl_v.size() > cpu_v.size())
		{
			input_file.read(buffer_cpu.data(), buffer_size_cpu);
			if (input_file.gcount() == 0)
			{
				break;
			}

			if (input_file.gcount() != buffer_size_cpu)
			{
				buffer_cpu.resize(input_file.gcount());
			}

			cpu_buffer_mutex.lock();
			cpu_v.push_back(buffer_cpu);
			cpu_buffer_mutex.unlock();
		}
		else
		{
			input_file.read(buffer_cl.data(), buffer_size_cl);
			if (input_file.gcount() < buffer_size_cl)
			{
				buffer_cl.resize(input_file.gcount());
				cpu_buffer_mutex.lock();
				cpu_v.push_back(buffer_cl);
				cpu_buffer_mutex.unlock();
				break;
			}
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
	finished = true;
}