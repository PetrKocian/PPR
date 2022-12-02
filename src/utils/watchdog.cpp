#include "watchdog.h"
#include <iostream>

Watchdog::Watchdog(std::chrono::milliseconds timeout, Distribution &distr) : timeout(timeout), final_distr(distr) {}

void Watchdog::start()
{
	guarding = true;
	watchdog_thread = std::thread(&Watchdog::run, this);
}

void Watchdog::stop()
{
	guarding = false;
	std::cout << "Count: " << processed_items << std::endl;
	watchdog_thread.detach();
}

void Watchdog::kick(size_t count)
{
	processed_items += count;
}

void Watchdog::run()
{
	size_t processed_old;
	size_t processed_now;
	while (guarding)
	{
		processed_old = processed_items;
		std::this_thread::sleep_for(timeout);
		processed_now = processed_items;

		//check if value increased
		if (processed_now <= processed_old)
		{
			timeout_count++;
			std::cout << "Program inactive for " << (timeout_count*timeout.count()/1000) << " seconds" << std::endl;
			//Terminate whole program after timeout*6 seconds
			if (timeout_count > 5)
			{
				std::cout << "Terminating, current results are: " << std::endl 
				<< "Processed doubles: " << processed_items;
				final_distr.make_distribution_decision();
				final_distr.print_distribution_decision();
				std::terminate();
			}
		}
		else
		{
			//reset 30 seconds timeout count
			timeout_count = 0;
		}
	}
}