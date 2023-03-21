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

#define MAX_SINGLE_SUIT_CYCLE_SIZE CARD_SIZE
#define MAX_TWO_SUIT_CYCLE_SIZE 1008

class Position : public Position_base {
public:
    class Two_suit_cycle {
    private:
        Card m_card1, m_card2;
        bool m_is_target;

    public:
        Two_suit_cycle(const Card& card1, const Card& card2, bool is_target) noexcept : m_card1(card1), m_card2(card2), m_is_target(is_target) {};
        Two_suit_cycle() noexcept : m_card1(Card()), m_card2(Card()), m_is_target(true) {};
        bool correct() const noexcept {
            if (m_card1 == m_card2) 
                return false;
            if (m_card1.suit() == m_card2.suit())
                return false;
            if (m_card1.rank() == 0)
                return false;
            if (m_card2.rank() == 0)
                return false;
            return true; 
        };
        void set_is_target(bool is_target) noexcept {
            m_is_target = is_target;
            assert(correct());
        };
        const Card& get_card1() const noexcept {
            assert(correct());
            return m_card1;
        };
        const Card& get_card2() const noexcept {
            assert(correct());
            return m_card2;
        };
        bool get_is_target() const noexcept {
            assert(correct());
            return m_is_target;
        };
    };


private:
    Bits m_array_bits_column_card[TABLEAU_COLUMN_SIZE];
    Card m_array_column_top[TABLEAU_COLUMN_SIZE];
    Card m_array_homecell[HOMECELL_SIZE];
    Card m_array_freecell[FREECELL_SIZE];
    Bits m_bits_homecell;
    Bits m_bits_freecell;
    Bits m_bits_homecell_next;
    Bits m_bits_column_top;
    Bits m_array_bits_column_next[TABLEAU_COLUMN_SIZE];
    uint64_t m_zobrist_key;
    int m_ncard_freecell;
    int m_ncard_tableau;
    unsigned char m_array_location[CARD_SIZE];
    Card m_array_single_suit_cycle[MAX_SINGLE_SUIT_CYCLE_SIZE];
    unsigned char m_nsingle_suit_cycle;
    Bits m_bits_single_suit_cycle;
    Two_suit_cycle m_array_two_suit_cycle[MAX_TWO_SUIT_CYCLE_SIZE];
    unsigned int m_ntwo_suit_cycle;
    unsigned char m_count_in_two_suit_cycle[CARD_SIZE];

    bool correct() const;
    void initialize(const Card (&)[TABLEAU_COLUMN_SIZE][64], const Card (&)[HOMECELL_SIZE], const Card (&)[FREECELL_SIZE]) noexcept;
    int find_freecell_empty() const noexcept {
        int i;
        for (i=0; i<FREECELL_SIZE; ++i)
            if (! m_array_freecell[i]) 
                break;
        return i; 
    };
    int find_column_empty() const noexcept {
        int i;
        for (i=0; i<TABLEAU_COLUMN_SIZE; ++i)
            if (! m_array_column_top[i]) 
                break;
        return i; 
    };
    Card obtain_below_homecell(const Card& card) const noexcept {
        if (card.rank() == 0) 
            return Card::homecell();
        return Card(card.suit(), card.rank() - 1); 
    };
    void update_array_card_below(const Card& card, const Card& below) noexcept {
        assert(card.is_card() && below.is_card_or_location() && (card != below));
        m_zobrist_key ^= Position::table.get(card, m_row_data.get_below(card));
        m_row_data.set_below(card, below);
        m_zobrist_key ^= Position::table.get(card, below);
    };

public:
    Position(int) noexcept;
    bool correct_Action(const Action&) const noexcept;
    uint64_t get_zobrist_key() const noexcept {
        assert(correct());
        return m_zobrist_key;   
    };
    int get_nsingle_suit_cycle() const noexcept {
        assert(correct());
        return m_nsingle_suit_cycle;
    };
    int get_ntwo_suit_cycle() const noexcept {
        assert(correct());
        return m_ntwo_suit_cycle;
    };
    int ncard_rest() const noexcept {
        assert(correct());
        return m_ncard_freecell + m_ncard_tableau;
    };
    void find_cycle() noexcept;
    void delete_cycle(const Card&) noexcept;
    void add_cycle(const Card&) noexcept;
    void check_cycle() const noexcept;
    int calc_h_cost() noexcept;
    int dfs(int, int, int) noexcept;
    int gen_actions(Action (&)[MAX_ACTION_SIZE]) const noexcept;
    int move_auto(Action*) noexcept;
    void make(const Action&) noexcept;
    void unmake(const Action&) noexcept;
};
  

#endif
