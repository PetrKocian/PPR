#pragma once
#include <immintrin.h>
#include <string>
#include <mutex>
#include "stats.h"

//number of elements passed to managing threads for cpu/opencl
#define NUMBER_OF_DOUBLES_CPU 10000
#define NUMBER_OF_DOUBLES_CL 1000000

extern std::mutex cpu_buffer_mutex;
extern std::mutex cl_buffer_mutex;

//execution mode
enum mode
{
	all = 0,
	smp = 1,
	opencl = 2
};

enum distr
{
	normal = 0,
	poisson = 1,
	exponential = 2,
	uniform = 3
};

std::string pd_v_str(__m256d vec);
distr decide_distr(Stats stats);
