#include <iostream>
#include <cassert>
#include <random>
#include <fstream>
#include <cmath>
#include <utility>
#include <string>

#ifndef Card_h
#define Card_h

using namespace std;

#define SUIT_SIZE 4
#define RANK_SIZE 13
#define CARD_SIZE (SUIT_SIZE * RANK_SIZE)
#define HOMECELL_SIZE 4
#define FREECELL_SIZE 4
#define TABLEAU_COLUMN_SIZE 8
#define MAX_TABLEAU_STACK_SIZE 19
#define ID_SIZE 55

class Card {
private:
	char m_id;

	constexpr bool correct() const noexcept { return (m_id >= -1) && (m_id <= 54); };

public:
	static constexpr int spade = 0;
	static constexpr int club = 1;
	static constexpr int diamond = 2;
	static constexpr int heart = 3;
	static constexpr int homecell_id = 52;
	static constexpr int freecell_id = 53;
	static constexpr int field_id = 54;
	static constexpr char str_suits[SUIT_SIZE] = {'S', 'C', 'D', 'H'};
	static constexpr char str_ranks[RANK_SIZE][3] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};

	constexpr Card() noexcept : m_id(-1) {};
	constexpr explicit Card(int id) noexcept : m_id(id) {};
	constexpr Card(int suit, int rank) noexcept : m_id(suit * 13 + rank) {};
	static constexpr Card empty() noexcept { 
		return Card(); 
	};
	static constexpr Card homecell() noexcept { 
		return Card(Card::homecell_id); 
	};
	static constexpr Card freecell() noexcept { 
		return Card(Card::freecell_id); 
	};
	static constexpr Card field() noexcept { 
		return Card(Card::field_id); 
	};
	int get_id() const noexcept {
		assert(correct());
		return m_id;
	};
	int suit() const noexcept {
		assert(is_card());
		return suit(m_id);
	};
	static int suit(int id) noexcept {
		assert((0 <= id) && (id <= 51));
		return id / RANK_SIZE;
	};
	static pair<int, int> suits_xcolor(int) noexcept;
	int rank() const noexcept {
		assert(is_card());
		return rank(m_id);
	};
	static int rank(int id) noexcept {
		assert((0 <= id) && (id <= 51));
		return id % RANK_SIZE;
	};
	string gen_str() const noexcept;
	Card next() const noexcept {
		assert(correct());
    	return next(m_id);
	};
	static Card next(int id) noexcept {
		assert((0 <= id) && (id <= 51));
		constexpr signed char tbl[52] = { 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, -1,
	                                  	  14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1,
	                                      27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, -1,
	                                      40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1  };
    	return Card(tbl[id]);
	};
	Card prev() const noexcept {
		assert(correct());
    	return prev(m_id);
	};
	static Card prev(int id) noexcept {
		assert((0 <= id) && (id <= 51));
		constexpr signed char tbl[52] = { -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
	                                      -1, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
	                                      -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
	                                      -1, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50  };
    	return Card(tbl[id]);
	};
	constexpr bool is_card() const noexcept { 
		return (m_id >= 0) && (m_id <= 51); 
	};
	constexpr bool is_location() const noexcept { 
		return (m_id >= 52) && (m_id <= 54); 
	};                       
	constexpr bool is_card_or_location() const noexcept { 
		return (m_id >= 0) && (m_id <= 54); 
	};
	constexpr bool is_homecell() const noexcept { 
		return m_id == homecell_id; 
	};                       
	constexpr bool is_freecell() const noexcept { 
		return m_id == freecell_id; 
	};                       
	constexpr bool is_field() const noexcept { 
		return m_id == field_id; 
	};                       
	constexpr operator bool() const noexcept { 
		return is_card_or_location(); 
	};          
	constexpr bool operator!() const noexcept { 
		return !is_card_or_location(); 
	};
};

inline bool operator==(const Card &card1, const Card &card2) noexcept { 
	return card1.get_id() == card2.get_id(); 
};
inline bool operator!=(const Card &card1, const Card &card2) noexcept { 
	return ! (card1 == card2); 
};
inline bool operator==(const Card &card, int id) noexcept { 
	return card.get_id() == id; 
};
inline bool operator!=(const Card &card, int id) noexcept { 
	return ! (card == id); 
};
inline bool operator==(int id, const Card &card) noexcept { 
	return id == card.get_id(); 
};
inline bool operator!=(int id, const Card &card) noexcept { 
	return ! (id == card); 
};

#endif