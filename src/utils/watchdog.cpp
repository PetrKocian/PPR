#include "watchdog.h"
#include <iostream>

Watchdog::Watchdog(std::chrono::milliseconds timeout) : timeout(timeout) {}

void Watchdog::start()
{
	guarding = true;
	watchdog_thread = std::thread(&Watchdog::run, this);
}

void Watchdog::stop()
{
	guarding = false;
	watchdog_thread.join();
}

void Watchdog::kick(size_t count)
{
	processed_items += count;
}

void Watchdog::run()
{
	std::cout << "WATCHDOG RUNNING" << std::endl;
	size_t processed_old;
	size_t processed_now;
	while (guarding)
	{
		processed_old = processed_items;
		std::cout << "old " << processed_old << std::endl;
		std::this_thread::sleep_for(timeout);
		processed_now = processed_items;
		std::cout << "now " << processed_now << std::endl;

		if (processed_now <= processed_old)
		{
			std::cout << "WATCHDOG BITE" << std::endl;
		}
		else
		{
			std::cout << "WATCHDOG NO BITE" << std::endl;
		}
	}
}