#include <iostream>
#include <string>
#include "naive/naive.h"

int wmain(int argc, wchar_t** argv) {
	Numbers test, test2;
	std::string line;

	std::wcout << "Zadejte jmeno souboru:" << std::endl;

	//read name of file
	std::getline(std::cin, line);

	test = read_and_analyze_file_v("../../ppr_data/"+line);

	test2 = read_and_analyze_file_naive("../../ppr_data/" + line);



	std::wcout << "Vystup programu" << std::endl;
	//wait for enter
	std::getline(std::cin, line);

	
}