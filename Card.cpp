#include "Card.h"

constexpr int Card::spade;
constexpr int Card::club;
constexpr int Card::diamond;
constexpr int Card::heart;
constexpr int Card::homecell_id;
constexpr int Card::freecell_id;
constexpr int Card::field_id;
constexpr char Card::str_suits[SUIT_SIZE];
constexpr char Card::str_ranks[RANK_SIZE][3];

pair<int, int> Card::suits_xcolor(int id) noexcept {
	assert((0 <= id) && (id <= 51));
	if(id <= 25)
		return {2, 3};
	return {0, 1};
}

string Card::gen_str() const noexcept {
	assert(is_card());
	return string(&Card::str_suits[m_id / RANK_SIZE], 1) + string(Card::str_ranks[m_id % RANK_SIZE]);
}
