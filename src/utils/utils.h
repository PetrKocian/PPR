#pragma once
#include <immintrin.h>
#include <string>
#include <mutex>

extern std::mutex cpu_buffer_mutex;
extern std::mutex cl_buffer_mutex;

enum mode
{
	all = 0,
	smp = 1,
	opencl = 2
};

std::string pd_v_str(__m256d vec);
