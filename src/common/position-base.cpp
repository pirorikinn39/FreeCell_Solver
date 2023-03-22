#include <iostream>
#include "position-base.hpp"
Position_base::Table Position_base::table;

string Action::gen_SN() const noexcept {
    constexpr char tbl[4] = {'a', 'b', 'c', 'd'};
    string str;

    assert(ok());
    if (m_from <= 7) str += to_string(m_from + 1);
    else             str += string(1, tbl[m_from - TABLEAU_SIZE]);
      
    if      (m_to <=  7) str += to_string(m_to + 1);
    else if (m_to <= 11) str += string(1, tbl[m_to - TABLEAU_SIZE]);
    else                 str += "h";
    return str; }

bool Position_base::ok() const noexcept {
  try { 
    Bits bits_pile_top, bits_homecell, bits_freecell;
    Bits array_bits_pile_card[TABLEAU_SIZE];
    unsigned char ncard[CARD_SIZE];
    Card array_homecell_above[HOMECELL_SIZE];
    Card array_freecell_above[FREECELL_SIZE];
    Card array_pile_above[TABLEAU_SIZE];
    Card array_card_above[CARD_SIZE];
    Card array_homecell[HOMECELL_SIZE];
    int ncard_homecell = 0;
    int ncard_freecell = 0;
    int npile = 0;

    fill_n(ncard, CARD_SIZE, 0);
    fill_n(array_homecell, HOMECELL_SIZE, Card());
    fill_n(array_homecell_above, HOMECELL_SIZE, Card());
    fill_n(array_freecell_above, FREECELL_SIZE, Card());
    fill_n(array_pile_above, TABLEAU_SIZE, Card());
    fill_n(array_card_above, CARD_SIZE, Card());

    uint64_t zobrist_key = 0;
    for (int id=0; id<CARD_SIZE; ++id)
      zobrist_key ^= table.get(Card(id), m_row_data.get_below(id));
    if (zobrist_key != m_zobrist_key) throw E(__LINE__);
    
    for (int above=0; above<CARD_SIZE; ++above) {
      Card below = m_row_data.get_below(above);
      if (! below) throw E(__LINE__);
      
      if (below == Card::freecell()) {
	if (ncard_freecell >= FREECELL_SIZE) throw E(__LINE__);
	array_freecell_above[ncard_freecell++] = Card(above); }
      else if (below == Card::homecell()) {
	if (ncard_homecell >= HOMECELL_SIZE) throw E(__LINE__);
	array_homecell_above[ncard_homecell++] = Card(above); }
      else if (below == Card::field()) {
	if (npile >= TABLEAU_SIZE) throw E(__LINE__);
	array_pile_above[npile++] = Card(above); }
      else {
	if (! below.is_card()) throw E(__LINE__);
	array_card_above[below.get_id()] = Card(above); } }
    
    for (int pile=0; pile<TABLEAU_SIZE; ++pile) {
      array_bits_pile_card[pile].clear();
      if (! array_pile_above[pile].is_card()) continue;
      for (Card card=array_pile_above[pile]; card.is_card();
	   card=array_card_above[card.get_id()])
	array_bits_pile_card[pile].set_bit(card); }
    
    int npile_origin = 0;
    for (int pile1=0; pile1<TABLEAU_SIZE; ++pile1) {
      if (! m_array_bits_column_card[pile1]) continue;
      npile_origin += 1;
      int pile2;
      for (pile2=0; pile2<TABLEAU_SIZE; ++pile2)
	if (array_bits_pile_card[pile2] == m_array_bits_column_card[pile1]) break;
      if (pile2 == TABLEAU_SIZE) throw E(__LINE__); }
    
    ncard_freecell = 0;
    for (int freecell=0; freecell<FREECELL_SIZE; ++freecell) {
      int count = 0;
      for (Card above=array_freecell_above[freecell]; above;
	   above=array_card_above[above.get_id()]) {
	if (! above.is_card()) throw E(__LINE__);
	ncard[above.get_id()] += 1U;
	count += 1;
	ncard_freecell += 1;
	if (bits_freecell.is_set_bit(above)) throw E(__LINE__);
	bits_freecell.set_bit(above); }
      if (count > 1) throw E(__LINE__); }
    if (bits_freecell != m_bits_freecell) throw E(__LINE__);
    if (ncard_freecell != m_ncard_freecell) throw E(__LINE__);

    bits_freecell.clear();
    for (int freecell=0; freecell<FREECELL_SIZE; ++freecell) {
      Card card = m_array_freecell[freecell];
      if (! card) continue;
      if (! card.is_card()) throw E(__LINE__);
      if (bits_freecell.is_set_bit(card.get_id())) throw E(__LINE__);
      bits_freecell.set_bit(card.get_id()); }
    if (bits_freecell != m_bits_freecell) throw E(__LINE__);

    for (int id=0; id<CARD_SIZE; ++id) {
      int from = m_array_location[id];
      if (from >= 16) throw E(__LINE__);
      if (from >= 12) {
	if (! m_bits_homecell.is_set_bit(id)) throw E(__LINE__);
	if (Card::suit(id) != from - 12) throw E(__LINE__);
	if (Card::rank(id) > m_array_homecell[from - 12].rank()) throw E(__LINE__); } // !=
      else if (from >= 8) {
	if (! m_bits_freecell.is_set_bit(id)) throw E(__LINE__);
	if (m_array_freecell[from - 8] != id) throw E(__LINE__); }
      else {
	bool is_contained = false;
	for (Card below=m_array_column_top[from]; below.is_card();
	     below=m_row_data.get_below(below))
	  if (below == id) {
	    is_contained = true;
	    break; }
	if (! is_contained) throw E(__LINE__); } }
    
    for (int homecell=0; homecell<HOMECELL_SIZE; ++homecell) {
      for (Card above=array_homecell_above[homecell]; above;
	   above=array_card_above[above.get_id()]) {
	if (! above.is_card()) throw E(__LINE__);
	ncard[above.get_id()] += 1U;
	if (bits_homecell.is_set_bit(above)) throw E(__LINE__);
	array_homecell[above.suit()] = above;
	bits_homecell.set_bit(above); } }
    if (bits_homecell != m_bits_homecell) throw E(__LINE__);
    
    for (int suit=0; suit<SUIT_SIZE; ++suit)
      if (array_homecell[suit] != m_array_homecell[suit]) throw E(__LINE__);
    
    bits_pile_top.clear();
    int ncard_tableau = 0;
    for (int pile=0; pile<npile; ++pile) {
      Card top;
      for (Card above=array_pile_above[pile]; above; above=array_card_above[above.get_id()]) {
	if (! above.is_card()) throw E(__LINE__);
	ncard[above.get_id()] += 1U;
	ncard_tableau += 1;
	top = above; }
      
      if (bits_pile_top.is_set_bit(top)) throw E(__LINE__);
      bits_pile_top.set_bit(top); }
    if (bits_pile_top != m_bits_column_top) throw E(__LINE__);
    if (ncard_tableau != m_ncard_tableau) throw E(__LINE__);

    for (int id=0; id<CARD_SIZE; ++id)
      if (ncard[id] != 1) throw E(__LINE__);

    bits_pile_top.clear();
    for (int pile=0; pile<TABLEAU_SIZE; ++pile) {
      Card pile_top = m_array_column_top[pile];
      if (! pile_top) {
	if (m_array_bits_column_next[pile]) throw E(__LINE__);
	continue; }
      if (! pile_top.is_card()) throw E(__LINE__);
      if (bits_pile_top.is_set_bit(pile_top)) throw E(__LINE__);
      if (Bits::placeable(pile_top) != m_array_bits_column_next[pile]) throw E(__LINE__);
      bits_pile_top.set_bit(pile_top); }
    if (bits_pile_top != m_bits_column_top) throw E(__LINE__);

    Bits bits_homecell_next;
    for (int suit=0; suit<SUIT_SIZE; ++suit) {
      Card card = array_homecell[suit];
      if (! card.is_card()) bits_homecell_next.set_bit(Card(suit, 0));
      else if (card.rank() < 12) bits_homecell_next.set_bit(card.next()); }
    if (bits_homecell_next != m_bits_homecell_next) throw E(__LINE__); }
  catch (const char *cstr) {
    cerr << cstr << endl;
    return false; }
  return true; }

