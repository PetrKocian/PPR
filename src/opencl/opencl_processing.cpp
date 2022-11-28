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
		exit(1);
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
	/*
	std::cout << "KURT_inside " << s.kurtosis() << std::endl;
	std::cout << "count100 " << s.get_n() << std::endl;
	std::cout << "mean100 " << s.mean() << std::endl;
	*/
	return s;
}


