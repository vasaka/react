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

#ifndef REACT_DEFINES_HPP
#define REACT_DEFINES_HPP

#include "react/react.hpp"
#include "react/utils.hpp"

#define PROFILE_FUNC() \
static const int react_defined_action = react_define_new_action(__FUNCTION__); \
react::action_guard react_defined_guard(react_defined_action);

#define PROFILE_BLOCK(NAME) \
static const int react_defined_action_ ## NAME = react_define_new_action(#NAME); \
react::action_guard react_defined_guard(react_defined_action_ ## NAME);

#define PROFILE_START(NAME) \
static const int react_defined_action_ ## NAME = react_define_new_action(#NAME); \
react_start_action(react_defined_action_ ## NAME);

#define PROFILE_STOP(NAME) \
react_stop_action(react_defined_action_ ## NAME);

#endif //REACT_DEFINES_HPP
