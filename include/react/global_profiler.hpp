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

#ifndef __react_global_profiler_h__
#define __react_global_profiler_h__

#include "react/react.hpp"
#include "react/utils.hpp"

#include <thread>
#include <fstream>

#define CONTINUOUS_REACT_OUTPUT 0

namespace react {

class global_profiler_t;
std::string get_thread_id();

}

/*!
 *  Macros provided for shinyprofiler style profiler usage.
 */

#define PROFILE_FUNC_GLOBAL()\
static thread_local const int react_defined_action = react::global_profiler_t::get_profiler().get_action_set().define_new_action( \
	__FUNCTION__ + std::string("_") + react::get_thread_id()); \
react::action_guard_t react_defined_guard(react::global_profiler_t::get_updater(), react_defined_action);

#define MERGE_PROFILE_FUNC_GLOBAL()\
static thread_local const int react_defined_action = react::global_profiler_t::get_profiler().get_action_set().define_new_action( \
	__FUNCTION__ + std::string("_") + react::get_thread_id() + "_merge"); \
react::action_guard_t react_defined_guard(react::global_profiler_t::get_updater(), react_defined_action, true);

#define SAMPLE_MERGE_PROFILE_FUNC_GLOBAL(SAMPLE_PERIOD)\
static int react_sample_counter = 0; \
static thread_local const int react_defined_action = react::global_profiler_t::get_profiler().get_action_set().define_new_action( \
	__FUNCTION__ + std::string("_") + react::get_thread_id() + "_sample_" + std::to_string(SAMPLE_PERIOD)); \
react::action_guard_t react_defined_guard(react::global_profiler_t::get_updater(), react_defined_action, ((react_sample_counter++) % SAMPLE_PERIOD) != 0);

#define PROFILE_BLOCK_GLOBAL(NAME)\
static thread_local const int react_defined_action_ ## NAME = react::global_profiler_t::get_profiler().get_action_set().define_new_action( \
	#NAME + std::string("_") + react::get_thread_id()); \
react::action_guard_t react_defined_guard(react::global_profiler_t::get_updater(), react_defined_action_ ## NAME);

#define MERGE_PROFILE_BLOCK_GLOBAL(NAME)\
static thread_local const int react_defined_action_ ## NAME = react::global_profiler_t::get_profiler().get_action_set().define_new_action( \
	#NAME + std::string("_") + react::get_thread_id() + "_merge"); \
react::action_guard_t react_defined_guard(react::global_profiler_t::get_updater(), react_defined_action_ ## NAME, true);

#define SAMPLE_MERGE_PROFILE_BLOCK_GLOBAL(NAME, SAMPLE_PERIOD)\
static int react_sample_counter ## NAME = 0; \
static thread_local const int react_defined_action_ ## NAME = react::global_profiler_t::get_profiler().get_action_set().define_new_action( \
#NAME + std::string("_") + react::get_thread_id() + "_sample_" + std::to_string(SAMPLE_PERIOD)); \
react::action_guard_t react_defined_guard(react::global_profiler_t::get_updater(), react_defined_action_ ## NAME, ((react_sample_counter ## NAME ++) % SAMPLE_PERIOD) != 0);

namespace react {


/*!
 * \brief Class to manage global action set and call tree and per-thread call tree updater.
 *
 *  Allows you to globally log actions in call-tree manner, new threads will attach to root.
 */
class global_profiler_t {
public:
	/*!
	 * \brief Returns global profiler action set.
	 */
	react::actions_set_t&	get_action_set();

	/*!
	 * \brief Returns global profiler.
	 */
	static global_profiler_t& get_profiler();

	/*!
	 * \brief Returns per-thread updater.
	 */
	static react::call_tree_updater_t* get_updater();

private:
	/*!
	 * \brief Initializes profiler.
	 * \param name Output filename.
	 */
	global_profiler_t(const std::string &file_name);

	/*!
	 * \brief Stop output thread if exists and output collected data.
	 */
	~global_profiler_t();

	/*!
	 * \brief Continuously output collected data(optional).
	 */
	void profile_loop();

	/*!
	 * \brief Writes current call tre to out file.
	 */
	void write_call_tree();

	/*!
	 * \brief Global action set.
	 */
	react::actions_set_t			m_actions_set;

	/*!
	 * \brief Global call tree.
	 */
	react::concurrent_call_tree_t	m_call_tree;

	/*!
	 * \brief Global aggregator.
	 */
	react::stream_aggregator_t		m_aggregator;

	/*!
	 * \brief Profiler output thread(optional).
	 */
	std::thread						m_profile_thread;

	/*!
	 * \brief Outout file.
	 */
	std::ofstream					m_output;

	/*!
	 * \brief Output file name.
	 */
	std::string						m_name;

	/*!
	 * \brief Output refresh interval.
	 */
	const int						m_refresh_interval;

	/*!
	 * \brief If continuous output active.
	 */
	bool							m_active;
};

}

#endif //__react_global_profiler_h__
