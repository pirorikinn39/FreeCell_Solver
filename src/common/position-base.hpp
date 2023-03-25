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

#define MAX_ACTION_SIZE  40 // 34 seems sufficient.
#define MAX_PATH_SIZE   255
#define BAD_LOCATION     64
#define HOMECELL_SIZE     4
#define FREECELL_SIZE     4
#define TABLEAU_SIZE      8

union Position_row {
#ifdef TEST_ZKEY
  Position_row() noexcept { m_array_u64[6] = 0ULL; }
#endif
  
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
    m_array_i8[card.get_id()] = below.get_id(); }
  const Card get_below(int id) const noexcept {
    assert(0 <= id && id <= 51 && ok());
    return Card(m_array_i8[id]); }
  const Card get_below(const Card& card) const noexcept { return get_below(card.get_id()); }
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
      if (! Card(m_array_i8[id])) return false;
#ifdef TEST_ZKEY
    for (int id=52; id<56; ++id)
      if (m_array_i8[id] != 0) return false;
#endif
    return true; }

private:
#ifdef TEST_ZKEY
  uint64_t m_array_u64[7];
#endif
  int8_t m_array_i8[56];
};
  
class Action {
  Card m_card;
  char m_from, m_to;
    
public:
  Action() noexcept : m_from(BAD_LOCATION) {}
  Action(const Card& card, int from, int to) noexcept : m_card(card), m_from(from), m_to(to) {
    assert(ok()); }
  bool ok() const noexcept {
    if (! m_card.is_card()) return false;
    if (m_from == m_to) return false;
    if (12 <= m_to && m_to <= 15) return (0 <= m_from && m_from <= 11);
    if ( 8 <= m_to && m_to <= 11) return (0 <= m_from && m_from <=  7);
    if ( 0 <= m_to && m_to <=  7) return (0 <= m_from && m_from <= 11);
    return false; }
  Card get_card() const noexcept { return m_card; }
  int get_from() const noexcept { return m_from; }
  int get_to() const noexcept { return m_to; }
};

class Position_base {
protected:
  struct Table {
    uint64_t z_factor[DECK_SIZE][ID_SIZE];
    Table() noexcept {
      mt19937_64 mt(0);
      for (int i=0; i<DECK_SIZE; ++i)
	for (int j=0; j<ID_SIZE; ++j) z_factor[i][j] = mt(); }
    uint64_t get(const Card& card, const Card& below) const noexcept {
      assert(card.is_card() && below);
      return z_factor[card.get_id()][below.get_id()]; }
  };
  static Table table;
  Position_row m_row_data;
  Bits m_array_bits_pile_card[TABLEAU_SIZE];
  Card m_array_pile_top[TABLEAU_SIZE];
  Card m_array_homecell[HOMECELL_SIZE];
  Bits m_bits_homecell;
  Bits m_bits_freecell;
  Bits m_bits_homecell_next;
  Bits m_bits_pile_top;
  Bits m_array_bits_pile_next[TABLEAU_SIZE];
  uint64_t m_zobrist_key;
  int m_ncard_freecell;
  int m_ncard_tableau;
  unsigned char m_array_location[DECK_SIZE];
  
  void initialize(const Card (&)[TABLEAU_SIZE][64], const Card (&)[HOMECELL_SIZE],
		  const Card (&)[FREECELL_SIZE]) noexcept;
  void update_array_card_below(const Card& card, const Card& below) noexcept {
    assert(card.is_card() && below && card != below);
    m_zobrist_key ^= table.get(card, m_row_data.get_below(card));
    m_row_data.set_below(card, below);
    m_zobrist_key ^= table.get(card, below); }
  int find_pile_empty() const noexcept {
    int i;
    for (i=0; i<TABLEAU_SIZE; ++i)
      if (! m_array_pile_top[i]) break;
    return i; }
  Card obtain_below_homecell(const Card& card) const noexcept {
    assert(card.is_card());
    if (card.rank() == 0) return Card::homecell();
    return Card(card.suit(), card.rank() - 1); }
  
public:
  explicit Position_base(int) noexcept;
  
  void make(const Action& action) noexcept;
  void unmake(const Action& action) noexcept;
  
  int gen_actions(Action (&)[MAX_ACTION_SIZE]) const noexcept;
  bool is_legal(const Action&) const noexcept;
  bool ok() const noexcept;
  uint64_t get_zobrist_key() const noexcept { return m_zobrist_key; }
  int ncard_rest() const noexcept { return m_ncard_freecell + m_ncard_tableau; }
  const Position_row& get_row_data() const noexcept { return m_row_data; }
};

string gen_solution(int, const Action *, int) noexcept;
#endif
