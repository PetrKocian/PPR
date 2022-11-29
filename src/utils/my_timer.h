#pragma once
#include <chrono>

//class for timing benchmarks
class Timer
{
private:
	std::chrono::high_resolution_clock::time_point start_value;
	std::chrono::high_resolution_clock::time_point end_value;
	int64_t time = 0;
public:
	void start();
	void end();
	void clear();
	int get_time() const;
	int get_time_ms() const;
};

//global timer instance
static Timer t;