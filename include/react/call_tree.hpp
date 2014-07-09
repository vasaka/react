/*
* 2013+ Copyright (c) Andrey Kashin <kashin.andrej@gmail.com>
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

#ifndef REACT_CALL_TREE_HPP
#define REACT_CALL_TREE_HPP

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "actions_set.hpp"

#include <unordered_map>
#include <vector>
#include <mutex>

#include <boost/variant.hpp>

namespace react {

/*!
 * \brief Types that can be stored in react call tree key-value storage
 */
typedef boost::variant<
	bool,
	int,
	double,
	std::string
> stat_value_t;

/*!
 * \brief Helper structure for printing stats stored in stat_value_t to json
 */
struct JsonRenderer : boost::static_visitor<>
{
	JsonRenderer(const std::string &key, rapidjson::Value &stat_value,
			 rapidjson::Document::AllocatorType &allocator):
		key(key), stat_value(stat_value), allocator(allocator) {}

	void operator () (bool value) const
	{
		stat_value.AddMember(key.c_str(), value, allocator);
	}

	void operator () (int value) const
	{
		stat_value.AddMember(key.c_str(), value, allocator);
	}

	void operator () (double value) const
	{
		stat_value.AddMember(key.c_str(), value, allocator);
	}

	void operator () (const std::string& value) const
	{
		stat_value.AddMember(key.c_str(), value.c_str(), allocator);
	}

private:
	std::string key;
	rapidjson::Value &stat_value;
	rapidjson::Document::AllocatorType &allocator;
};

/*!
 * \brief Represents node of call tree
 */
struct node_t {
	/*!
	 * \brief Type of container where child nodes are stored
	 */
	typedef std::vector<std::pair<int, size_t>> Container;

	/*!
	 * \brief Pointer to node type
	 */
	typedef size_t pointer;

	/*!
	 * \brief Initializes node with \a action_code and zero start and stop times
	 * \param action_code Action code of the node
	 */
	node_t(int action_code): action_code(action_code), start_time(0), stop_time(0) {}

	/*!
	 * \brief Action which this node represents
	 */
	int action_code;

	/*!
	 * \brief Time when node action was started
	 */
	int64_t start_time;

	/*!
	 * \brief Time when node action was stopped
	 */
	int64_t stop_time;

	/*!
	 * \brief Child nodes, actions that happen inside this action
	 */
	Container links;
};

/*!
 * \brief Stores call tree.
 *
 * Each node of the tree represents information about single action:
 * - Action code
 * - Time when action was started
 * - Time when action was stopped
 */
class call_tree_t {
public:
	typedef node_t::pointer p_node_t;

	/*!
	 * \brief Value for representing null node pointer
	 */
	static const p_node_t NO_NODE = -1;

	/*!
	 * \brief Pointer to the root of call tree
	 */
	p_node_t root;

	/*!
	 * \brief Initializes call tree with single root node and specified actions set
	 * \param actions_set Set of available actions for monitoring in call tree
	 */
	call_tree_t(const actions_set_t &actions_set): actions_set(actions_set) {
		root = new_node(+actions_set_t::NO_ACTION);
	}

	/*!
	 * \brief Frees memory consumed by call tree
	 */
	~call_tree_t() {}

	/*!
	 * \brief Returns actions set monitored by this tree
	 * \return Actions set monitored by this tree
	 */
	const actions_set_t& get_actions_set() const {
		return actions_set;
	}

	/*!
	 * \brief Returns links from \a node
	 * \param node Target node
	 * \return Links from target node
	 */
	const node_t::Container &get_node_links(p_node_t node) const {
		return nodes[node].links;
	}

	/*!
	 * \brief Returns an action code for \a node
	 * \param node Target node
	 * \return Action code of the target node
	 */
	int get_node_action_code(p_node_t node) const {
		return nodes[node].action_code;
	}

	/*!
	 * \brief Sets time when action represented by \a node was started
	 * \param node Action's node
	 * \param time Time when action was started
	 */
	void set_node_start_time(p_node_t node, int64_t time) {
		nodes[node].start_time = time;
	}

	/*!
	 * \brief Sets time when action represented by \a node was stopped
	 * \param node Action's node
	 * \param time Time when action was stopped
	 */
	void set_node_stop_time(p_node_t node, int64_t time) {
		nodes[node].stop_time = time;
	}

	/*!
	 * \brief Returns start time of action represented by \a node
	 * \param node Action's node
	 * \return Start time of action
	 */
	int64_t get_node_start_time(p_node_t node) const {
		return nodes[node].start_time;
	}

	/*!
	 * \brief Returns stop time of action represented by \a node
	 * \param node Action's node
	 * \return Stop time of action
	 */
	int64_t get_node_stop_time(p_node_t node) const {
		return nodes[node].stop_time;
	}

	/*!
	 * \brief Adds new child with \a action_code to \a node
	 * \param node Target parent node
	 * \param action_code Child's action code
	 * \return Pointer to newly created child
	 */
	p_node_t add_new_link(p_node_t node, int action_code) {
		if (!actions_set.code_is_valid(action_code)) {
			throw std::invalid_argument("Can't add new link: action code is invalid");
		}

		p_node_t action_node = new_node(action_code);
		nodes[node].links.push_back(std::make_pair(action_code, action_node));
		return action_node;
	}

	/*!
	 * \brief Finds a child with \a action_code to \a node or creates new if does not exists
	 * \param node Target parent node
	 * \param action_code Child's action code
	 * \return Pointer to child found or NO_NODE
	 */
	p_node_t find_link(p_node_t node, int action_code) {
		if (!actions_set.code_is_valid(action_code)) {
			throw std::invalid_argument("Can't add new link: action code is invalid");
		}

		node_t::Container &links = nodes[node].links;
		auto it = std::find_if(links.rbegin(), links.rend(), [&](const node_t::Container::value_type &v){return v.first == action_code;});
		return it == links.rend() ? call_tree_t::NO_NODE : it->second;
	}

	template<typename T>
	void add_stat(const std::string &key, T value) {
		stats[key] = value;
	}

	void add_stat(const std::string &key, const char *value) {
		stats[key] = std::string(value);
	}

	bool has_stat(const std::string &key) const {
		return stats.find(key) != stats.end();
	}

	template<typename T>
	const T &get_stat(const std::string &key) const {
		return boost::get<T>(stats.at(key));
	}

	/*!
	 * \brief Converts call tree to json
	 * \param stat_value Json node for writing
	 * \param allocator Json allocator
	 * \return Modified json node
	 */
	rapidjson::Value& to_json(rapidjson::Value &stat_value,
							  rapidjson::Document::AllocatorType &allocator) const {
		return to_json(root, stat_value, allocator);
	}

	/*!
	 * \brief Recursively merges this tree into \a rhs_node
	 * \param rhs_node Node in which this tree will be merged
	 * \param rhs_tree Tree in which this tree will be merged
	 */
	void merge_into(call_tree_t::p_node_t rhs_node, call_tree_t& rhs_tree) const {
		merge_into(root, rhs_node, rhs_tree);
	}

private:
	/*!
	 * \internal
	 *
	 * \brief Recursively converts subtree to json
	 * \param current_node Node which subtree will be converted
	 * \param stat_value Json node for writing
	 * \param allocator Json allocator
	 * \return Modified json node
	 */
	rapidjson::Value& to_json(p_node_t current_node, rapidjson::Value &stat_value,
							  rapidjson::Document::AllocatorType &allocator) const {
		if (current_node != root) {
			stat_value.AddMember("name", actions_set.get_action_name(get_node_action_code(current_node)).c_str(), allocator);
			stat_value.AddMember("start_time", get_node_start_time(current_node), allocator);
			stat_value.AddMember("stop_time", get_node_stop_time(current_node), allocator);
		} else {
			for (auto it = stats.begin(); it != stats.end(); ++it) {
				boost::apply_visitor(JsonRenderer(it->first, stat_value, allocator), it->second);
			}
		}

		if (!nodes[current_node].links.empty()) {
			rapidjson::Value subtree_actions(rapidjson::kArrayType);

			for (auto it = nodes[current_node].links.begin(); it != nodes[current_node].links.end(); ++it) {
				p_node_t next_node = it->second;
				rapidjson::Value subtree_value(rapidjson::kObjectType);
				to_json(next_node, subtree_value, allocator);
				subtree_actions.PushBack(subtree_value, allocator);
			}

			stat_value.AddMember("actions", subtree_actions, allocator);
		}

		return stat_value;
	}

	/*!
	 * \internal
	 *
	 * \brief Recursively merges \a lhs_node into \a rhs_node
	 * \param lhs_node Node which will be merged
	 * \param rhs_node Node in which this tree will be merged
	 * \param rhs_tree Tree in which this tree will be merged
	 */
	void merge_into(p_node_t lhs_node, call_tree_t::p_node_t rhs_node, call_tree_t& rhs_tree) const {
		if (lhs_node != root) {
			rhs_tree.set_node_start_time(rhs_node, get_node_start_time(lhs_node));
			rhs_tree.set_node_stop_time(rhs_node, get_node_stop_time(lhs_node));
		}

		for (auto it = nodes[lhs_node].links.begin(); it != nodes[lhs_node].links.end(); ++it) {
			int action_code = it->first;
			p_node_t lhs_next_node = it->second;
			p_node_t rhs_next_node = rhs_tree.add_new_link(rhs_node, action_code);
			merge_into(lhs_next_node, rhs_next_node, rhs_tree);
		}
	}

	/*!
	 * \internal
	 *
	 * \brief Allocates space for new node
	 * \param action_code Action code of new node
	 * \return Pointer to newly created node
	 */
	p_node_t new_node(int action_code) {
		nodes.emplace_back(action_code);
		return nodes.size() - 1;
	}

	/*!
	 * \brief Tree nodes
	 */
	std::vector<node_t> nodes;

	/*!
	 * \brief Available actions for monitoring
	 */
	const actions_set_t &actions_set;

	/*!
	 * \brief Key-Value map for storing arbitary user stats
	 */
	std::unordered_map<std::string, stat_value_t> stats;
};

/*!
 * \brief Concurrent version of time stats tree to handle simultanious updates
 */
class concurrent_call_tree_t {
public:
	/*!
	 * \brief Initializes call_tree with \a actions_set
	 * \param actions_set Set of available action for monitoring
	 */
	concurrent_call_tree_t(actions_set_t &actions_set): call_tree(actions_set) {}

	/*!
	 * \brief Gets ownership of time stats tree
	 */
	void lock() const {
		tree_mutex.lock();
	}

	/*!
	 * \brief Releases ownership of time stats tree
	 */
	void unlock() const {
		tree_mutex.unlock();
	}

	/*!
	 * \brief Returns inner time stats tree
	 * \return Inner time stats tree
	 */
	call_tree_t& get_call_tree() {
		return call_tree;
	}

	/*!
	 * \brief Returns copy of inner time stats tree
	 * \return Copy of inner time stats tree
	 */
	call_tree_t copy_call_tree() const {
		lock();
		call_tree_t call_tree_copy = call_tree;
		unlock();
		return call_tree_copy;
	}

private:
	/*!
	 * \brief Lock to handle concurrency during updates
	 */
	mutable std::mutex tree_mutex;

	/*!
	 * \brief Inner call_tree
	 */
	call_tree_t call_tree;
};


} // namespace react

#endif // REACT_CALL_TREE_HPP
