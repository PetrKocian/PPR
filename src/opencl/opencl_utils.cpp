#include <CL/cl.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include "opencl_utils.h"

//return code from filename as string
std::string load_kernel_code(std::string filename) {
	std::ifstream fileStream(filename);
	std::stringstream buffer;
	buffer << fileStream.rdbuf();
	return buffer.str();
}

//returns vector of all available opencl devices
std::vector<cl::Device> get_all_opencl_devices()
{
	std::vector<cl::Device> all_devices;
	std::vector<cl::Device> all_platform_devices;
	std::vector<cl::Platform> all_platforms;
	cl::Platform::get(&all_platforms);

	if (all_platforms.size() == 0) {
		std::cout << " No platforms found" << std::endl;
		exit(1);
	}

	for (cl::Platform platform : all_platforms)
	{
		platform.getDevices(CL_DEVICE_TYPE_ALL, &all_platform_devices);
		for (cl::Device device : all_platform_devices)
		{
			all_devices.push_back(device);
		}
	}

	if (all_devices.size() == 0) {
		std::cout << " No devices found" << std::endl;
		exit(1);
	}

	return all_devices;
}

//CURRENTLY NOT IN USE
//returns true or false based if requested device was found, and returns it as &target_device if true
bool get_device(std::string requested_device_name, cl::Device& target_device)
{
	std::vector<cl::Device> all_devices;
	std::vector<cl::Device> all_platform_devices;
	std::vector<cl::Platform> all_platforms;
	bool device_found = false;
	cl::Platform::get(&all_platforms);

	for (cl::Platform platform : all_platforms)
	{
		platform.getDevices(CL_DEVICE_TYPE_ALL, &all_platform_devices);
		for (cl::Device device : all_platform_devices)
		{
			all_devices.push_back(device);
		}
	}

	if (all_devices.size() == 0) {
		std::cout << " Device " << requested_device_name << " not found" << std::endl;
		exit(1);
	}

	for (int i = 0; i < all_devices.size(); i++)
	{
		if (all_devices[i].getInfo<CL_DEVICE_NAME>() == requested_device_name)
		{
			target_device = all_devices[i];
			device_found = true;
			break;
		}
	}

	return device_found;
}