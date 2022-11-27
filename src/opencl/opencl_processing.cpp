#include <CL/cl.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include "opencl_processing.h"
#include "opencl_utils.h"
#include <filesystem>



//TODO: CLEANUP
//split into some functions at least
//stats.h think about implementation to set stats

#define NUMBER_OF_ELEMENTS_CL 1000000

#define DOUBLES_BUFFER_SIZE_CL (sizeof(double)*NUMBER_OF_ELEMENTS_CL)

#define WORKITEMS (NUMBER_OF_ELEMENTS_CL/100)

#define NUMBER_OF_RESULTS (WORKITEMS * 5)

#define RESULT_BUFFER_SIZE_CL (sizeof(double)*NUMBER_OF_RESULTS)

void prepare_opencl_device(cl::Device device, Device_opencl_struct &device_struct)
{
	device_struct.device = device;
	device_struct.context = cl::Context(device);

	cl::Program::Sources sources;
	std::string cl_code = load_kernel_code("../src/opencl/stats_kernel.cl");
	sources.push_back({ cl_code.c_str(), cl_code.length() });

	device_struct.program = cl::Program(device_struct.context, sources);
	if (device_struct.program.build({ device }) != CL_SUCCESS) {
		std::cout << "Error building for device " << device.getInfo<CL_DEVICE_NAME>() << std::endl << device_struct.program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
		//TODO::exit(1);
	}

	device_struct.buffer_doubles = cl::Buffer(device_struct.context, CL_MEM_READ_WRITE, DOUBLES_BUFFER_SIZE_CL);
	device_struct.buffer_result = cl::Buffer(device_struct.context, CL_MEM_READ_WRITE, RESULT_BUFFER_SIZE_CL);

	device_struct.queue = cl::CommandQueue(device_struct.context, device_struct.device);

	device_struct.kernel = cl::Kernel(device_struct.program, "compute_stats");


	device_struct.kernel.setArg(0, device_struct.buffer_doubles);
	device_struct.kernel.setArg(1, device_struct.buffer_result);
}

Stats compute_stats_opencl(Device_opencl_struct& dev, std::vector<char> buffer)
{

	std::array<double, NUMBER_OF_RESULTS> result_arr;

	dev.queue.enqueueWriteBuffer(dev.buffer_doubles, CL_TRUE, 0, DOUBLES_BUFFER_SIZE_CL, buffer.data());
	cl_int result = dev.queue.enqueueNDRangeKernel(dev.kernel, cl::NullRange, cl::NDRange(WORKITEMS), cl::NullRange);
	dev.queue.enqueueReadBuffer(dev.buffer_result, CL_TRUE, 0, RESULT_BUFFER_SIZE_CL, result_arr.data());
	dev.queue.finish();
	Stats_partial sp;
	Stats s;
	sp.n = result_arr[0];
	sp.m1 = result_arr[1];
	sp.m2 = result_arr[2];
	sp.m3 = result_arr[3];
	sp.m4 = result_arr[4];
	s.set_stats(sp);

	for (int i = 5; i < NUMBER_OF_RESULTS; i += 5)
	{
		sp.n = result_arr[i];
		sp.m1 = result_arr[i + 1];
		sp.m2 = result_arr[i + 2];
		sp.m3 = result_arr[i + 3];
		sp.m4 = result_arr[i + 4];
		s.add_stats(sp);
	}

	std::cout << "KURT_inside " << s.kurtosis() << std::endl;
	std::cout << "count100 " << s.get_n() << std::endl;
	std::cout << "mean100 " << s.mean() << std::endl;

	return s;
}

void test_vadd(std::string filename)
{
	Device_opencl_struct dev;

	std::vector<char> buffer(DOUBLES_BUFFER_SIZE_CL);
	//std::array<double, NUMBER_OF_RESULTS> result_arr;

	std::ifstream input_file(filename, std::ifstream::in | std::ifstream::binary);
	input_file.read(buffer.data(), DOUBLES_BUFFER_SIZE_CL);

	std::vector<cl::Device> all_devices = get_all_devices();

	cl::Device default_device = all_devices[0];

	prepare_opencl_device(default_device, dev);

	compute_stats_opencl(dev, buffer);

	/*

	cl::Context context({ default_device });

	cl::Program::Sources sources;

	std::string cl_code = load_kernel_code("../src/opencl/stats_kernel.cl");

	sources.push_back({ cl_code.c_str(), cl_code.length() });

	cl::Program program(context, sources);
	if (program.build({ default_device }) != CL_SUCCESS) {
		std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << std::endl;
		//TODO::exit(1);
	}

	std::cout << "Hit Enter to continue" << std::endl;
	std::string line;
	std::getline(std::cin, line);


	cl::Buffer buffer_doubles = cl::Buffer(context, CL_MEM_READ_WRITE, DOUBLES_BUFFER_SIZE_CL);
	cl::Buffer buffer_result = cl::Buffer(context, CL_MEM_READ_WRITE, RESULT_BUFFER_SIZE_CL);

	cl::CommandQueue queue(context, default_device);*/
	/*
	dev.queue.enqueueWriteBuffer(dev.buffer_doubles, CL_TRUE, 0, DOUBLES_BUFFER_SIZE_CL, buffer.data());

	//cl::Kernel kurt = cl::Kernel(program, "compute_stats");

	dev.kernel.setArg(0, dev.buffer_doubles);
	dev.kernel.setArg(1, dev.buffer_result);

	cl_int result = dev.queue.enqueueNDRangeKernel(dev.kernel, cl::NullRange, cl::NDRange(WORKITEMS), cl::NullRange);

	//TODO: 
	if (result != CL_SUCCESS)
		std::cout << "ERROR" << result << std::endl;

	dev.queue.enqueueReadBuffer(dev.buffer_result, CL_TRUE, 0, RESULT_BUFFER_SIZE_CL, result_arr.data());


	dev.queue.finish();

	std::cout << std::endl << result_arr[0] << std::endl <<"mean " << result_arr[1] 
		<< std::endl<< result_arr[2] <<std::endl  << result_arr[3] << std::endl
		<< "kurt OPENCL " << result_arr[4]<<std::endl;
	std::cout << result_arr[5] << " " << result_arr[6] << " " << result_arr[7] << " " << result_arr[8] << " " << std::endl;

	Stats_partial sp = combine_stats(result_arr[0], result_arr[5], result_arr[4], result_arr[9],
		result_arr[3], result_arr[8], result_arr[2], result_arr[7], result_arr[1], result_arr[6]);

	Stats s;
	s.set_stats(sp);


	for (int i = 0; i < result_arr.size(); i++)
	{
		std::cout << result_arr[i] << std::endl;
	}

	std::cout << "KURT " << s.kurtosis() << std::endl;
	std::cout << "count " << s.get_n() << std::endl;
	std::cout << " mean " << s.mean() << std::endl;

	sp.n = result_arr[0];
	sp.m1 = result_arr[1];
	sp.m2 = result_arr[2];
	sp.m3 = result_arr[3];
	sp.m4 = result_arr[4];
	s.set_stats(sp);

	for (int i = 5; i < 500; i += 5)
	{
		sp.n = result_arr[i];
		sp.m1 = result_arr[i+1];
		sp.m2 = result_arr[i+2];
		sp.m3 = result_arr[i+3];
		sp.m4 = result_arr[i+4];
		s.add_stats(sp);
	}

	std::cout << "KURT100 " << s.kurtosis() << std::endl;
	std::cout << "count100 " << s.get_n() << std::endl;
	std::cout << "mean100 " << s.mean() << std::endl;
	*/
	std::cout << "Complete" << std::endl;
}


