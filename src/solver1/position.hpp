#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <fstream>
#include <cmath>
#include <utility>
#include <string>
#include <string.h>
#include <algorithm>
#include "bits.hpp"

#ifndef Position_h
#define Position_h

#define MAX_ACTION_SIZE 34
#define MAX_SINGLE_SUIT_CYCLE_SIZE CARD_SIZE
#define MAX_TWO_SUIT_CYCLE_SIZE 1008

#define STR(x) #x
#define E(l) "internal error (line " STR(l) " in " __FILE__ ")"

class Position {
public:
#ifdef TEST_ZKEY
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
            for (int i=0; i<7; ++i)
                if (m_array_card_belows[i] != union_array_card.m_array_card_belows[i])
                    return false;
            return true;
        };
        bool identify(const Position& position) const noexcept {  
            return identify(position.m_union_array_card_below);
        };
    };
#endif

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


    class Action {
    private:
        char m_from, m_to;

        Action(int from, int to) noexcept : m_from(from), m_to(to) {};

    public:
        Action() noexcept : m_from(64) {};
        bool correct() const noexcept {
            if (m_from == m_to) 
                return false;
            if ((m_to >= 12) && (m_to <= 15)) 
                return (m_from >= 0) && (m_from <= 11);
            if ((m_to >= 8) && (m_to <= 11)) 
                return (m_from >= 0) && (m_from <= 7);
            if ((m_to >= 0) && (m_to <= 7)) 
                return (m_from >= 0) && (m_from <= 11);
            return false; 
        };
        string gen_SN() const noexcept;
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
    struct Table {
        uint64_t z_factor[CARD_SIZE][ID_SIZE];
        Table() noexcept {
            mt19937_64 mt(0);
            for (int i=0; i<CARD_SIZE; ++i)
                for (int j=0; j<ID_SIZE; ++j)
                    z_factor[i][j] = mt(); 
        };
        uint64_t get(const Card& card, const Card& below) const noexcept {
            assert(card && below);
            return z_factor[card.get_id()][below.get_id()]; 
        };
    };

    static Table table;
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
#ifdef TEST_ZKEY
        m_zobrist_key ^= Position::table.get(card, m_union_array_card_below.get_array_card_below(card));
        m_union_array_card_below.set_array_card_below(card, below);
#else
        m_zobrist_key ^= Position::table.get(card, m_array_card_below[card.get_id()]);
        m_array_card_below[card.get_id()] = below;
#endif
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
