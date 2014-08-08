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

#include "react/global_profiler.hpp"

#include <sstream>

namespace react {

global_profiler_t global_profiler_t::m_profiler("log.react");

std::string get_thread_id() {
	std::ostringstream ss;
	ss << std::this_thread::get_id();
	std::string id_str = ss.str();
	return id_str;
}

global_profiler_t::global_profiler_t(const std::string &file_name)
	: m_call_tree(m_actions_set)
	, m_aggregator(m_output)
	, m_output(file_name)
	, m_name(file_name)
	, m_refresh_interval(1000)
	, m_active(CONTINUOUS_REACT_OUTPUT)
{
	if (CONTINUOUS_REACT_OUTPUT) {
		m_profile_thread = std::thread(&global_profiler_t::profile_loop, this);
	}
}

global_profiler_t::~global_profiler_t()
{
	if (CONTINUOUS_REACT_OUTPUT) {
		m_active = false;
		m_profile_thread.join();
	}

	write_call_tree();
}

void global_profiler_t::profile_loop()
{
	while (m_active) {
		std::this_thread::sleep_for(std::chrono::milliseconds(m_refresh_interval));
		write_call_tree();
	}
}

void global_profiler_t::write_call_tree()
{
	react::call_tree_t output_tree = m_call_tree.copy_call_tree();
	m_output.close();
	m_output.open(m_name, std::ios_base::out | std::ios_base::trunc);
	m_aggregator.aggregate(output_tree);
}


react::call_tree_updater_t* global_profiler_t::get_updater()
{
	static thread_local react::call_tree_updater_t updater(get_profiler().m_call_tree);
	return &updater;
}

react::actions_set_t& global_profiler_t::get_action_set() {
	return m_actions_set;
}

global_profiler_t& global_profiler_t::get_profiler()
{
	return m_profiler;
}

}

