#pragma once

#include <vector>

struct Numbers
{
public:
	uint32_t valid_doubles = 0;
	uint32_t invalid = 0;
	std::vector<double> doubles;
};

Numbers read_and_analyze_file_v(std::string filename);
Numbers read_and_analyze_file_naive(std::string filename);