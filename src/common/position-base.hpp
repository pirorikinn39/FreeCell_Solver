#ifndef POSITION_BASE_H
#define POSITION_BASE_H

#include <algorithm>
#include <random>
#include <string>
#include <cstdint>
#include "card.hpp"
#include "bits.hpp"

#define STR(x) #x
#define E(l) " [internal error (line " STR(l) " in " __FILE__ ")]"

#define MAX_ACTION_SIZE 40 // 34 seems sufficient.
#define BAD_LOCATION    64
#define HOMECELL_SIZE    4
#define FREECELL_SIZE    4
#define TABLEAU_SIZE     8

#ifdef TEST_ZKEY
union Position_row {
  Position_row() noexcept { m_array_u64[6] = 0ULL; }
#else
struct Position_row {
#endif
  
public:  
  Position_row& operator=(const Position_row& o) noexcept {
    assert(o.ok());
#ifdef TEST_ZKEY
    if (this != &o) copy_n(o.m_array_u64, 7, m_array_u64);
#else
    if (this != &o) copy_n(o.m_array_i8, 52, m_array_i8);
#endif
    return *this; }
  void set_below(const Card& card, const Card& below) noexcept {  
    assert(card.is_card() && below);
    m_array_i8[card.get_id()] = below; }
  const Card& get_below(int id) const noexcept {
    assert(0 <= id && id <= 51 && ok());
    return m_array_i8[id]; }
  const Card& get_below(const Card& card) const noexcept { return get_below(card.get_id()); }
  bool operator==(const Position_row& o) const noexcept {
    assert(ok() && o.ok());
#ifdef TEST_ZKEY
    for (int i=0; i<7; ++i)
      if (m_array_u64[i] != o.m_array_u64[i]) return false;
#else
    for (int id=0; id<52; ++id)
      if (m_array_i8[id] != o.m_array_i8[id]) return false;
#endif
    return true; }
  bool ok() const noexcept {
    for (int id=0; id<52; ++id)
      if (! m_array_i8[id]) return false;
#ifdef TEST_ZKEY
    for (int id=52; id<56; ++id)
      if (m_array_i8[id].get_id() != 0) return false;
#endif
    return true; }

private:
#ifdef TEST_ZKEY
  uint64_t m_array_u64[7];
#endif
  Card m_array_i8[56];
};
  
class Action {
  char m_from, m_to;
    
public:
  Action() noexcept : m_from(BAD_LOCATION) {};
  Action(int from, int to) noexcept : m_from(from), m_to(to) {};
  bool ok() const noexcept {
    if (m_from == m_to)  return false;
    if (12 <= m_to && m_to <= 15) return (0 <= m_from && m_from <= 11);
    if ( 8 <= m_to && m_to <= 11) return (0 <= m_from && m_from <=  7);
    if ( 0 <= m_to && m_to <=  7) return (0 <= m_from && m_from <= 11);
    return false; }
  string gen_SN() const noexcept;
  int get_from() const noexcept { assert(ok()); return m_from; }
  int get_to() const noexcept { assert(ok()); return m_to; }
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
  Position_row m_row_data;
  Bits m_array_bits_column_card[TABLEAU_SIZE];
  Card m_array_column_top[TABLEAU_SIZE];
  Card m_array_homecell[HOMECELL_SIZE];
  Card m_array_freecell[FREECELL_SIZE];
  Bits m_bits_homecell;
  Bits m_bits_freecell;
  Bits m_bits_homecell_next;
  Bits m_bits_column_top;
  Bits m_array_bits_column_next[TABLEAU_SIZE];
  uint64_t m_zobrist_key;
  int m_ncard_freecell;
  int m_ncard_tableau;
  unsigned char m_array_location[CARD_SIZE];
  
  void update_array_card_below(const Card& card, const Card& below) noexcept {
    assert(card.is_card() && below && card != below);
    m_zobrist_key ^= table.get(card, m_row_data.get_below(card));
    m_row_data.set_below(card, below);
    m_zobrist_key ^= table.get(card, below); }
  int find_freecell_empty() const noexcept {
    int i;
    for (i=0; i<FREECELL_SIZE; ++i)
      if (! m_array_freecell[i]) break;
        return i; }
  int find_column_empty() const noexcept {
    int i;
    for (i=0; i<TABLEAU_SIZE; ++i)
      if (! m_array_column_top[i]) break;
    return i; }
  Card obtain_below_homecell(const Card& card) const noexcept {
    assert(card.is_card());
    if (card.rank() == 0) return Card::homecell();
    return Card(card.suit(), card.rank() - 1); }
  
public:
  const Position_row& get_row_data() const noexcept { return m_row_data; }
  bool ok() const noexcept;
};

#endif
