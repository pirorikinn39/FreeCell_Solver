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
#include <algorithm>
#include "../common/position-base.hpp"
#include "../common/bits.hpp"

#define MAX_SINGLE_SUIT_CYCLE_SIZE (N_SUIT * (N_RANK-1))
#define MAX_TWO_SUIT_CYCLE_SIZE    (N_SUIT * (N_RANK-1) * (N_SUIT-1) * (N_RANK-1) / 2)

class Two_suit_cycle {
  Card m_card1, m_card2;
  bool m_is_target;

public:
  Two_suit_cycle(const Card& card1, const Card& card2, bool is_target) noexcept :
    m_card1(card1), m_card2(card2), m_is_target(is_target) { assert(ok()); }
  Two_suit_cycle() noexcept : m_is_target(true) {}
  bool ok() const noexcept {
    if (m_card1.suit() == m_card2.suit()) return false;
    if (m_card1.rank() == 0) return false;
    if (m_card2.rank() == 0) return false;
    return true; }
  void set_target() noexcept { m_is_target = true; }
  void clear_target() noexcept { m_is_target = false; }
  const Card& get_card1() const noexcept { return m_card1; }
  const Card& get_card2() const noexcept { return m_card2; }
  bool is_target() const noexcept { return m_is_target; }
  bool is_included(const Card& card) const noexcept {
    return (card == m_card1 || card == m_card2); }
};
inline bool operator==(const Two_suit_cycle& c1, const Two_suit_cycle& c2) noexcept {
  return ((c1.get_card1() == c2.get_card1() && c1.get_card2() == c2.get_card2())
	  || (c1.get_card2() == c2.get_card1() && c1.get_card1() == c2.get_card2())); }

class Position : public Position_base {
  Bits m_bits_single_suit_cycle;
  int m_nsingle_suit_cycle;
  int m_ntwo_suit_cycle;
  Card m_array_single_suit_cycle[MAX_SINGLE_SUIT_CYCLE_SIZE];
  Two_suit_cycle m_array_two_suit_cycle[MAX_TWO_SUIT_CYCLE_SIZE];
  unsigned char m_count_in_two_suit_cycle[DECK_SIZE];
  
  bool ok() const noexcept;

public:
  explicit Position(int) noexcept;
  int get_nsingle_suit_cycle() const noexcept { return m_nsingle_suit_cycle; }
  int get_ntwo_suit_cycle() const noexcept { return m_ntwo_suit_cycle; }
  void delete_cycle(const Card&) noexcept;
  void add_cycle(const Card&) noexcept;
  void check_cycle() const noexcept;
  int calc_h_cost() noexcept;
  int dfs(int, int, int) noexcept;
  int move_auto(Action*) noexcept;
  void make(const Action&) noexcept;
  void unmake(const Action&) noexcept;
};
  

#endif
