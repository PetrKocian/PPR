#include <iostream>
#include <string>
#include "naive/naive.h"

int wmain(int argc, wchar_t** argv) {
	Numbers test;
	std::string line;

	std::wcout << "Zadejte jmeno souboru:" << std::endl;

	//read name of file
	std::getline(std::cin, line);

	test = read_and_analyze_file("../../ppr_data/"+line);



	std::wcout << "Vystup programu" << std::endl;
	//wait for enter
	std::getline(std::cin, line);

	
}