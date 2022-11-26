#include "arg_parser.h"
#include "../opencl/opencl_utils.h"
#include <iostream>
#include <vector>




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

void parse_arguments(std::string &filename, std::vector<cl::Device> &devices)
{
	std::vector<std::string> args = generate_args();
	filename = args.at(0);
	cl::Device device;

	if (args.size() < 2)
	{
		std::cout << "NOT ENOUGH ARGS" << std::endl;
		return;
	}

	if (args.at(1) == "all")
	{
		std::cout << "ALL"  << std::endl;
		devices = get_all_devices();
	}
	else if (args.at(1) == "SMP")
	{
		std::cout << "SMMPPP" << std::endl;
	}
	else
	{
		for (int i = 1; i < args.size(); i++)
		{
			if (get_device(args.at(i), device) == false)
			{
				std::cout << "Specified device not found" << std::endl;
				//exit(1);
			}
			devices.push_back(device);
		}
	}
}
