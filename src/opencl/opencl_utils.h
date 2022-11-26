#pragma once
#include <string>
#include <CL/cl.hpp>

struct Device_opencl_struct 
{
	cl::Device device;
	cl::Context context;
	cl::Program program;
	cl::Buffer buffer_doubles;
	cl::Buffer buffer_result;
	cl::CommandQueue queue;
	cl::Kernel kernel;
};

std::string load_kernel_code(std::string filename);
std::vector<cl::Device> get_all_devices();
bool get_device(std::string requested_device_name, cl::Device& target_device);
