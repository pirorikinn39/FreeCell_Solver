#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <fstream>
#include <cmath>
#include <utility>
#include <string>
#include <string.h>
#include <unordered_map>
#include <bitset>
#include <algorithm>
#include <chrono>
#include "Bits.h"

#ifndef Position_h
#define Position_h

#define MAX_ACTION_SIZE 35
#define MAX_H_COST (CARD_SIZE * 2)

#define STR(x) #x
#define E(l) "internal error (line " STR(l) " in " __FILE__ ")"

class Position {
private:
    struct Table {
        uint64_t z_factor[CARD_SIZE][ID_SIZE];
        Table() noexcept {
            mt19937_64 mt(0);
            for (int i=0; i<CARD_SIZE; ++i)
                for (int j=0; j<ID_SIZE; ++j)
                    z_factor[i][j] = mt(); 
        }
        uint64_t get(const Card& card, const Card& below) const noexcept {
            assert(card && below);
            return z_factor[card.get_id()][below.get_id()]; 
        } 
    };

#ifdef TEST_ZKEY
public:
    union Union_array_card {
        Card m_array_card_below[56] = {};
        uint64_t m_array_card_belows[7];

        void set_array_card_below(int id, const Card& below) noexcept {  
            assert((0 <= id) && (id <= 51));
            assert(Card(id) && below);
            m_array_card_below[id] = below;
        };
        void set_array_card_below(const Card& card, const Card& below) noexcept {  
            assert(card && below);
            set_array_card_below(card.get_id(), below);
        };
        void set_array_card_belows(const uint64_t* array_card_belows) noexcept {
            assert(array_card_belows);
            copy_n(array_card_belows, 7, m_array_card_belows);   
        };
        void set_array_card_belows(const Position& position) noexcept {
            assert(position.correct());
            set_array_card_belows(position.m_union_array_card_below.m_array_card_belows);  
        };
        const Card& get_array_card_below(int id) const noexcept {  
            assert((0 <= id) && (id <= 51));
            return m_array_card_below[id];
        };
        const Card& get_array_card_below(const Card& card) const noexcept {  
            return get_array_card_below(card.get_id());
        };
        const Card* get_array_card_below() const noexcept {                        
            return m_array_card_below;
        };
        const uint64_t* get_array_card_belows() const noexcept {                        
            return m_array_card_belows;
        };
        bool identify(const Position::Union_array_card& union_array_card) const noexcept {  
            for(int i=0; i<7; ++i)
                if(m_array_card_belows[i] != union_array_card.m_array_card_belows[i])
                    return false;
            return true;
        };
        bool identify(const Position& position) const noexcept {  
            return identify(position.m_union_array_card_below);
        };
    };
#endif

private:
    class Entry_tt {
    private:
        unsigned char m_h_cost;
        bool m_is_decided;
        Card m_candidate_homecell_next[HOMECELL_SIZE];
#ifdef TEST_ZKEY
        Position::Union_array_card m_union_array_card_below;
#endif

        bool correct() const;

    public:
        Entry_tt(int, const Position&, const Card*) noexcept;
        void set_h_cost(int) noexcept;
        void set_is_decided(bool) noexcept;
        int get_h_cost() const noexcept;
        bool get_is_decided() const noexcept;
        const Card* get_candidate_homecell_next() const noexcept;
#ifdef TEST_ZKEY
        bool identify(const Position&) const noexcept;
#endif      
    };

public:
    class Action {
    private:
        char m_from, m_to;

        Action(int from, int to) noexcept : m_from(from), m_to(to) {};
        
    public:
        Action() noexcept : m_from(Position::bad_location) {};
        bool correct() const noexcept;
        string gen_SN() const noexcept;
        int get_from() const noexcept;
        int get_to() const noexcept;
        friend Position;
    };

    class Action_for_h {
    private:
        Card m_card;
        unsigned char m_from, m_to;

        Action_for_h(const Card& card, int from, int to) noexcept : m_card(card), m_from(from), m_to(to) {};

    public:
        Action_for_h() noexcept : m_from(Position::bad_location) {};
        bool correct() const noexcept;
        const Card& get_card() const noexcept;
        int get_from() const noexcept;
        int get_to() const noexcept;
        friend Position;
    };

private:
    static Position::Table table;
    static constexpr int bad_location = 64;
#ifdef TEST_ZKEY
    Position::Union_array_card m_union_array_card_below;
#else
    Card m_array_card_below[CARD_SIZE];
#endif
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
    Bits m_bits_deadlocked;
    unsigned char m_ncard_deadlocked;
    unsigned char m_array_ncard_not_deadlocked_below_and[CARD_SIZE];
    bool m_is_solved;
    unordered_map<uint64_t, Position::Entry_tt> m_tt;

    bool correct() const;
    bool correct_for_h() const;
    void initialize(const Card (&)[TABLEAU_COLUMN_SIZE][64], const Card (&)[HOMECELL_SIZE], const Card (&)[FREECELL_SIZE]) noexcept;
    int find_freecell_empty() const noexcept;
    int find_column_empty() const noexcept;
    Card obtain_top_in_homecell(const Card&) const noexcept;
    void update_array_card_below(const Card& card, const Card& below) noexcept {
        assert(card.is_card() && below.is_card_or_location() && (card != below));
#ifdef TEST_ZKEY
        m_zobrist_key ^= Position::table.get(card, m_union_array_card_below.get_array_card_below(card));
        m_union_array_card_below.set_array_card_below(card, below);
#else
        m_zobrist_key ^= Position::table.get(card, m_array_card_below[card.get_id()]);
        m_array_card_below[card.get_id()] = below;
#endif
        m_zobrist_key ^= Position::table.get(card, below);
    };
    Card obtain_Card(char) const noexcept;

public:
    Position(int) noexcept;
    bool correct_Action(const Position::Action&) const noexcept;
    bool correct_Action(const Position::Action_for_h&) const noexcept;
    void print() const noexcept;
    void print_for_h() const noexcept;
    void print_location() const noexcept;
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
    int dfstt1(int, Position::Action_for_h*, Position::Entry_tt&) noexcept;
    int gen_actions(Position::Action (&)[MAX_ACTION_SIZE]) const noexcept;
    int move_to_homecell_next(const Card&, Position::Action_for_h*) noexcept;
    int move_auto(Position::Action*) noexcept;
    int move_auto(Position::Action_for_h*) noexcept;
    void back_to_parent(const Position::Action_for_h*, int) noexcept;
    void make(const Action&) noexcept;
    void make(const Position::Action_for_h& action) noexcept;
    void unmake(const Action&) noexcept;
    void unmake(const Position::Action_for_h& action) noexcept;
};
  

#endif