#include <CL/cl.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include <filesystem>
#include "opencl_processing.h"
#include "opencl_utils.h"
#include "../utils/utils.h"


//local defines
#define NUMBER_OF_ELEMENTS_CL NUMBER_OF_DOUBLES_CL

#define DOUBLES_BUFFER_SIZE_CL (sizeof(double)*NUMBER_OF_ELEMENTS_CL)

#define WORKITEMS (NUMBER_OF_ELEMENTS_CL/100)

#define NUMBER_OF_RESULTS (WORKITEMS * 6)

#define RESULT_BUFFER_SIZE_CL (sizeof(double)*NUMBER_OF_RESULTS)

//function passed executed by an opencl manager thread
void cl_manager(std::vector<std::vector<char>>& cl_buffer, Stats& result, std::atomic<bool>& finished, Watchdog& dog, cl::Device& device)
{
	//local variables
	Device_opencl_struct dev;
	std::vector<char> buffer;
	Stats stats;
	bool get_data = true;

	//builds stats kernel for target device
	prepare_opencl_device(device, dev);

	//keep computing until while there is data in opencl buffer
	while (get_data)
	{
		cl_buffer_mutex.lock();
		//pop chunk of data from buffer
		if (!cl_buffer.empty())
		{
			buffer = cl_buffer.back();
			cl_buffer.pop_back();
			cl_buffer_mutex.unlock();

			//compute stats for chunk and add to final result
			stats = compute_stats_opencl(dev, buffer);
			result.add_stats(stats);
			//kick watchdog
			dog.kick(stats.get_n());
		}
		else
		{
			//if finished, release mutex and break loop
			if (finished)
			{
				get_data = false;
			}
			cl_buffer_mutex.unlock();
		}
	}
}


void prepare_opencl_device(cl::Device device, Device_opencl_struct &device_struct)
{
	//assign device to struct and create context
	device_struct.device = device;
	device_struct.context = cl::Context(device);

	//load stats kernel code and build it
	cl::Program::Sources sources;
	std::string cl_code = load_kernel_code("../src/opencl/stats_kernel.cl");
	sources.push_back({ cl_code.c_str(), cl_code.length() });

	device_struct.program = cl::Program(device_struct.context, sources);
	if (device_struct.program.build({ device }) != CL_SUCCESS) {
		std::cout << "Error building for device " << device.getInfo<CL_DEVICE_NAME>() << std::endl << device_struct.program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
		exit(1);
	}

	//init buffers
	device_struct.buffer_doubles = cl::Buffer(device_struct.context, CL_MEM_READ_WRITE, DOUBLES_BUFFER_SIZE_CL);
	device_struct.buffer_result = cl::Buffer(device_struct.context, CL_MEM_READ_WRITE, RESULT_BUFFER_SIZE_CL);

	//init queue and kernel
	device_struct.queue = cl::CommandQueue(device_struct.context, device_struct.device);

	device_struct.kernel = cl::Kernel(device_struct.program, "compute_stats");

	//set buffers as arguments to kernel
	device_struct.kernel.setArg(0, device_struct.buffer_doubles);
	device_struct.kernel.setArg(1, device_struct.buffer_result);
}

Stats compute_stats_opencl(Device_opencl_struct& dev, std::vector<char> buffer)
{
	//local variables
	std::array<double, NUMBER_OF_RESULTS> result_arr;
	Stats_partial sp;
	Stats stats;

	//execute kernel with input buffer and load data into result array
	dev.queue.enqueueWriteBuffer(dev.buffer_doubles, CL_TRUE, 0, DOUBLES_BUFFER_SIZE_CL, buffer.data());
	cl_int result = dev.queue.enqueueNDRangeKernel(dev.kernel, cl::NullRange, cl::NDRange(WORKITEMS), cl::NullRange);
	dev.queue.enqueueReadBuffer(dev.buffer_result, CL_TRUE, 0, RESULT_BUFFER_SIZE_CL, result_arr.data());
	dev.queue.finish();

	//initialize final stats
	sp.n = result_arr[0];
	sp.m1 = result_arr[1];
	sp.m2 = result_arr[2];
	sp.m3 = result_arr[3];
	sp.m4 = result_arr[4];
	sp.only_ints = result_arr[5];
	stats.set_stats(sp);

	//add stats from each workitem to final stats
	for (int i = 6; i < NUMBER_OF_RESULTS; i += 6)
	{
		sp.n = result_arr[i];
		sp.m1 = result_arr[i + 1];
		sp.m2 = result_arr[i + 2];
		sp.m3 = result_arr[i + 3];
		sp.m4 = result_arr[i + 4];
		sp.only_ints = result_arr[i + 5];
		stats.add_stats(sp);
	}

	//return final stats
	return stats;
}


