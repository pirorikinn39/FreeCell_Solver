#include "position.hpp"

bool Position::correct() const noexcept {
  if (! Position_base::ok()) return false;
  try { 
    Card array_single_suit_cycle[MAX_SINGLE_SUIT_CYCLE_SIZE];
    Two_suit_cycle array_two_suit_cycle[MAX_TWO_SUIT_CYCLE_SIZE];
    Bits bits_single_suit_cycle;
    int nsingle_suit_cycle = 0;
    int ntwo_suit_cycle = 0;
    unsigned char count_in_two_suit_cycle[DECK_SIZE];

    bits_single_suit_cycle.clear();
    for (int pile=0; pile<TABLEAU_SIZE; ++pile) {
      for (Card card=m_array_pile_top[pile]; card.is_card();
	   card=m_row_data.get_below(card)) {
	Bits bits_same_suit_small_rank = Bits::same_suit_small_rank(card);
	for (Card below=m_row_data.get_below(card); below.is_card();
	     below=m_row_data.get_below(below)) {
	  if (! (Bits(below) & bits_same_suit_small_rank)) continue; // is set
	  array_single_suit_cycle[nsingle_suit_cycle++] = card;
	  bits_single_suit_cycle.set_bit(card);
	  break; } } }
    if (nsingle_suit_cycle != m_nsingle_suit_cycle) throw E(__LINE__);
    
    for (int i=0; i<nsingle_suit_cycle; ++i) {
      int check = 0; // delete
      for (int j=0; j<m_nsingle_suit_cycle; ++j) {
	if (array_single_suit_cycle[i] == m_array_single_suit_cycle[j]) {
	  check = 1;
	  break; } }
      if (! check) throw E(__LINE__); }
    
    ntwo_suit_cycle = 0;
    memset(count_in_two_suit_cycle, 0, sizeof(count_in_two_suit_cycle));
    for (int column1=0; column1<TABLEAU_SIZE; ++column1) { // size-1 ?
      for (Card card1=m_array_pile_top[column1]; card1.is_card(); card1=m_row_data.get_below(card1)) {
	if (Bits(card1) & bits_single_suit_cycle) continue;// is set
	int suit_card1 = card1.suit();
	Bits bits_explored_card2; // change name
	for (Card prev_card1=card1.prev(); prev_card1; prev_card1=prev_card1.prev()) {
	  if (m_bits_homecell.is_set_bit(prev_card1)) break;
	  if (m_bits_freecell.is_set_bit(prev_card1)) continue;
	  int column2 = m_array_location[prev_card1.get_id()];
	  if (column2 < column1) continue; // <= ?
	  for (Card card2=m_array_pile_top[column2]; card2!=prev_card1;
	       card2=m_row_data.get_below(card2)) {
	    if (bits_explored_card2.is_set_bit(card2)) continue;
	    if (Bits(card2) & bits_single_suit_cycle) continue; // is set
	    if (card2.suit() == suit_card1) continue;
	    bits_explored_card2.set_bit(card2);
	    Bits bits_same_suit_small_rank_card2 = Bits::same_suit_small_rank(card2);
	    for (Card below1=m_row_data.get_below(card1); below1.is_card();
		 below1=m_row_data.get_below(below1)) {
	      if (! (Bits(below1) & bits_same_suit_small_rank_card2)) // is set
		continue;
	      array_two_suit_cycle[ntwo_suit_cycle++] = Two_suit_cycle(card1, card2, true);
	      ++count_in_two_suit_cycle[card1.get_id()];
	      ++count_in_two_suit_cycle[card2.get_id()];
	      break; } } } } }
    if (ntwo_suit_cycle != (int)m_ntwo_suit_cycle) throw E(__LINE__); // remove cast
    
    for (int i=0; i<ntwo_suit_cycle; ++i) {
      int check = 0; // delete it 
      for (int j=0; j<(int)m_ntwo_suit_cycle; ++j) {
	if ((array_two_suit_cycle[i].get_card1() == m_array_two_suit_cycle[j].get_card1()) && (array_two_suit_cycle[i].get_card2() == m_array_two_suit_cycle[j].get_card2())) {
	  check = 1;
	  break; // provide method
	}
	else if ((array_two_suit_cycle[i].get_card1() == m_array_two_suit_cycle[j].get_card2()) && (array_two_suit_cycle[i].get_card1() == m_array_two_suit_cycle[j].get_card2())) {
	  check = 1;
	  break;
	}
      }
      if (! check) throw E(__LINE__); }
    
    for (int id=0; id<DECK_SIZE; ++id)
      if (count_in_two_suit_cycle[id] != m_count_in_two_suit_cycle[id]) throw E(__LINE__); }
  catch (const char *cstr) {
    cerr << cstr << endl;
    return false; }
  return true; }

Position::Position(int seed) noexcept : Position_base::Position_base(seed) {
    find_cycle();
    assert(correct()); }

void Position::find_cycle() noexcept {
    m_nsingle_suit_cycle = 0;
    m_bits_single_suit_cycle.clear();
    for (int column=0; column<TABLEAU_SIZE; ++column) {
        for (Card card=m_array_pile_top[column]; card.is_card(); card=m_row_data.get_below(card)) {
            Bits bits_same_suit_small_rank = Bits::same_suit_small_rank(card);
            for (Card below=m_row_data.get_below(card); below.is_card(); below=m_row_data.get_below(below)) {
                if (! (Bits(below) & bits_same_suit_small_rank))
                    continue;
                m_array_single_suit_cycle[m_nsingle_suit_cycle++] = card;
                m_bits_single_suit_cycle.set_bit(card);
                break;
            }
        }
    }
    assert(m_nsingle_suit_cycle <= MAX_SINGLE_SUIT_CYCLE_SIZE);

    m_ntwo_suit_cycle = 0;
    memset(m_count_in_two_suit_cycle, 0, sizeof(m_count_in_two_suit_cycle));
    for (int column1=0; column1<TABLEAU_SIZE; ++column1) {
        for (Card card1=m_array_pile_top[column1]; card1.is_card(); card1=m_row_data.get_below(card1)) {
            if (Bits(card1) & m_bits_single_suit_cycle)
                continue;
            int suit_card1 = card1.suit();
            Bits bits_explored_card2;
            for (Card prev_card1=card1.prev(); prev_card1; prev_card1=prev_card1.prev()) {
                if (m_bits_homecell.is_set_bit(prev_card1))
                    break;
                if (m_bits_freecell.is_set_bit(prev_card1))
                    continue;
                int column2 = m_array_location[prev_card1.get_id()];
                if (column2 < column1)
                    continue;
                for (Card card2=m_array_pile_top[column2]; card2!=prev_card1; card2=m_row_data.get_below(card2)) {
                    if (bits_explored_card2.is_set_bit(card2))
                        continue;
                    if (Bits(card2) & m_bits_single_suit_cycle)
                        continue;
                    if (card2.suit() == suit_card1)
                        continue;
                    bits_explored_card2.set_bit(card2);
                    Bits bits_same_suit_small_rank_card2 = Bits::same_suit_small_rank(card2);
                    for (Card below1=m_row_data.get_below(card1); below1.is_card(); below1=m_row_data.get_below(below1)) {
                        if (! (Bits(below1) & bits_same_suit_small_rank_card2))
                            continue;
                        m_array_two_suit_cycle[m_ntwo_suit_cycle++] = Two_suit_cycle(card1, card2, true);
                        ++m_count_in_two_suit_cycle[card1.get_id()];
                        ++m_count_in_two_suit_cycle[card2.get_id()];
                        break;
                    }
                }
            }
        }
    }
    assert(m_ntwo_suit_cycle <= MAX_TWO_SUIT_CYCLE_SIZE);
}

void Position::delete_cycle(const Card& card) noexcept {
    int tail = m_nsingle_suit_cycle - 1;
    for (int i=m_nsingle_suit_cycle-1; i>=0; --i) {
        if (m_array_single_suit_cycle[i] != card)
            continue;
        m_bits_single_suit_cycle.clear_bit(card);
        if (i < tail)
            m_array_single_suit_cycle[i] = m_array_single_suit_cycle[tail];
        --tail;
        --m_nsingle_suit_cycle;
        break;
    }
    assert(m_nsingle_suit_cycle >= 0);

    m_count_in_two_suit_cycle[card.get_id()] = 0;
    tail = m_ntwo_suit_cycle - 1;
    for (int i=m_ntwo_suit_cycle-1; i>=0; --i) {
        if (m_array_two_suit_cycle[i].get_card1() == card)
            --m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card2().get_id()];
        else if (m_array_two_suit_cycle[i].get_card2() == card)
            --m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card1().get_id()];
        else
            continue;
        if (i < tail)
            m_array_two_suit_cycle[i] = m_array_two_suit_cycle[tail];
        --tail;
        --m_ntwo_suit_cycle;
    }
    assert(m_ntwo_suit_cycle >= 0);
}

void Position::add_cycle(const Card& card1) noexcept {
    Bits bits_tableau = Bits::full() ^ m_bits_homecell ^ m_bits_freecell;
    if (! (Bits::same_suit_small_rank(card1) & bits_tableau))
        return;

    if (Bits::same_suit_small_rank(card1) & m_array_bits_pile_card[m_array_location[card1.get_id()]]) {
        m_array_single_suit_cycle[(int)m_nsingle_suit_cycle++] = card1;
        m_bits_single_suit_cycle.set_bit(card1);
        assert(m_nsingle_suit_cycle <= MAX_SINGLE_SUIT_CYCLE_SIZE);
        assert(correct());
        return;
    }

    int suit_card1 = card1.suit();
    Bits bits_explored_card2;
    for (Card prev_card1=card1.prev(); prev_card1; prev_card1=prev_card1.prev()) {
        if (m_bits_homecell.is_set_bit(prev_card1))
            break;
        if (m_bits_freecell.is_set_bit(prev_card1))
            continue;
        int column2 = m_array_location[prev_card1.get_id()];

        for (Card card2=m_array_pile_top[column2]; card2!=prev_card1; card2=m_row_data.get_below(card2)) {
            if (bits_explored_card2.is_set_bit(card2))
                continue;
            if (Bits(card2) & m_bits_single_suit_cycle)
                continue;
            if (card2.suit() == suit_card1)
                continue;
            bits_explored_card2.set_bit(card2);
            Bits bits_same_suit_small_rank_card2 = Bits::same_suit_small_rank(card2);
            for (Card below1=m_row_data.get_below(card1); below1.is_card(); below1=m_row_data.get_below(below1)) {
                if (! (Bits(below1) & bits_same_suit_small_rank_card2))
                    continue;
                m_array_two_suit_cycle[m_ntwo_suit_cycle++] = Two_suit_cycle(card1, card2, true);
                ++m_count_in_two_suit_cycle[card1.get_id()];
                ++m_count_in_two_suit_cycle[card2.get_id()];
                break;
            }
        }
    }
    assert(m_ntwo_suit_cycle <= MAX_TWO_SUIT_CYCLE_SIZE);
    assert(correct());
}

int Position::calc_h_cost() noexcept {
    assert(correct());
    Bits bits_decided;
    int size = 0;
    int index; 

    for (int i=0; i<(int)m_ntwo_suit_cycle; ++i) {
        if (m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card1().get_id()] == 1) {
            m_array_two_suit_cycle[i].set_is_target(false);
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card1().get_id()] -= 1;
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card2().get_id()] -= 1;
            if (! (bits_decided & (Bits(m_array_two_suit_cycle[i].get_card1()) | Bits(m_array_two_suit_cycle[i].get_card2())))) {
                ++size;
                bits_decided.set_bit(m_array_two_suit_cycle[i].get_card2());
            }
        }
        else if (m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card2().get_id()] == 1) {
            m_array_two_suit_cycle[i].set_is_target(false);
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card1().get_id()] -= 1;
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card2().get_id()] -= 1;
            if (! (bits_decided & (Bits(m_array_two_suit_cycle[i].get_card1()) | Bits(m_array_two_suit_cycle[i].get_card2())))) {
                ++size;
                bits_decided.set_bit(m_array_two_suit_cycle[i].get_card1());
            }
        }
    }

    for (int i=0; i<(int)m_ntwo_suit_cycle; ++i) {
        if (! m_array_two_suit_cycle[i].get_is_target())
            continue;
        if ((bits_decided & Bits(m_array_two_suit_cycle[i].get_card1())) || (bits_decided & Bits(m_array_two_suit_cycle[i].get_card2()))) {
            m_array_two_suit_cycle[i].set_is_target(false);
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card1().get_id()] -= 1;
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card2().get_id()] -= 1;
        }
    }

    for (index=0; index<(int)m_ntwo_suit_cycle; ++index)
        if (m_array_two_suit_cycle[index].get_is_target())
            break;
    if (index == (int)m_ntwo_suit_cycle) {
        for (int i=0; i<(int)m_ntwo_suit_cycle; ++i) {
            if (! m_array_two_suit_cycle[i].get_is_target()) {
                m_array_two_suit_cycle[i].set_is_target(true);
                m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card1().get_id()] += 1;
                m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card2().get_id()] += 1;
            }
        }
        size += ncard_rest() + m_nsingle_suit_cycle;
        assert((size >= 0) && (size <= 256));
        return size;
    }
    size += dfs(index, 0, DECK_SIZE);

    for (int i=0; i<(int)m_ntwo_suit_cycle; ++i) {
        if (! m_array_two_suit_cycle[i].get_is_target()) {
            m_array_two_suit_cycle[i].set_is_target(true);
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card1().get_id()] += 1;
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card2().get_id()] += 1;
        }
    }
    size += ncard_rest() + m_nsingle_suit_cycle;
    assert((size >= 0) && (size <= 255));
    return size;
}

int Position::dfs(int index, int size, int th) noexcept {
    if (index == (int)m_ntwo_suit_cycle)
        return min(size, th);

    assert(m_array_two_suit_cycle[index].get_is_target());
    if (size + 1 >= th)
        return th;

    const Card& card1 = m_array_two_suit_cycle[index].get_card1();
    const Card& card2 = m_array_two_suit_cycle[index].get_card2();
    int new_index;
    int array_index_deleted_cycle[DECK_SIZE];
    int nindex_deleted_cycle = 0;
    if (m_count_in_two_suit_cycle[card1.get_id()] == 1) {
        for (int i=index; i<(int)m_ntwo_suit_cycle; ++i) {
            if (! m_array_two_suit_cycle[i].get_is_target())
                continue;
            if ((m_array_two_suit_cycle[i].get_card1() != card2) && (m_array_two_suit_cycle[i].get_card2() != card2))
                continue;
            array_index_deleted_cycle[nindex_deleted_cycle++] = i;
            m_array_two_suit_cycle[i].set_is_target(false);
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card1().get_id()] -= 1;
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card2().get_id()] -= 1;
        }
        for (new_index=index+1; new_index<(int)m_ntwo_suit_cycle; ++new_index)
            if (m_array_two_suit_cycle[new_index].get_is_target())
                break;
        th = dfs(new_index, size + 1, th);
        for (int i=0; i<nindex_deleted_cycle; ++i) {
            m_array_two_suit_cycle[array_index_deleted_cycle[i]].set_is_target(true);
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card1().get_id()] += 1;
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card2().get_id()] += 1;
        }        
    }
    else if (m_count_in_two_suit_cycle[card2.get_id()] == 1) {
        for (int i=index; i<(int)m_ntwo_suit_cycle; ++i) {
            if (! m_array_two_suit_cycle[i].get_is_target())
                continue;
            if ((m_array_two_suit_cycle[i].get_card1() != card1) && (m_array_two_suit_cycle[i].get_card2() != card1))
                continue;
            array_index_deleted_cycle[nindex_deleted_cycle++] = i;
            m_array_two_suit_cycle[i].set_is_target(false);
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card1().get_id()] -= 1;
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card2().get_id()] -= 1;
        }
        for (new_index=index+1; new_index<(int)m_ntwo_suit_cycle; ++new_index)
            if (m_array_two_suit_cycle[new_index].get_is_target())
                break;
        th = dfs(new_index, size + 1, th);
        for (int i=0; i<nindex_deleted_cycle; ++i) {
            m_array_two_suit_cycle[array_index_deleted_cycle[i]].set_is_target(true);
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card1().get_id()] += 1;
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card2().get_id()] += 1;
        }
    }
    else {
        for (int i=index; i<(int)m_ntwo_suit_cycle; ++i) {
            if (! m_array_two_suit_cycle[i].get_is_target())
                continue;
            if ((m_array_two_suit_cycle[i].get_card1() != card1) && (m_array_two_suit_cycle[i].get_card2() != card1))
                continue;
            array_index_deleted_cycle[nindex_deleted_cycle++] = i;
            m_array_two_suit_cycle[i].set_is_target(false);
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card1().get_id()] -= 1;
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card2().get_id()] -= 1;
        }
        for (new_index=index+1; new_index<(int)m_ntwo_suit_cycle; ++new_index)
            if (m_array_two_suit_cycle[new_index].get_is_target())
                break;
        th = dfs(new_index, size + 1, th);
        for (int i=0; i<nindex_deleted_cycle; ++i) {
            m_array_two_suit_cycle[array_index_deleted_cycle[i]].set_is_target(true);
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card1().get_id()] += 1;
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card2().get_id()] += 1;
        }

        nindex_deleted_cycle = 0;
        for (int i=index; i<(int)m_ntwo_suit_cycle; ++i) {
            if (! m_array_two_suit_cycle[i].get_is_target())
                continue;
            if ((m_array_two_suit_cycle[i].get_card1() != card2) && (m_array_two_suit_cycle[i].get_card2() != card2))
                continue;
            array_index_deleted_cycle[nindex_deleted_cycle++] = i;
            m_array_two_suit_cycle[i].set_is_target(false);
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card1().get_id()] -= 1;
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[i].get_card2().get_id()] -= 1;
        }
        for (new_index=index+1; new_index<(int)m_ntwo_suit_cycle; ++new_index)
            if (m_array_two_suit_cycle[new_index].get_is_target())
                break;
        th = dfs(new_index, size + 1, th);
        for (int i=0; i<nindex_deleted_cycle; ++i) {
            m_array_two_suit_cycle[array_index_deleted_cycle[i]].set_is_target(true);
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card1().get_id()] += 1;
            m_count_in_two_suit_cycle[m_array_two_suit_cycle[array_index_deleted_cycle[i]].get_card2().get_id()] += 1;
        }
    }
    return th;
}

int Position::move_auto(Action* history) noexcept {
    assert(correct());
    int naction = 0;
    Bits bits_from = m_bits_freecell | m_bits_pile_top;
    Bits bits_possible = bits_from & m_bits_homecell_next;

    for (Card card=bits_possible.pop(); card; card=bits_possible.pop()) {
        if (card.rank() > 0) {
            Bits bits_placeable_in_homecell = m_bits_homecell & Bits::placeable(card);
            if (bits_placeable_in_homecell.popu() < 2) 
                continue; 
        }
        history[naction] = Action(m_array_location[card.get_id()], card.suit() + 12);
        make(history[naction++]);
        bits_from = m_bits_freecell | m_bits_pile_top;
        bits_possible = bits_from & m_bits_homecell_next;
    }
    return naction;
}

void Position::make(const Action& action) noexcept {
    assert(is_legal(action));
    Position_base::make(action);

    Card card;
    if      (action.get_to() <=  7) card = m_array_pile_top[ action.get_to() ];
    else if (action.get_to() <= 11) card = m_array_freecell[action.get_to() - TABLEAU_SIZE];
    else                            card = m_array_homecell[action.get_to() - 12];
    
    if (action.get_from() <= 7 && m_array_pile_top[ action.get_from() ].is_card())
      delete_cycle(card);
    if (action.get_to() <= 7 && m_row_data.get_below(card).is_card()) add_cycle(card);

    assert(correct()); }

void Position::unmake(const Action& action) noexcept {
    Position_base::unmake(action);
    
    Card card;
    if (action.get_from() <= 7) card = m_array_pile_top[ action.get_from() ];
    else                        card = m_array_freecell[ action.get_from() - TABLEAU_SIZE ];

    if (action.get_to() <= 7 && m_array_pile_top[ action.get_to() ].is_card())
      delete_cycle(card);
    if (action.get_from() <= 7 && m_row_data.get_below(card).is_card()) add_cycle(card);
    
    assert(correct());
    assert(is_legal(action)); }
