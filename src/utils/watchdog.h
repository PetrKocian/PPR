#include <atomic>
#include <thread>
#include <chrono>
#include "utils.h"
#pragma once

class Watchdog
{
private:
	Distribution final_distr;
	std::atomic<size_t> processed_items = 0;
	uint16_t timeout_count = 0;
	std::thread watchdog_thread;
	std::chrono::milliseconds timeout;
	void run();
	bool guarding = false;
public:
	Watchdog(std::chrono::milliseconds timeout);
	void kick(size_t count);
	void start();
	void stop();
};