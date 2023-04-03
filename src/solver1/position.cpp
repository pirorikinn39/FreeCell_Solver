#include "position.hpp"

bool Position::ok() const noexcept {
  if (! Position_base::ok()) return false;
  try { 
    Card array_single_suit_cycle[MAX_SINGLE_SUIT_CYCLE_SIZE];
    Two_suit_cycle array_two_suit_cycle[MAX_TWO_SUIT_CYCLE_SIZE];
    Bits bits_single_suit_cycle;
    int nsingle_suit_cycle = 0;
    int ntwo_suit_cycle = 0;
    unsigned char count_in_two_suit_cycle[DECK_SIZE] = {0};

    for (int pile=0; pile<TABLEAU_SIZE; ++pile) {
      for (Card card=m_array_pile_top[pile]; card.is_card();
	   card=m_row_data.get_below(card)) {
	for (Card below=m_row_data.get_below(card); below.is_card();
	     below=m_row_data.get_below(below)) {
	  if (! Bits::same_suit_small_rank(card).is_set_bit(below)) continue;
	  array_single_suit_cycle[nsingle_suit_cycle++] = card;
	  bits_single_suit_cycle.set_bit(card);
	  break; } } }
    if (nsingle_suit_cycle != m_nsingle_suit_cycle) throw E(__LINE__);
    
    for (int i=0; i<nsingle_suit_cycle; ++i) {
      int j;
      for (j=0; j<m_nsingle_suit_cycle; ++j) {
	if (array_single_suit_cycle[i] == m_array_single_suit_cycle[j]) break; }
      if (j == m_nsingle_suit_cycle) throw E(__LINE__); }
    
    ntwo_suit_cycle = 0;
    for (int pile1=0; pile1<TABLEAU_SIZE-1; ++pile1) {
      for (Card card1=m_array_pile_top[pile1]; card1.is_card();
	   card1=m_row_data.get_below(card1)) {
	if (bits_single_suit_cycle.is_set_bit(card1)) continue;
	Bits bits_na_to_card2 = bits_single_suit_cycle | Bits::same_suit(card1.suit());
	for (Card prev_card1=card1.prev(); prev_card1; prev_card1=prev_card1.prev()) {
	  if (m_bits_homecell.is_set_bit(prev_card1)) break;
	  if (m_bits_freecell.is_set_bit(prev_card1)) continue;
	  int pile2 = m_array_location[prev_card1.get_id()];
	  if (pile2 <= pile1) continue;
	  for (Card card2=m_array_pile_top[pile2]; card2!=prev_card1;
	       card2=m_row_data.get_below(card2)) {
	    if (bits_na_to_card2.is_set_bit(card2)) continue;
	    bits_na_to_card2.set_bit(card2);
	    Bits bits_same_suit_small_rank_card2 = Bits::same_suit_small_rank(card2);
	    for (Card below1=m_row_data.get_below(card1); below1.is_card();
		 below1=m_row_data.get_below(below1)) {
	      if (! bits_same_suit_small_rank_card2.is_set_bit(below1))	continue;
	      array_two_suit_cycle[ntwo_suit_cycle++] = Two_suit_cycle(card1, card2, true);
	      count_in_two_suit_cycle[card1.get_id()] += 1;
	      count_in_two_suit_cycle[card2.get_id()] += 1;
	      break; } } } } }
    if (ntwo_suit_cycle != m_ntwo_suit_cycle) throw E(__LINE__);
    
    for (int i=0; i<ntwo_suit_cycle; ++i) {
      int j;
      for (j=0; j<m_ntwo_suit_cycle; ++j)
	if (array_two_suit_cycle[i] == m_array_two_suit_cycle[j]) break;
      if (j == m_ntwo_suit_cycle) throw E(__LINE__); }
    
    for (int id=0; id<DECK_SIZE; ++id)
      if (count_in_two_suit_cycle[id] != m_count_in_two_suit_cycle[id]) throw E(__LINE__); }
  
  catch (const char *cstr) {
    cerr << cstr << endl;
    return false; }
  return true; }

Position::Position(int seed) noexcept : Position_base::Position_base(seed),
					m_nsingle_suit_cycle(0), m_ntwo_suit_cycle(0) {
  for (int pile=0; pile<TABLEAU_SIZE; ++pile)
    for (Card card=m_array_pile_top[pile]; card.is_card(); card=m_row_data.get_below(card))
      for (Card below=m_row_data.get_below(card); below.is_card();
	   below=m_row_data.get_below(below)) {
	if (! Bits::same_suit_small_rank(card).is_set_bit(below)) continue;
	m_array_single_suit_cycle[m_nsingle_suit_cycle++] = card;
	m_bits_single_suit_cycle.set_bit(card);
	break; }
  assert(m_nsingle_suit_cycle <= MAX_SINGLE_SUIT_CYCLE_SIZE);

  fill_n(m_count_in_two_suit_cycle, DECK_SIZE, 0);
  for (int pile1=0; pile1<TABLEAU_SIZE-1; ++pile1)
    for (Card card1=m_array_pile_top[pile1]; card1.is_card();
	 card1=m_row_data.get_below(card1)) {
      if (m_bits_single_suit_cycle.is_set_bit(card1)) continue;
      Bits bits_na_to_card2 = m_bits_single_suit_cycle | Bits::same_suit(card1.suit());
      for (Card prev_card1=card1.prev(); prev_card1; prev_card1=prev_card1.prev()) {
	if (m_bits_homecell.is_set_bit(prev_card1)) break;
	if (m_bits_freecell.is_set_bit(prev_card1)) continue;
	int pile2 = m_array_location[prev_card1.get_id()];
	if (pile2 <= pile1) continue;
	for (Card card2=m_array_pile_top[pile2]; card2!=prev_card1;
	     card2=m_row_data.get_below(card2)) {
	  if (bits_na_to_card2.is_set_bit(card2)) continue;
	  bits_na_to_card2.set_bit(card2);
	  Bits bits_same_suit_small_rank_card2 = Bits::same_suit_small_rank(card2);
	  for (Card below1=m_row_data.get_below(card1); below1.is_card();
	       below1=m_row_data.get_below(below1)) {
	    if (! bits_same_suit_small_rank_card2.is_set_bit(below1)) continue;
	    m_array_two_suit_cycle[m_ntwo_suit_cycle++] = Two_suit_cycle(card1, card2, true);
	    m_count_in_two_suit_cycle[card1.get_id()] += 1;
	    m_count_in_two_suit_cycle[card2.get_id()] += 1;
	    break; } } } }
  assert(m_ntwo_suit_cycle <= MAX_TWO_SUIT_CYCLE_SIZE);
  assert(ok()); }

void Position::delete_cycle(const Card& card) noexcept {
  assert(card.is_card());
  int tail = m_nsingle_suit_cycle - 1;
  for (int i=m_nsingle_suit_cycle-1; i>=0; --i) {
    if (m_array_single_suit_cycle[i] != card) continue;
    m_bits_single_suit_cycle.clear_bit(card);
    m_array_single_suit_cycle[i] = m_array_single_suit_cycle[tail--];
    m_nsingle_suit_cycle -= 1;
    break; }
  assert(m_nsingle_suit_cycle >= 0);
  
  m_count_in_two_suit_cycle[card.get_id()] = 0;
  tail = m_ntwo_suit_cycle - 1;
  for (int i=m_ntwo_suit_cycle-1; i>=0; --i) {
    const Card& card1 = m_array_two_suit_cycle[i].get_card1();
    const Card& card2 = m_array_two_suit_cycle[i].get_card2();
    if      (card1 == card) m_count_in_two_suit_cycle[card2.get_id()] -= 1;
    else if (card2 == card) m_count_in_two_suit_cycle[card1.get_id()] -= 1;
    else continue;
    m_array_two_suit_cycle[i] = m_array_two_suit_cycle[tail--];
    m_ntwo_suit_cycle -= 1; }
  assert(m_ntwo_suit_cycle >= 0); }

void Position::add_cycle(const Card& card1) noexcept {
  assert(card1.is_card());

  int pile1 = m_array_location[card1.get_id()];
  assert(pile1 < TABLEAU_SIZE);
  assert(m_array_pile_top[pile1] == card1);
  
  Bits bits_tableau = Bits::full() ^ m_bits_homecell ^ m_bits_freecell;
  if (! (Bits::same_suit_small_rank(card1) & bits_tableau))  return;

  if (Bits::same_suit_small_rank(card1) & m_array_bits_pile_card[pile1]) {
    m_array_single_suit_cycle[m_nsingle_suit_cycle++] = card1;
    m_bits_single_suit_cycle.set_bit(card1);
    assert(m_nsingle_suit_cycle <= MAX_SINGLE_SUIT_CYCLE_SIZE);
    return; }

  Bits bits_na_to_card2 = m_bits_single_suit_cycle | Bits::same_suit(card1.suit());
  for (Card prev_card1=card1.prev(); prev_card1; prev_card1=prev_card1.prev()) {
    if (m_bits_homecell.is_set_bit(prev_card1)) break;
    if (m_bits_freecell.is_set_bit(prev_card1)) continue;
    int pile2 = m_array_location[prev_card1.get_id()];
    
    for (Card card2=m_array_pile_top[pile2]; card2!=prev_card1;
	 card2=m_row_data.get_below(card2)) {
      if (bits_na_to_card2.is_set_bit(card2)) continue;
      bits_na_to_card2.set_bit(card2);
      if (! (Bits::same_suit_small_rank(card2) & m_array_bits_pile_card[pile1])) continue;
      m_array_two_suit_cycle[m_ntwo_suit_cycle++] = Two_suit_cycle(card1, card2, true);
      m_count_in_two_suit_cycle[card1.get_id()] += 1;
      m_count_in_two_suit_cycle[card2.get_id()] += 1; } }
  
  assert(m_ntwo_suit_cycle <= MAX_TWO_SUIT_CYCLE_SIZE); }

int Position::calc_h_cost() noexcept {
  Bits bits_decided;
  int h2_cost = 0;
  int index; 

  for (int i=0; i<m_ntwo_suit_cycle; ++i) {
    const Card& card1 = m_array_two_suit_cycle[i].get_card1();
    const Card& card2 = m_array_two_suit_cycle[i].get_card2();
    if (m_count_in_two_suit_cycle[card1.get_id()] == 1) {
      m_array_two_suit_cycle[i].clear_target();
      m_count_in_two_suit_cycle[card1.get_id()] -= 1;
      m_count_in_two_suit_cycle[card2.get_id()] -= 1;
      if (! (bits_decided & (Bits(card1) | Bits(card2)))) {
	h2_cost += 1;
	bits_decided.set_bit(card2); } }
    else if (m_count_in_two_suit_cycle[card2.get_id()] == 1) {
      m_array_two_suit_cycle[i].clear_target();
      m_count_in_two_suit_cycle[card1.get_id()] -= 1;
      m_count_in_two_suit_cycle[card2.get_id()] -= 1;
      if (! (bits_decided & (Bits(card1) | Bits(card2)))) {
	h2_cost += 1;
	bits_decided.set_bit(card1); } } }

  for (int i=0; i<m_ntwo_suit_cycle; ++i) {
    if (! m_array_two_suit_cycle[i].get_is_target()) continue;
    
    const Card& card1 = m_array_two_suit_cycle[i].get_card1();
    const Card& card2 = m_array_two_suit_cycle[i].get_card2();
    if ((bits_decided & Bits(card1)) || (bits_decided & Bits(card2))) {
      m_array_two_suit_cycle[i].clear_target();
      m_count_in_two_suit_cycle[card1.get_id()] -= 1;
      m_count_in_two_suit_cycle[card2.get_id()] -= 1; } }
  
  for (index=0; index<m_ntwo_suit_cycle; ++index)
    if (m_array_two_suit_cycle[index].get_is_target()) break;
  
  if (index < m_ntwo_suit_cycle) h2_cost += dfs(index, 0, DECK_SIZE);
  
  for (int i=0; i<m_ntwo_suit_cycle; ++i) {
    const Card& card1 = m_array_two_suit_cycle[i].get_card1();
    const Card& card2 = m_array_two_suit_cycle[i].get_card2();
    if (! m_array_two_suit_cycle[i].get_is_target()) {
      m_array_two_suit_cycle[i].set_target();
      m_count_in_two_suit_cycle[card1.get_id()] += 1;
      m_count_in_two_suit_cycle[card2.get_id()] += 1; } }

  h2_cost += ncard_rest() + m_nsingle_suit_cycle;
  assert(0 <= h2_cost && h2_cost <= 256);
  return h2_cost; }

int Position::dfs(int current, int size, int th) noexcept {
  if (current == m_ntwo_suit_cycle) return min(size, th);

  assert(m_array_two_suit_cycle[current].get_is_target());
  if (size + 1 >= th) return th;

  const Card& card1 = m_array_two_suit_cycle[current].get_card1();
  const Card& card2 = m_array_two_suit_cycle[current].get_card2();
  int new_index;
  int array_index_deleted_cycle[DECK_SIZE];
  int nindex_deleted_cycle = 0;
  if (m_count_in_two_suit_cycle[card1.get_id()] == 1) {
    for (int i=current; i<m_ntwo_suit_cycle; ++i) {
      if (! m_array_two_suit_cycle[i].get_is_target()) continue;
      if (m_array_two_suit_cycle[i].get_card1() != card2
	  && m_array_two_suit_cycle[i].get_card2() != card2) continue;
      array_index_deleted_cycle[nindex_deleted_cycle++] = i;
      m_array_two_suit_cycle[i].clear_target();
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card1().get_id()] -= 1;
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card2().get_id()] -= 1; }
    for (new_index=current+1; new_index<(int)m_ntwo_suit_cycle; ++new_index)
      if (m_array_two_suit_cycle[new_index].get_is_target()) break;
    th = dfs(new_index, size + 1, th);
    for (int i=0; i<nindex_deleted_cycle; ++i) {
      m_array_two_suit_cycle[array_index_deleted_cycle[i]].set_target();
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card1().get_id()] += 1;
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card2().get_id()] += 1;
    }        
  }
  else if (m_count_in_two_suit_cycle[card2.get_id()] == 1) {
    for (int i=current; i<(int)m_ntwo_suit_cycle; ++i) {
      if (! m_array_two_suit_cycle[i].get_is_target())
	continue;
      if ((m_array_two_suit_cycle[i].get_card1() != card1) && (m_array_two_suit_cycle[i].get_card2() != card1))
	continue;
      array_index_deleted_cycle[nindex_deleted_cycle++] = i;
      m_array_two_suit_cycle[i].clear_target();
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card1().get_id()] -= 1;
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card2().get_id()] -= 1;
    }
    for (new_index=current+1; new_index<(int)m_ntwo_suit_cycle; ++new_index)
      if (m_array_two_suit_cycle[new_index].get_is_target())
	break;
    th = dfs(new_index, size + 1, th);
    for (int i=0; i<nindex_deleted_cycle; ++i) {
      m_array_two_suit_cycle[array_index_deleted_cycle[i]].set_target();
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card1().get_id()] += 1;
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card2().get_id()] += 1;
    }
  }
  else {
    for (int i=current; i<(int)m_ntwo_suit_cycle; ++i) {
      if (! m_array_two_suit_cycle[i].get_is_target())
	continue;
      if ((m_array_two_suit_cycle[i].get_card1() != card1) && (m_array_two_suit_cycle[i].get_card2() != card1))
	continue;
      array_index_deleted_cycle[nindex_deleted_cycle++] = i;
      m_array_two_suit_cycle[i].clear_target();
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card1().get_id()] -= 1;
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card2().get_id()] -= 1;
    }
    for (new_index=current+1; new_index<(int)m_ntwo_suit_cycle; ++new_index)
      if (m_array_two_suit_cycle[new_index].get_is_target())
	break;
    th = dfs(new_index, size + 1, th);
    for (int i=0; i<nindex_deleted_cycle; ++i) {
      m_array_two_suit_cycle[array_index_deleted_cycle[i]].set_target();
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card1().get_id()] += 1;
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card2().get_id()] += 1;
    }
    
    nindex_deleted_cycle = 0;
    for (int i=current; i<(int)m_ntwo_suit_cycle; ++i) {
      if (! m_array_two_suit_cycle[i].get_is_target())
	continue;
      if ((m_array_two_suit_cycle[i].get_card1() != card2) && (m_array_two_suit_cycle[i].get_card2() != card2))
	continue;
      array_index_deleted_cycle[nindex_deleted_cycle++] = i;
      m_array_two_suit_cycle[i].clear_target();
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card1().get_id()] -= 1;
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card2().get_id()] -= 1;
    }
    for (new_index=current+1; new_index<(int)m_ntwo_suit_cycle; ++new_index)
      if (m_array_two_suit_cycle[new_index].get_is_target())
	break;
    th = dfs(new_index, size + 1, th);
    for (int i=0; i<nindex_deleted_cycle; ++i) {
      m_array_two_suit_cycle[array_index_deleted_cycle[i]].set_target();
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card1().get_id()] += 1;
      m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card2().get_id()] += 1;
    }
  }
  return th; }

int Position::move_auto(Action* history) noexcept {
  int naction = 0;
  Bits bits_from = m_bits_freecell | m_bits_pile_top;
  Bits bits_possible = bits_from & m_bits_homecell_next;

  for (Card card=bits_possible.pop(); card; card=bits_possible.pop()) {
    if (card.rank() > 0) {
      Bits bits_placeable_in_homecell = m_bits_homecell & Bits::placeable(card);
      if (bits_placeable_in_homecell.popu() < 2) continue; }
    history[naction] = Action(card, m_array_location[card.get_id()], card.suit() + 12);
    make(history[naction++]);
    bits_from = m_bits_freecell | m_bits_pile_top;
    bits_possible = bits_from & m_bits_homecell_next; }
  return naction; }

void Position::make(const Action& action) noexcept {
    assert(is_legal(action));
    Position_base::make(action);

    Card card = action.get_card();
    if (action.get_from() <= 7 && m_array_pile_top[ action.get_from() ].is_card())
      delete_cycle(card);
    if (action.get_to() <= 7 && m_row_data.get_below(card).is_card()) add_cycle(card);
    assert(ok()); }

void Position::unmake(const Action& action) noexcept {
    Position_base::unmake(action);
    
    Card card = action.get_card();
    if (action.get_to() <= 7 && m_array_pile_top[ action.get_to() ].is_card())
      delete_cycle(card);
    if (action.get_from() <= 7 && m_row_data.get_below(card).is_card()) add_cycle(card);
    assert(ok());
    assert(is_legal(action)); }
