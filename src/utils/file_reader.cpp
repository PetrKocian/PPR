#include "file_reader.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>

//mutexes to protect shared buffers
std::mutex cpu_buffer_mutex;
std::mutex cl_buffer_mutex;

void read_file(std::string filename, std::vector<std::vector<char>> &opencl_v, std::vector<std::vector<char>> &cpu_v, std::atomic<int> &finished, mode mode, int reader_index)
{
	//open file
	std::ifstream input_file(filename, std::ifstream::in | std::ifstream::binary);

	//local declarations
	size_t buffer_size = sizeof(double) * NUMBER_OF_DOUBLES;
	std::vector<char> buffer_cpu(buffer_size);
	std::vector<char> buffer_cl(buffer_size);

	//check if file opened
	if (!input_file)
	{
		std::cout << "File failed to open, exiting" << std::endl;
		std::terminate();
	}

	//set correct position in file for each reader thread, dividing and multiplying by buffer size to 
	const uintmax_t filesize = std::filesystem::file_size(filename);
	input_file.seekg((((filesize / NUMBER_OF_READERS) * reader_index)/buffer_size)*buffer_size);


	while (true)
	{
		//check if reader thread reached the end of its block
		if (input_file.tellg()==((((filesize / NUMBER_OF_READERS) * (reader_index+1)) / buffer_size) * buffer_size) && (reader_index != (NUMBER_OF_READERS - 1)))
		{
			break;
		}

		//control memory usage
		while (opencl_v.size() + cpu_v.size() > 30)
		{
			//std::cout << "OVERFLOW" << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			if (opencl_v.size() + cpu_v.size() < 20)
			{
				break;
			}
		}

		//don't load data into cpu buffer if using only opencl devices, and load data only in cpu buffer if in SMP or seq mode
		if (mode != opencl && opencl_v.size() > cpu_v.size() || mode == smp || mode == sequential)
		{
			input_file.read(buffer_cpu.data(), buffer_size);

			//stop reading if end of file
			if (input_file.gcount() == 0)
			{
				break;
			}

			//resize vector for last read elements
			if (input_file.gcount() != buffer_size)
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
			input_file.read(buffer_cl.data(), buffer_size);

			//if last read count is smaller then opencl buffer size, load into cpu buffer
			if (input_file.gcount() < buffer_size)
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

	//decrement counter of running reader threads
	finished--;
}

