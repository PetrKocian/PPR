#include "my_timer.h"
#include <iostream>

void Timer::clear()
{
	time = 0;
}

void Timer::start()
{
	start_value = std::chrono::high_resolution_clock::now();
}

void Timer::end()
{
	end_value = std::chrono::high_resolution_clock::now();
	auto temp_time = std::chrono::duration_cast<std::chrono::microseconds>(end_value - start_value);
	time = temp_time.count();
}

int Timer::get_time() const
{
	return time;
}

int Timer::get_time_ms() const
{
	return time / 1000;
}

void Timer::print() const
{
	std::cout	<< "Time: " << this->get_time() << "us" << std::endl
				<< "      " << this->get_time_ms() << "ms" << std::endl;
}