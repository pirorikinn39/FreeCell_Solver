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


#define MAX_F_COST_52F (DECK_SIZE * 2)

class Position : public Position_base {
  class Entry_tt {
#ifdef TEST_ZKEY
    Position_row m_row_data;
#endif
    unsigned char m_lower_bound;
    bool m_is_solved;
    Card m_candidate_homecell_next[HOMECELL_SIZE];

  public:
    Entry_tt(int lower_bound, const Position_row& row_data,
	     const Card* candidate_homecell_next) noexcept : 
#ifdef TEST_ZKEY
      m_row_data(row_data), m_lower_bound(lower_bound), m_is_solved(false)
#else
      m_lower_bound(lower_bound), m_is_solved(false)
#endif
    {
      copy_n(candidate_homecell_next, N_SUIT, m_candidate_homecell_next);
      assert(ok()); }
    void update_lower_bound(int lower_bound) noexcept {
      assert(m_lower_bound <= lower_bound && lower_bound <= MAX_F_COST_52F);
      m_lower_bound = lower_bound; }
    void set_solved() noexcept {
      assert(! m_is_solved);
      m_is_solved = true; }
    
    int get_lower_bound() const noexcept { return m_lower_bound; }
    bool is_solved() const noexcept { return m_is_solved; }
    const Card* get_candidate_homecell_next() const noexcept {
      return m_candidate_homecell_next; }
    void test_zobrist_key(const Position_row& row_data) const noexcept {
#ifdef TEST_ZKEY
      if (m_row_data == row_data) return;
      cerr << "Zobrist Key Conflict" << endl;
      terminate();
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
  int calc_h_cost_52f(Card*) noexcept;

public:
  explicit Position(int) noexcept;
  int get_ncard_deadlocked() const noexcept { return m_ncard_deadlocked; }
  uint64_t m_tt_size() const noexcept { return m_tt.size(); }
  int solve_52f(int bound_max) noexcept;
  int dfstt1(int, Action*, Entry_tt&) noexcept;
  int move_auto(Action*) noexcept;
  int move_auto_52f(Action*) noexcept;
  void unmake_n(const Action* path, int n) noexcept {
    for (int i=1; i<=n; ++i) unmake(*(path - i)); }
  void make(const Action&) noexcept;
  void unmake(const Action&) noexcept;
};

#endif
