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


#define MAX_H_COST (DECK_SIZE * 2)

class Position : public Position_base {

private:
    class Entry_tt {
    private:
      unsigned char m_h_cost;
      bool m_is_decided; // delete
      Card m_candidate_homecell_next[HOMECELL_SIZE]; 
#ifdef TEST_ZKEY
      Position_row m_row_data;
#endif
        bool correct() const;

    public:
      Entry_tt(int h_cost, const Position& position, const Card* candidate_homecell_next) noexcept : m_h_cost(h_cost), m_is_decided(false) {
            assert((h_cost >= 0) && (h_cost < 256));
            copy_n(candidate_homecell_next, N_SUIT, m_candidate_homecell_next);
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
      bool identify(const Position& position) const noexcept { // delete
            assert(correct());
            return m_row_data == position.get_row_data(); }
#endif      
    };

private:
  Bits m_bits_deadlocked;
  unsigned char m_ncard_deadlocked;
  unsigned char m_array_ncard_not_deadlocked_below_and[DECK_SIZE]; // delete _and
  bool m_is_solved;
  unordered_map<uint64_t, Position::Entry_tt> m_tt;

  bool correct() const noexcept;
  void initialize() noexcept;
  void back_to_parent(const Action* history, int naction) noexcept {
    for (int i=1; i<=naction; ++i) unmake(*(history - i)); }

public:
  explicit Position(int) noexcept;
    bool correct_Action(const Action&) const noexcept;
    uint64_t get_zobrist_key_for_h() const noexcept { return m_zobrist_key; }
    int get_ncard_deadlocked() const noexcept { return m_ncard_deadlocked; }
    int ncard_rest_for_h() const noexcept { return m_ncard_freecell + m_ncard_tableau; }
    int calc_h_cost_52f(Card*) noexcept;
    int obtain_ncard_not_deadlocked_above(const Card& card) const noexcept {
        return m_array_ncard_not_deadlocked_below_and[m_array_pile_top[m_array_location[card.get_id()]].get_id()] - m_array_ncard_not_deadlocked_below_and[card.get_id()]; }
    uint64_t m_tt_size() const noexcept { return m_tt.size(); }
    int calc_h_cost() noexcept;
    int dfstt1(int, Action*, Entry_tt&) noexcept;
    int move_to_homecell_next(const Card&, Action*) noexcept;
    int move_auto(Action*) noexcept;
    int move_auto_52f(Action*) noexcept;
    void make(const Action&) noexcept;
    void unmake(const Action&) noexcept;
};

#endif
