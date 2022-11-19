#include <iostream>
#include <string>
#include "naive/naive.h"
#include "utils/watchdog.h"

int wmain(int argc, wchar_t** argv) {
	Numbers test, test2;
	std::string line;

	std::wcout << "Zadejte jmeno souboru:" << std::endl;

	//read name of file
	std::getline(std::cin, line);
	read_and_analyze_file_tbb("../../ppr_data/" + line);

	test = read_and_analyze_file_v("../../ppr_data/"+line);

	test2 = read_and_analyze_file_naive("../../ppr_data/" + line);

	Watchdog dog(std::chrono::milliseconds(5000));
	dog.start();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	dog.kick(1);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	dog.kick(1);
	dog.stop();

	std::wcout << "Vystup programu" << std::endl;
	//wait for enter
	std::getline(std::cin, line);

	
}