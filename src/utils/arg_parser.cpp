#include "arg_parser.h"
#include "../opencl/opencl_utils.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <queue>


void parse_arguments(int argc, char* argv[], std::vector<cl::Device> &devices, std::string &filename, mode &mode_ret)
{
	//program name + filename + mode = 3
	if (argc < 3)
	{
		std::cout << "Not enough arguments, exiting" << std::endl;
		return;
	}

	//get filename and mode
	filename = argv[1];
	std::string mode_in(argv[2]);

	//local variables
	cl::Device device;
	std::vector<cl::Device> devices_temp;
	std::stringstream args_ss;
	std::string arguments;


	//check if mode is all or SMP
	if (mode_in == "all" && argc == 3)
	{
		devices = get_all_opencl_devices();
		mode_ret = all;
	}
	else if (mode_in == "SMP" && argc == 3)
	{
		mode_ret = smp;
	}
	//else check for opencl device names
	else
	{
		//create a std::string from arguments
		for (int i = 2; i < argc; i++)
		{
			args_ss << argv[i];
			args_ss << " ";
		}
		arguments = args_ss.str();
		
		//get all opencl devices
		devices_temp = get_all_opencl_devices();
		for (cl::Device &dev : devices_temp)
		{
			//check for each available device if it's in argument string
			std::string dev_name = dev.getInfo<CL_DEVICE_NAME>();
			auto pos = arguments.find(dev_name);
			if (pos != std::string::npos)
			{
				//device found in argument string, add to device return vector and remove it from argument string
				devices.push_back(dev);
				arguments = arguments.substr(0, pos) + arguments.substr(pos+dev_name.size());
				std::cout << arguments << std::endl;
			}
		}

		//erase whitespace
		arguments.erase(std::remove_if(arguments.begin(), arguments.end(), [](unsigned char x) { return std::isspace(x); }), arguments.end());
		
		//string should be empty if parameters were correctly input
		if (!arguments.empty())
		{
			std::cout << "Invalid args, exiting\nAvailable devices:" << std::endl;
			for (auto dev_t : devices_temp)
			{
				std::cout << dev_t.getInfo<CL_DEVICE_NAME>() << std::endl;
			}
			exit(1);
		}
		std::cout << "Selected devices mode" << std::endl;
		mode_ret = opencl;
	}
}
