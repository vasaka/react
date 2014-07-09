/*
* 2014+ Copyright (c) Vasaka <vasaka@gmail.com>
* All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*/

#ifndef _GLIBCXX_USE_NANOSLEEP
#define _GLIBCXX_USE_NANOSLEEP
#endif

#ifndef _GLIBCXX_USE_CLOCK_REALTIME
#define _GLIBCXX_USE_CLOCK_REALTIME
#endif

#include <thread>
#include <chrono>

#include "react/defines.hpp"

// Defining stub functions
bool find_record() {
	std::this_thread::sleep_for( std::chrono::microseconds(10) );
	return (rand() % 4) == 0;
}

std::string read_from_disk() {
	PROFILE_FUNC();

	std::this_thread::sleep_for( std::chrono::microseconds(1000) );
	return "DISK";
}

void put_into_cache(std::string data) {
	PROFILE_FUNC();

	std::this_thread::sleep_for( std::chrono::microseconds(50) );
}

std::string load_from_cache() {
	PROFILE_FUNC();

	std::this_thread::sleep_for( std::chrono::microseconds(25) );
	return "CACHE";
}

std::string cache_read() {
	PROFILE_FUNC();

	std::string data;

	PROFILE_START(action_find); // Starts new action which will be inner to ACTION_READ
	bool found = find_record();
	PROFILE_STOP(action_find);

	if (!found) {
		PROFILE_BLOCK(load_from_disk);

		data = read_from_disk();
		put_into_cache(data);
		return data; // Here all action guards are destructed and actions are correctly finished
	}
	data = load_from_cache();

	return data;
}

const int ITERATIONS_NUMBER = 10;

void run_example() {
	std::cout << "Running cache read " << ITERATIONS_NUMBER << " times" << std::endl;

	react::stream_aggregator_t aggregator(std::cout);

	for (int i = 0; i < ITERATIONS_NUMBER; ++i) {
		react_activate(&aggregator);

		std::string data = cache_read();

		react_deactivate();
	}
}

int main() {
	run_example();
	return 0;
}
