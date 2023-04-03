#ifndef BITS_H
#define BITS_H

#include <cassert>
#include <cstdint>
#include "card.hpp"

class Bits {
  struct Table {
    uint64_t bits[DECK_SIZE];
    uint64_t bits_next[DECK_SIZE];
    uint64_t bits_placeable[DECK_SIZE];
    uint64_t bits_same_suit_small_rank[DECK_SIZE];
    uint64_t bits_same_suit[N_SUIT];
    Table() noexcept {
      for (int id=0; id<DECK_SIZE; ++id) {
	bits[id] = 1ULL << (63 - id);
	
	Card next = Card::next(id);
	if (next.is_card()) bits_next[id] = 1ULL << (63 - next.get_id());
	
	Card prev = Card::prev(id);
	if (prev.is_card()) {
	  auto suits = Card::suits_xcolor(id);
	  int id1 = Card(suits.first,  prev.rank()).get_id();
	  int id2 = Card(suits.second, prev.rank()).get_id();
	  bits_placeable[id] |= 1ULL << (63 - id1);
	  bits_placeable[id] |= 1ULL << (63 - id2); }
	
	for (Card card=prev; card.is_card(); card=card.prev())
	  bits_same_suit_small_rank[id] |= 1ULL << (63 - card.get_id()); }
      for (int suit = 0; suit < N_SUIT; ++suit)
	for (Card card(suit, 0); card; card=card.next())
	  bits_same_suit[suit] ^= 1ULL << (63 - card.get_id()); } };
  static Table table;
  uint64_t m_bits;

  constexpr Bits(uint64_t bits) noexcept : m_bits(bits) {}
#if defined(__GNUC__)
  int count_popu() const noexcept { assert(ok()); return __builtin_popcountll(m_bits); }
  int count_lz() const noexcept {
    assert(ok());
    if (! m_bits) return -1;
    return __builtin_clzll(m_bits); }
#else
  int count_popu() const noexcept {
    assert(ok());
    uint64_t bits = m_bits;
    int popu = 0;
    for (; bits; bits &= bits-1ULL) popu += 1U;
    return popu; }
  int count_lz() const noexcept {
    assert(ok());
    if (! m_bits) return -1;
    
    static constexpr char tbl[16] = { 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };
    uint64_t u, bits = m_bits;
    int lz = -1;
    u = bits >> 32; if (u) { lz += 32; bits = u; }
    u = bits >> 16; if (u) { lz += 16; bits = u; }
    u = bits >>  8; if (u) { lz +=  8; bits = u; }
    u = bits >>  4; if (u) { lz +=  4; bits = u; }
    
    assert(bits < 16U);
    lz += tbl[bits];
    assert(12 <= lz && lz <= 63);
    return 63 - lz; }
#endif

public:
  constexpr Bits() noexcept : m_bits(0ULL) {}
  Bits(int id) noexcept : m_bits(Bits::table.bits[id]) {}
  Bits(const Card& card) noexcept : m_bits(Bits::table.bits[card.get_id()]) {}
  constexpr Bits(const Bits& o) noexcept : m_bits(o.m_bits) {}
  
  static Bits next(const Card& card) noexcept { return next(card.get_id()); }
  static Bits placeable(const Card& card) noexcept { return placeable(card.get_id()); }
  static constexpr Bits full() noexcept      { return Bits(UINT64_C(0xfffffffffffff000)); }
  static constexpr Bits not_kings() noexcept { return Bits(UINT64_C(0xfff7ffbffdffe000)); }
  static constexpr Bits kings() noexcept     { return Bits(UINT64_C(0x0008004002001000)); }
  static Bits next(int id) noexcept {
    assert(0 <= id && id <= 51);
    return Bits(Bits::table.bits_next[id]); }
  static Bits placeable(int id) noexcept {
    assert(0 <= id && id <= 51);
    return Bits(Bits::table.bits_placeable[id]); }
  static Bits same_suit_small_rank(int id) noexcept {
    assert(0 <= id && id <= 51);
    return Bits(Bits::table.bits_same_suit_small_rank[id]); }
  static Bits same_suit_small_rank(const Card& card) noexcept {
    return same_suit_small_rank(card.get_id()); }
  static Bits same_suit(int suit) noexcept {
    assert(0 <= suit && suit < N_SUIT);
    return Bits(Bits::table.bits_same_suit[suit]); }

  constexpr bool ok() const noexcept { return ! (m_bits & 0xfffULL); }
  constexpr uint64_t get_bits() const noexcept { return m_bits; }
  constexpr operator bool() const noexcept { return m_bits != 0ULL; }
  constexpr bool operator!() const noexcept { return m_bits == 0ULL; }
  
  int popu() const noexcept { assert(ok()); return count_popu(); }
  bool is_set_bit(int id) const noexcept {
    assert(ok() && 0 <= id && id <= 51);
    return m_bits & Bits::table.bits[id]; }
  bool is_set_bit(const Card& card) const noexcept {
    return is_set_bit(card.get_id()); }
  bool operator==(const Bits& bits) const noexcept {
    assert(ok() && bits.ok());
    return m_bits == bits.m_bits; }
  bool operator!=(const Bits& bits) const noexcept { return ! operator==(bits); }
  Bits operator&(const Bits& bits) const noexcept {
    assert(ok() && bits.ok());
    return Bits(m_bits & bits.m_bits); }
  Bits operator|(const Bits& bits) const noexcept {
    assert(ok() && bits.ok());
    return Bits(m_bits | bits.m_bits); }
  Bits operator^(const Bits& bits) const noexcept {
    assert(ok() && bits.ok());
    return Bits(m_bits ^ bits.m_bits); }

  void clear() noexcept { assert(ok()); m_bits = 0ULL; }
  void alter(int id) noexcept {
    assert(ok() && (0 <= id) && (id <= 51));
    m_bits ^= Bits::table.bits[id]; };
  void alter(const Card& card) noexcept { alter(card.get_id()); }
  void set_bit(int id) noexcept {
    assert(! (m_bits & (1ULL << (63 - id))));
    alter(id); }
  void set_bit(const Card& card) noexcept { set_bit(card.get_id()); }
  void clear_bit(int id) noexcept {
    assert(m_bits & (1ULL << (63 - id)));
    alter(id); }
  void clear_bit(const Card& card) noexcept { clear_bit(card.get_id()); }
  Card pop() noexcept {
    assert(ok());
    if (! m_bits) return Card(-1);
    int id = count_lz();
    clear_bit(id);
    return Card(id); }
  Bits operator|=(const Bits& bits) noexcept {
    assert(ok() && bits.ok());
    m_bits |= bits.m_bits;
    return *this; }
  Bits operator^=(const Bits& bits) noexcept {
    assert(ok() && bits.ok());
    m_bits ^= bits.m_bits;
    return *this; }
};

#endif
