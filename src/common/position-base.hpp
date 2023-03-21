#ifndef POSITION_BASE_H
#define POSITION_BASE_H

#include <algorithm>
#include <random>
#include <cstdint>
#include "card.hpp"

#define STR(x) #x
#define E(l) "internal error (line " STR(l) " in " __FILE__ ")"

#define MAX_ACTION_SIZE 40 // 34 seems sufficient.

union Position_row {
#ifdef TEST_ZKEY
  Position_row() noexcept { m_array_u64[6] = 0ULL; }
  Position_row& operator=(const Position_row& o) noexcept {
    assert(o.ok());
    if (this != &o) copy_n(o.m_array_u64, 7, m_array_u64);
    return *this; }
  bool operator==(const Position_row& o) const noexcept {
    assert(ok() && o.ok());
    for (int i=0; i<7; ++i)
      if (m_array_u64[i] != o.m_array_u64[i]) return false;
    return true; }
  bool ok() const noexcept {
    for (int id=0; id<52; ++id)
      if (! m_array_i8[id]) return false;
    for (int id=52; id<56; ++id)
      if (m_array_i8[id].get_id() != 0) return false;
    return true; }

private:
  uint64_t m_array_u64[7];
#else
  Position_row& operator=(const Position_row& o) noexcept {
    assert(o.ok());
    if (this != &o) copy_n(o.m_array_i8, 52, m_array_i8);
    return *this; }
  bool operator==(const Position_row& o) const noexcept {
    assert(ok() && o.ok());
    for (int id=0; id<52; ++id)
      if (m_array_i8[id] != o.m_array_i8[id]) return false;
    return true; }
  bool ok() const noexcept {
    for (int id=0; id<52; ++id)
      if (! m_array_i8[id]) return false;
    return true; }
#endif
  
public:  
  void set_below(const Card& card, const Card& below) noexcept {  
    assert(card.is_card() && below);
    m_array_i8[card.get_id()] = below; }
  const Card& get_below(int id) const noexcept {
    assert(0 <= id && id <= 51 && ok());
    return m_array_i8[id]; }
  const Card& get_below(const Card& card) const noexcept { return get_below(card.get_id()); }

private:
  Card m_array_i8[56];
};

class Position_base {
protected:
  struct Table {
    uint64_t z_factor[CARD_SIZE][ID_SIZE];
    Table() noexcept {
      mt19937_64 mt(0);
      for (int i=0; i<CARD_SIZE; ++i)
	for (int j=0; j<ID_SIZE; ++j) z_factor[i][j] = mt(); }
    uint64_t get(const Card& card, const Card& below) const noexcept {
      assert(card.is_card() && below);
      return z_factor[card.get_id()][below.get_id()]; }
  };
  static Table table;
  static constexpr int bad_location = 64;
#ifdef TEST_ZKEY
  Position_row m_row_data;
#else
  Card m_array_card_below[CARD_SIZE];
#endif

public:
#ifdef TEST_ZKEY
  const Position_row& get_row_data() const noexcept { return m_row_data; }
#endif
};

#endif
