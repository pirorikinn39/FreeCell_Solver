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

#define MAX_SINGLE_SUIT_CYCLE_SIZE DECK_SIZE
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
    Card m_array_single_suit_cycle[MAX_SINGLE_SUIT_CYCLE_SIZE];
    unsigned char m_nsingle_suit_cycle;
    Bits m_bits_single_suit_cycle;
    Two_suit_cycle m_array_two_suit_cycle[MAX_TWO_SUIT_CYCLE_SIZE];
    unsigned int m_ntwo_suit_cycle;
    unsigned char m_count_in_two_suit_cycle[DECK_SIZE];

  bool correct() const noexcept;

public:
  explicit Position(int) noexcept;
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
