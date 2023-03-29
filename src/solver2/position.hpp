#ifndef POSITION_H
#define POSITION_H

#include <iostream>
#include <iomanip>
#include <cassert>
#include <fstream>
#include <cmath>
#include <utility>
#include <string>
#include <string.h>
#include <unordered_map>
#include <bitset>
#include <algorithm>
#include <chrono>
#include "../common/position-base.hpp"
#include "../common/bits.hpp"


#define MAX_H_COST (DECK_SIZE * 2)

class Position : public Position_base {
  class Entry_tt {
#ifdef TEST_ZKEY
    Position_row m_row_data;
#endif
    unsigned char m_h_cost;
    bool m_is_solved;
    Card m_candidate_homecell_next[HOMECELL_SIZE];

  public:
    Entry_tt(int h_cost, const Position_row& row_data,
	     const Card* candidate_homecell_next) noexcept : 
#ifdef TEST_ZKEY
      m_h_cost(h_cost), m_is_solved(false), m_row_data(row_data)
#else
      m_h_cost(h_cost), m_is_solved(false)
#endif
    {
      copy_n(candidate_homecell_next, N_SUIT, m_candidate_homecell_next);
      assert(ok()); }
    void set_h_cost(int h_cost) noexcept {
      assert(0 <= h_cost && h_cost <= MAX_H_COST);
      m_h_cost = h_cost; }
    void set_solved() noexcept {
      assert(! m_is_solved);
      m_is_solved = true; }
    
    int get_h_cost() const noexcept { return m_h_cost; }
    bool is_solved() const noexcept { return m_is_solved; }
    const Card* get_candidate_homecell_next() const noexcept {
      return m_candidate_homecell_next; }
    bool test_zkey(const Position_row& row_data) const noexcept {
#ifdef TEST_ZKEY
      return m_row_data == row_data;
#else
      return true;
#endif
    }
    bool ok() const noexcept;
  };

private:
  unordered_map<uint64_t, Position::Entry_tt> m_tt;
  Bits m_bits_deadlocked;
  int m_ncard_deadlocked;
  unsigned char m_array_nbelow_not_deadlocked[DECK_SIZE];
  bool m_is_solved;

  bool ok() const noexcept;
  void one_suit_analysis(int &, Bits &, unsigned char *) const noexcept;
  int calc_nabove_not_deadlocked(const Card& card) const noexcept;
  int move_to_homecell(const Card&, Action*) noexcept;

public:
  explicit Position(int) noexcept;
  uint64_t get_zobrist_key_for_h() const noexcept { return m_zobrist_key; }
  int get_ncard_deadlocked() const noexcept { return m_ncard_deadlocked; }
  int calc_h_cost_52f(Card*) noexcept;
  uint64_t m_tt_size() const noexcept { return m_tt.size(); }
  int calc_h_cost() noexcept;
  int dfstt1(int, Action*, Entry_tt&) noexcept;
  int move_auto(Action*) noexcept;
  int move_auto_52f(Action*) noexcept;
  void unmake_n(const Action* path, int n) noexcept {
    for (int i=1; i<=n; ++i) unmake(*(path - i)); }
  void make(const Action&) noexcept;
  void unmake(const Action&) noexcept;
};

#endif
