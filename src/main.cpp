#include <iostream>
#include <string>
#include "naive/naive.h"
#include "utils/watchdog.h"
#include "utils/arg_parser.h"

#include "opencl/opencl_processing.h"

int wmain(int argc, wchar_t** argv) {
	Numbers test1, test2;
	std::string line;
	std::vector<cl::Device> devices;



	std::wcout << "Zadejte jmeno souboru:" << std::endl;

	//read name of file
	std::getline(std::cin, line);
	/*
	//parse arguments from input - filename and opencl devices
	parse_arguments(line, devices);

	if (devices.size() == 0)
	{
		//SMP
	}
	else
	{
		// all/selected devices
	}

	std::getline(std::cin, line);

	*/
	//return 0;

	//read_and_analyze_file_tbb("../../ppr_data/" + line);
	test2 = read_and_analyze_file_naive("../../ppr_data/" + line);

	test1 = read_and_analyze_file_v("../../ppr_data/"+line);

	//test2 = read_and_analyze_file_naive("../../ppr_data/" + line);

	test_vadd("../../ppr_data/" + line);


	/*Watchdog dog(std::chrono::milliseconds(5000));
	dog.start();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	dog.kick(1);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	dog.kick(1);
	dog.stop();*/

	std::wcout << "Vystup programu" << std::endl;
	//wait for enter
	std::getline(std::cin, line);

	
}