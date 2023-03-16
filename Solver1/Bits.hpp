#include <iostream>
#include <cassert>
#include <random>
#include <fstream>
#include <cmath>
#include <utility>
#include <string>
#include "../Common/Card.hpp"

#ifndef Bits_h
#define Bits_h

class Bits {
private:
    struct Table {
        uint64_t bits[CARD_SIZE];
        uint64_t bits_next[CARD_SIZE];
        uint64_t bits_placeable[CARD_SIZE];
        uint64_t bits_same_suit_small_rank[CARD_SIZE];
        uint64_t bits_different_suit[CARD_SIZE];
        Table() noexcept {
            for (int id=0; id<CARD_SIZE; ++id) {
                bits[id] = 1ULL << (63 - id);
                bits_next[id] = bits_placeable[id] = bits_same_suit_small_rank[id] = 0ULL;
                bits_different_suit[id] = 0xfffffffffffff000ULL;

                Card next = Card::next(id);
                if (next.is_card())
                    bits_next[id] = 1ULL << (63 - next.get_id());

                Card prev = Card::prev(id);
                if (prev.is_card()) {
                    auto suits = Card::suits_xcolor(id);
                    int id1 = Card(suits.first,  prev.rank()).get_id();
                    int id2 = Card(suits.second, prev.rank()).get_id();
                    bits_placeable[id] |= 1ULL << (63 - id1);
                    bits_placeable[id] |= 1ULL << (63 - id2); 
                }

                int suit = Card(id).suit();
                if (prev.is_card()) {
                    for (int rank=0; rank<=prev.rank(); ++rank) {
                        Card card = Card(suit, rank);
                        bits_same_suit_small_rank[id] |= 1ULL << (63 - card.get_id());
                    }
                }

                for (int rank=0; rank<RANK_SIZE; ++rank) {
                    Card card = Card(suit, rank);
                    bits_different_suit[id] ^= 1ULL << (63 - card.get_id());
                }
            }
        }
    };
    static Table table;
    uint64_t m_bits;

    constexpr Bits(uint64_t bits) noexcept : m_bits(bits) {};
    constexpr bool correct() const noexcept { 
        return ! (m_bits & 0xfffULL); 
    };
#if defined(__GNUC__)
    int count_popu() const noexcept {
        assert(correct());
        return __builtin_popcountll(m_bits); 
    };
    int count_lz() const noexcept {
        assert(correct());
        if (! m_bits) 
            return -1;
        return __builtin_clzll(m_bits);
    };

#else
    int count_popu() const noexcept {
        assert(! corrupt());
        uint64_t bits = m_bits;
        int popu = 0;
        for (; bits; bits&=bits-1U) 
            popu += 1U;
        return popu;             
    };
    int count_lz() const noexcept {
        assert(! corrupt());
        if (! m_bits) 
            return -1;
    
        static constexpr char tbl[16] = { 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };
        uint64_t u, bits = m_bits;
        int lz = -1;
        u = bits >> 32; 
        if (u) { 
            lz += 32; 
            bits = u; 
        }
        u = bits >> 16; 
        if (u) { 
            lz += 16; 
            bits = u; 
        }
        u = bits >> 8; 
        if (u) {
            lz += 8; 
            bits = u; 
        }
        u = bits >> 4; 
        if (u) { 
            lz += 4; 
            bits = u; 
        }
        assert(bits < 16U);
        lz += tbl[bits];
        assert((12 <= lz) && (lz <= 63));
        return 63 - lz; 
    };
#endif

public:
    constexpr Bits() noexcept : m_bits(0ULL) {};
    Bits(int id) noexcept : m_bits(Bits::table.bits[id]) {};
    Bits(const Card& card) noexcept : m_bits(Bits::table.bits[card.get_id()]) {};
    uint64_t get_bits() const noexcept {
        assert(correct());
        return m_bits; 
    };
    static constexpr Bits full() noexcept { 
        return Bits(static_cast<uint64_t>(0xfffffffffffff000ULL)); 
    };
    static Bits next(int id) noexcept {
        assert((0 <= id) && (id <= 51));
        return Bits(Bits::table.bits_next[id]);
    };
    static Bits next(const Card& card) noexcept {
        return next(card.get_id());
    };
    static Bits placeable(int id) noexcept {
        assert((0 <= id) && (id <= 51));
        return Bits(Bits::table.bits_placeable[id]);
    };
    static Bits placeable(const Card& card) noexcept {
        return placeable(card.get_id()); 
    };
    static Bits same_suit_small_rank(int id) noexcept {
        assert((0 <= id) && (id <= 51));
        return Bits(Bits::table.bits_same_suit_small_rank[id]); 
    };
    static Bits same_suit_small_rank(const Card& card) noexcept {
        return same_suit_small_rank(card.get_id()); 
    };
    static Bits different_suit(int id) noexcept {
        assert((0 <= id) && (id <= 51));
        return Bits(Bits::table.bits_different_suit[id]);
    };
    static Bits different_suit(const Card& card) noexcept {
        return different_suit(card.get_id());
    };
    void alter(int id) noexcept {
        assert(correct());
        assert((0 <= id) && (id <= 51));
        m_bits ^= Bits::table.bits[id]; 
    };
    void alter(const Card& card) noexcept {
        alter(card.get_id()); 
    };
    void set_bit(int id) noexcept {
        assert(! (m_bits & (1ULL << (63 - id))));
        alter(id); 
    };
    void set_bit(const Card& card) noexcept {
        set_bit(card.get_id()); 
    };
    void clear_bit(int id) noexcept {
        assert(m_bits & (1ULL << (63 - id)));
        alter(id); 
    };
    void clear_bit(const Card& card) noexcept {
        clear_bit(card.get_id());
    };
    void clear() noexcept {
        assert(correct());
        m_bits = 0ULL; 
    };
    Card pop() noexcept {
        if (! m_bits) 
            return Card(-1);
        int id = count_lz();
        clear_bit(id);
        return Card(id);
    };
    int popu() const noexcept {
        return count_popu();
    };
    bool is_set_bit(int id) const noexcept {
        assert(correct());
        assert((0 <= id) && (id <= 51));
        return m_bits & Bits::table.bits[id]; 
    };
    bool is_set_bit(const Card& card) const noexcept {
        return is_set_bit(card.get_id());
    };
    operator bool() const noexcept {
        assert(correct());
        return m_bits != 0ULL; 
    };
    bool operator!() const noexcept {
        assert(correct());
        return m_bits == 0ULL; 
    };
    Bits operator|=(const Bits& bits) noexcept {
        assert(correct() && bits.correct());
        m_bits |= bits.m_bits;
        return *this; 
    };
    Bits operator^=(const Bits& bits) noexcept {
        assert(correct() && bits.correct());
        m_bits ^= bits.m_bits;
        return *this; 
    };
    bool operator==(const Bits& bits) const noexcept {
        assert(correct() && bits.correct());
        return m_bits == bits.m_bits; 
    };
    bool operator!=(const Bits& bits) const noexcept {
        assert(correct() && bits.correct());
        return ! operator==(bits);
    };
    Bits operator&(const Bits& bits) const noexcept {
        assert(correct() && bits.correct());
        return Bits(m_bits & bits.m_bits); 
    };
    Bits operator|(const Bits& bits) const noexcept {
        assert(correct() && bits.correct());
        return Bits(m_bits | bits.m_bits); 
    };
    Bits operator^(const Bits& bits) noexcept {
        assert(correct() && bits.correct());
        return Bits(m_bits ^ bits.m_bits); 
    };
};

#endif