#include <CL/cl.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include "vadd.h"
#include "../utils/stats.h"
#include <filesystem>

#define SIZE_OF_BUFFER sizeof(double)*10000

void Load_Code(std::string file, std::string& cl_code) {
	std::ifstream fileStream(file);
	std::stringstream buffer;
	buffer << fileStream.rdbuf();
	cl_code = buffer.str();
}


void test_vadd(std::string filename)
{

	std::vector<char> buffer(SIZE_OF_BUFFER);
	std::ifstream input_file(filename, std::ifstream::in | std::ifstream::binary);
	input_file.read(buffer.data(), SIZE_OF_BUFFER);

	std::string line;
	std::vector<cl::Platform> all_platforms;
	cl::Platform::get(&all_platforms);

	std::string cl_code;

	Load_Code("../src/opencl/vadd_kernel.cl", cl_code);

	if (all_platforms.size() == 0) {
		std::cout << " No platforms found. Check OpenCL installation!\n";
		exit(1);
	}
	cl::Platform default_platform = all_platforms[0];
	std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";

	// get default device (CPUs, GPUs) of the default platform
	std::vector<cl::Device> all_devices;
	default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);

	if (all_devices.size() == 0) {
		std::cout << " No devices found. Check OpenCL installation!\n";
		exit(1);
	}

	for (auto device : all_devices)
	{
		std::cout << "Available device: " << device.getInfo<CL_DEVICE_NAME>() << "\n";
	}

	cl::Device default_device = all_devices[0];

	cl::Context context({ default_device });

	cl::Program::Sources sources;

	std::array<double,500> result_arr;

	sources.push_back({ cl_code.c_str(), cl_code.length() });

	cl::Program program(context, sources);
	if (program.build({ default_device }) != CL_SUCCESS) {
		std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << std::endl;
		//exit(1);
	}

	std::cout << "Hit Enter to continue" << std::endl;
	std::getline(std::cin, line);

	cl::CommandQueue queue(context, default_device);

	cl::Buffer buffer_doubles = cl::Buffer(context, CL_MEM_READ_WRITE, SIZE_OF_BUFFER);
	cl::Buffer buffer_result = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(double)*500);

	queue.enqueueWriteBuffer(buffer_doubles, CL_TRUE, 0, SIZE_OF_BUFFER, buffer.data());

	cl::Kernel kurt = cl::Kernel(program, "compute_stats");

	kurt.setArg(0, buffer_doubles);
	kurt.setArg(1, buffer_result);

	cl_int result = queue.enqueueNDRangeKernel(kurt, cl::NullRange, cl::NDRange(100), cl::NullRange);

	if (result != CL_SUCCESS)
		std::cout << "ERROR" << result << std::endl;

	queue.enqueueReadBuffer(buffer_result, CL_TRUE, 0, sizeof(double) * 500, result_arr.data());

	queue.finish();

	std::cout << std::endl << result_arr[0] << std::endl <<"mean " << result_arr[1] 
		<< std::endl<< result_arr[2] <<std::endl  << result_arr[3] << std::endl
		<< "kurt OPENCL " << result_arr[4]<<std::endl;
	std::cout << result_arr[5] << " " << result_arr[6] << " " << result_arr[7] << " " << result_arr[8] << " " << std::endl;

	Stats_partial sp = combine_stats(result_arr[0], result_arr[5], result_arr[4], result_arr[9],
		result_arr[3], result_arr[8], result_arr[2], result_arr[7], result_arr[1], result_arr[6]);

	Stats s;
	s.set_stats(sp);

	std::cout << "KURT " << s.kurtosis() << std::endl;
	std::cout << "count " << s.get_n() << std::endl;
	std::cout << " mean " << s.mean() << std::endl;

	sp.n = result_arr[0];
	sp.m1 = result_arr[1];
	sp.m2 = result_arr[2];
	sp.m3 = result_arr[3];
	sp.m4 = result_arr[4];
	s.set_stats(sp);

	Stats s_toadd;

	for (int i = 5; i < 500; i += 5)
	{
		sp.n = result_arr[i];
		sp.m1 = result_arr[i+1];
		sp.m2 = result_arr[i+2];
		sp.m3 = result_arr[i+3];
		sp.m4 = result_arr[i+4];
		s_toadd.set_stats(sp);
		s.add_stats(s_toadd);
	}

	std::cout << "KURT100 " << s.kurtosis() << std::endl;
	std::cout << "count100 " << s.get_n() << std::endl;
	std::cout << "mean100 " << s.mean() << std::endl;

	std::cout << "Complete" << std::endl;
}


