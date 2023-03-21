#include <iostream>
#include <cassert>
#include <random>
#include <fstream>
#include <cmath>
#include <utility>
#include <string>
#include <chrono>
#include <unordered_map>
#include <climits>
#include "position.hpp"

#ifndef Solve_h
#define Solve_h

class Solve {
private:
	class Entry_tt {
	private:
		unsigned char m_h_cost;
		bool m_is_contained_route;
#ifdef TEST_ZKEY
	  Position_row m_union_array_card_below;
#endif
	public:
		Entry_tt(int h_cost, const Position& position) noexcept : m_h_cost(h_cost), m_is_contained_route(false) {
			assert((h_cost >= 0) && (h_cost <= UCHAR_MAX));
#ifdef TEST_ZKEY
			m_union_array_card_below = position.get_row_data();
#endif
		};
		void set_h_cost(int h_cost) noexcept {
			assert((h_cost >= 0) && (h_cost <= UCHAR_MAX));
    		m_h_cost = h_cost;
		};
		void set_is_contained_route(bool is_contained_route) noexcept {
			m_is_contained_route = is_contained_route;
		};
		int get_h_cost() const noexcept {
			return m_h_cost;
		};
		int get_is_contained_route() const noexcept {
			return m_is_contained_route;
		};
#ifdef TEST_ZKEY
		bool identify(const Position& position) const noexcept {
		  return m_union_array_card_below == position.get_row_data();
		};
#endif		
	};

	int m_game_id;
	Position m_position;
	bool m_is_solved;
	Position::Action m_answer[UCHAR_MAX];
	unordered_map<uint64_t, Solve::Entry_tt> m_tt;

	int dfstt1(int, int, Position::Action*, Solve::Entry_tt&) noexcept;
	pair<int, Solve::Entry_tt&> lookup(Position& position) noexcept {
		auto it = m_tt.find(position.get_zobrist_key());
    	if (it != m_tt.end()) {
#ifdef TEST_ZKEY
        	if (! it->second.identify(position)) {
            	cerr << "Zobrist Key Conflict" << endl;
            	terminate(); 
        	}
#endif
        	return {it->second.get_h_cost(), it->second};
    	}
    	else {
        	int h_cost = position.calc_h_cost();
        	auto pair = m_tt.emplace(piecewise_construct, forward_as_tuple(position.get_zobrist_key()), forward_as_tuple(h_cost, position));
        	assert(pair.second);
        	return {h_cost, (pair.first)->second};
    	}
	};

public:
	Solve(int) noexcept;
};

#endif
