#include "arg_parser.h"
#include "../opencl/opencl_utils.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <queue>

std::vector<std::string> generate_args()
{
	std::string line;
	std::vector<std::string> args;

	std::cout << "INPUT ARGUMENTS:" << std::endl;
	std::getline(std::cin, line);

	while(true)
	{
		auto pos = line.find(" ");
		if (pos == std::string::npos)
		{
			args.push_back(line);
			break;
		}

		args.push_back(line.substr(0, pos));
		line = line.substr(pos + 1);
	}
	std::cout << "succes " << std::endl;
	return args;
}

void parse_arguments(int argc, char* argv[], std::vector<cl::Device> &devices)
{
	if (argc < 3)
	{
		std::cout << "Not enough arguments, exiting" << std::endl;
		return;
	}
	//std::vector<std::string> args = generate_args();
	std::string filename(argv[1]);
	std::string mode(argv[2]);
	cl::Device device;
	std::vector<cl::Device> devices_temp;
	std::stringstream args_ss;
	std::string arguments;


	if (mode == "all" && argc == 3)
	{
		std::cout << "all mode"  << std::endl;
		devices = get_all_devices();
	}
	else if (mode == "SMP" && argc == 3)
	{
		std::cout << "SMP mode" << std::endl;
	}
	else
	{
		for (int i = 2; i < argc; i++)
		{
			args_ss << argv[i];
			args_ss << " ";
		}
		arguments = args_ss.str();
		
		devices_temp = get_all_devices();
		for (cl::Device &dev : devices_temp)
		{
			std::string dev_name = dev.getInfo<CL_DEVICE_NAME>();
			auto pos = arguments.find(dev_name);
			if (pos != std::string::npos)
			{
				std::cout << dev_name <<" device found" << std::endl;
				devices.push_back(dev);
				arguments = arguments.substr(0, pos) + arguments.substr(pos+dev_name.size());
				std::cout << arguments << std::endl;
			}
		}

		//erase spaces
		arguments.erase(std::remove_if(arguments.begin(), arguments.end(), [](unsigned char x) { return std::isspace(x); }), arguments.end());
		
		//string should be empty if parameters were correctly input
		if (!arguments.empty())
		{
			std::cout << "Invalid args, exiting" << std::endl;
			exit(1);
		}
		std::cout << "Selected devices mode" << std::endl;
	}
}
