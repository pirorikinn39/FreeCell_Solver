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


#define MAX_H_COST (CARD_SIZE * 2)

class Position : public Position_base {

private:
    class Entry_tt {
    private:
        unsigned char m_h_cost;
        bool m_is_decided;
        Card m_candidate_homecell_next[HOMECELL_SIZE];
#ifdef TEST_ZKEY
      Position_row m_row_data;
#endif
        bool correct() const;

    public:
      Entry_tt(int h_cost, const Position& position, const Card* candidate_homecell_next) noexcept : m_h_cost(h_cost), m_is_decided(false) {
            assert((h_cost >= 0) && (h_cost < 256));
            copy_n(candidate_homecell_next, SUIT_SIZE, m_candidate_homecell_next);
#ifdef TEST_ZKEY
            m_row_data = position.get_row_data();
#endif
            assert(correct());
        };
        void set_h_cost(int h_cost) noexcept {
            assert((h_cost >= 0) && (h_cost < 256));
            m_h_cost = h_cost;
            assert(correct());
        };
        void set_is_decided(bool is_decided) noexcept {
            assert(! m_is_decided);
            m_is_decided = is_decided;
            assert(correct());
        };
        int get_h_cost() const noexcept {
            assert(correct());
            return m_h_cost;
        };
        bool get_is_decided() const noexcept {
            assert(correct());
            return m_is_decided;
        };
        const Card* get_candidate_homecell_next() const noexcept {
            assert(correct());
            return m_candidate_homecell_next;
        };
#ifdef TEST_ZKEY
        bool identify(const Position& position) const noexcept {
            assert(correct());
            return m_row_data == position.get_row_data(); }
#endif      
    };

public:

    class Action_for_h {
    private:
        Card m_card;
        unsigned char m_from, m_to;

        Action_for_h(const Card& card, int from, int to) noexcept : m_card(card), m_from(from), m_to(to) {};

    public:
      Action_for_h() noexcept : m_from(BAD_LOCATION) {};
        bool correct() const noexcept {
            if (! (m_card.is_card()))
                return false;
            if (! ((m_card.get_id() >= 0) && (m_card.get_id() <= 51)))
                return false;
            if (m_from == m_to) 
                return false;
            if ((m_from >= 0) && (m_from <= 7))
                return (m_to == 8) || ((m_to >= 12) && (m_to <= 15));
            if ((m_from >= 8) && (m_from <= 11))
                return (m_to >= 12) && (m_to <= 15);
            return false; 
        };
        const Card& get_card() const noexcept {
            assert(correct());
            return m_card;
        };
        int get_from() const noexcept {
            assert(correct());
            return m_from;
        };
        int get_to() const noexcept {
            assert(correct());
            return m_to;
        };
        friend Position;
    };

private:
  Bits m_bits_deadlocked;
  unsigned char m_ncard_deadlocked;
  unsigned char m_array_ncard_not_deadlocked_below_and[CARD_SIZE];
  bool m_is_solved;
  unordered_map<uint64_t, Position::Entry_tt> m_tt;

  bool correct() const noexcept;
  bool correct_for_h() const;
    void initialize(const Card (&)[TABLEAU_SIZE][64], const Card (&)[HOMECELL_SIZE], const Card (&)[FREECELL_SIZE]) noexcept;

public:
    Position(int) noexcept;
    bool correct_Action(const Action&) const noexcept;
    bool correct_Action(const Position::Action_for_h&) const noexcept;
    uint64_t get_zobrist_key() const noexcept {
        assert(correct());
        return m_zobrist_key;
    };
    uint64_t get_zobrist_key_for_h() const noexcept {
        assert(correct_for_h());
        return m_zobrist_key;
    };
    int get_ncard_deadlocked() const noexcept {
        assert(correct_for_h());
        return m_ncard_deadlocked;
    };
    int ncard_rest() const noexcept {
        assert(correct());
        return m_ncard_freecell + m_ncard_tableau;
    };
    int ncard_rest_for_h() const noexcept {
        assert(correct_for_h());
        return m_ncard_freecell + m_ncard_tableau;
    };
    int obtain_lower_h_cost(Card*) noexcept;
    int obtain_ncard_not_deadlocked_above(const Card& card) const noexcept {
        assert(correct_for_h());
        return m_array_ncard_not_deadlocked_below_and[m_array_column_top[m_array_location[card.get_id()]].get_id()] - m_array_ncard_not_deadlocked_below_and[card.get_id()];  
    };
    uint64_t m_tt_size() const noexcept {
        assert(correct_for_h());
        return m_tt.size();
    };
    bool check_goal() const noexcept {
        assert(correct_for_h());
        return m_bits_homecell == Bits::full();
    };
    int calc_h_cost() noexcept;
    int dfstt1(int, Action_for_h*, Position::Entry_tt&) noexcept;
    int gen_actions(Action (&)[MAX_ACTION_SIZE]) const noexcept;
    int move_to_homecell_next(const Card&, Action_for_h*) noexcept;
    int move_auto(Action*) noexcept;
    int move_auto(Position::Action_for_h*) noexcept;
    void back_to_parent(const Position::Action_for_h* history, int naction) noexcept {
        for (int i=1; i<=naction; ++i)
            unmake(*(history - i));
        assert(correct_for_h());
    };
    void make(const Action&) noexcept;
    void make(const Position::Action_for_h& action) noexcept;
    void unmake(const Action&) noexcept;
    void unmake(const Position::Action_for_h& action) noexcept;
};

#endif
