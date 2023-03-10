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
#include "Position.h"

#ifndef Solve_h
#define Solve_h

#define MAX_COST 1000

class Solve {
private:
	class Entry_tt {
	private:
		unsigned char m_h_cost;
		bool m_is_contained_route;
#ifdef TEST_ZKEY
		Position::Union_array_card m_union_array_card_below;
#endif
	public:
		Entry_tt(int, const Position&) noexcept;
		void set_h_cost(int) noexcept;
		void set_is_contained_route(bool) noexcept;
		int get_h_cost() const noexcept;
		int get_is_contained_route() const noexcept;
#ifdef TEST_ZKEY
		bool identify(const Position&) const noexcept;
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
    	if(it != m_tt.end()) {
#ifdef TEST_ZKEY
        	if(! it->second.identify(position)) {
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