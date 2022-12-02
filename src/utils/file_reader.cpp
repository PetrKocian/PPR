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
	size_t buffer_size_cl = sizeof(double) * NUMBER_OF_DOUBLES_CL;
	size_t buffer_size_cpu = sizeof(double) * NUMBER_OF_DOUBLES_CPU;
	std::vector<char> buffer_cpu(buffer_size_cpu);
	std::vector<char> buffer_cl(buffer_size_cl);

	std::vector<int> cl_c, cpu_c;

	//check if file opened
	if (!input_file)
	{
		std::cout << "File failed to open, exiting" << std::endl;
		std::terminate();
	}



	const uintmax_t filesize = std::filesystem::file_size(filename);
	uintmax_t current_size = 0;
	input_file.seekg((((filesize / NUMBER_OF_READERS) * reader_index)/buffer_size_cl)*buffer_size_cl);
	cl_buffer_mutex.lock();
	std::cout << "reader " << reader_index << " filesize " << filesize << " seekg " << (filesize / NUMBER_OF_READERS) * reader_index << " tellg " << (filesize / NUMBER_OF_READERS) * (reader_index + 1) << std::endl;
	cl_buffer_mutex.unlock();

	while (true)
	{
		cpu_buffer_mutex.lock();

		std::cout << "reader " << reader_index << "tellg " << input_file.tellg() << " =? " << (filesize / NUMBER_OF_READERS) * (reader_index + 1) << std::endl;;
		cpu_buffer_mutex.unlock();


		if (input_file.tellg()==((((filesize / NUMBER_OF_READERS) * (reader_index+1)) / buffer_size_cl) * buffer_size_cl) && (reader_index != (NUMBER_OF_READERS - 1)))
		{
			break;
		}

		//control memory usage
		while (opencl_v.size() + cpu_v.size() > 100)
		{
			//std::cout << "OVERFLOW" << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			if (opencl_v.size() + cpu_v.size() < 60)
			{
				break;
			}
		}
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));

		cl_c.push_back(opencl_v.size());
		cpu_c.push_back(cpu_v.size());
		//std::cout << "CL: " << opencl_v.size() << "| CPU: " << cpu_v.size() << std::endl;


		//don't load data into cpu buffer if using only opencl devices, and load data only in cpu buffer if in SMP or seq mode
		if (mode != opencl && opencl_v.size() > cpu_v.size() || mode == smp || mode == sequential)
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
		
		current_size += input_file.gcount();

		//std::cout << "curr " << current_size << " fs/10 " << (filesize / 10) << std::endl;
		//std::cout << "CL: " << opencl_v.size() << "| CPU: " << cpu_v.size() << std::endl;

	


		/*if (current_size >= (filesize / NUMBER_OF_READERS) && (reader_index != (NUMBER_OF_READERS-1)))
		{
			//std::cout << "BREAK1" << std::endl;
			break;
		}*/

		//eof
		if (input_file.gcount() == 0)
		{
			//std::cout << "BREAK2" << std::endl;
			break;
		}	
	}
	//set finished flag
	finished--;

	for (int i = 0; i < cpu_c.size(); i++)
	{
		//std::cout << "CL: " << cl_c.at(i) << "| CPU: " << cpu_c.at(i) << std::endl;
	}
}

