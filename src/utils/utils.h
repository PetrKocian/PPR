#pragma once
#include <immintrin.h>
#include <string>
#include <mutex>
#include "stats.h"
#include <atomic>

//number of elements passed to managing threads for cpu/opencl
#define NUMBER_OF_DOUBLES_CPU 1000000
#define NUMBER_OF_DOUBLES_CL 1000000
#define NUMBER_OF_READERS 4
#define NUMBER_OF_CPU_T 4


extern std::mutex cpu_buffer_mutex;
extern std::mutex cl_buffer_mutex;

//execution mode
enum mode
{
	all = 0,
	smp = 1,
	opencl = 2,
	sequential = 3,
	debug = 4
};

enum distr_type
{
	normal = 0,
	poisson = 1,
	exponential = 2,
	uniform = 3,
	unknown = 4
};

class Distribution
{
private:
	std::atomic<size_t> normal_c = 0;
	std::atomic<size_t> poisson_c = 0;
	std::atomic<size_t> exponential_c = 0;
	std::atomic<size_t> uniform_c = 0;
	std::atomic<size_t> unknown_c = 0;
	size_t max_size = 0;
	size_t total = 0;
	distr_type final_distribution = unknown;
public:
	void make_distribution_decision();
	void push_distribution(distr_type distribution_type);
	void print_distribution_decision();
};

std::string pd_v_str(__m256d vec);
