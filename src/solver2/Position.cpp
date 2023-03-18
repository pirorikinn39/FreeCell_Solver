#include "Position.hpp"
Position::Table Position::table;
constexpr int Position::bad_location;

string Position::Action::gen_SN() const noexcept {
    constexpr char tbl[4] = {'a', 'b', 'c', 'd'};
    string str;

    assert(correct());
    if (m_from <= 7) 
        str += to_string(m_from + 1);
    else             
        str += string(1, tbl[m_from - TABLEAU_COLUMN_SIZE]);
      
    if (m_to <= 7)  
        str += to_string(m_to + 1);
    else if (m_to <= 11) 
        str += string(1, tbl[m_to - TABLEAU_COLUMN_SIZE]);
    else                 
        str += "h";
    return str; 
}

bool Position::correct() const {
	try { 
        Bits bits_column_top, bits_homecell, bits_freecell;
        Bits array_bits_column_card[TABLEAU_COLUMN_SIZE];
		unsigned char ncard1[CARD_SIZE], ncard2[CARD_SIZE];
    	Card array_homecell_above[HOMECELL_SIZE];
    	Card array_freecell_above[FREECELL_SIZE];
    	Card array_field_above[TABLEAU_COLUMN_SIZE];
    	Card array_card_above[CARD_SIZE];
    	Card array_homecell[HOMECELL_SIZE];
    	int ncard_homecell = 0;
    	int ncard_freecell = 0;
    	int ncolumn = 0;
    	uint64_t zobrist_key = 0;
        Card array_single_suit_cycle[MAX_SINGLE_SUIT_CYCLE_SIZE];
        Two_suit_cycle array_two_suit_cycle[MAX_TWO_SUIT_CYCLE_SIZE];
        Bits bits_single_suit_cycle;
        int nsingle_suit_cycle = 0;
        int ntwo_suit_cycle = 0;
        unsigned char count_in_two_suit_cycle[CARD_SIZE];

    	fill_n(ncard1, CARD_SIZE, 0);
    	fill_n(ncard2, CARD_SIZE, 0);
    	fill_n(array_homecell, HOMECELL_SIZE, Card());
    	fill_n(array_homecell_above, HOMECELL_SIZE, Card());
    	fill_n(array_freecell_above, FREECELL_SIZE, Card());
    	fill_n(array_field_above, TABLEAU_COLUMN_SIZE, Card());
    	fill_n(array_card_above, CARD_SIZE, Card());

#ifdef TEST_ZKEY
        for (int id=0; id<CARD_SIZE; ++id)
            zobrist_key ^= Position::table.get(Card(id), m_union_array_card_below.get_array_card_below(id));
#else
    	for (int id=0; id<CARD_SIZE; ++id)
    		zobrist_key ^= Position::table.get(Card(id), m_array_card_below[id]);
#endif
    	if (zobrist_key != m_zobrist_key) 
    		throw E(__LINE__);
    
    	for (int above=0; above<CARD_SIZE; ++above) {
#ifdef TEST_ZKEY
            Card below = m_union_array_card_below.get_array_card_below(above);
#else
    		Card below = m_array_card_below[above];
#endif            
    		if (! below) 
    			continue;

    		ncard1[above] += 1U;
    		if (below == Card::freecell()) {
    			if (ncard_freecell >= FREECELL_SIZE) 
    				throw E(__LINE__);
    			array_freecell_above[ncard_freecell++] = Card(above); 
    		}
    		else if (below == Card::homecell()) {
    			if (ncard_homecell >= HOMECELL_SIZE) 
    				throw E(__LINE__);
    			array_homecell_above[ncard_homecell++] = Card(above); 
    		}
    		else if (below == Card::field()) {
    			if (ncolumn >= TABLEAU_COLUMN_SIZE) 
    				throw E(__LINE__);
    			array_field_above[ncolumn++] = Card(above); 
    		}
    		else {
    			if (! below.is_card()) 
    				throw E(__LINE__);
    			array_card_above[below.get_id()] = Card(above); 
    		} 
    	}

        for (int column=0; column<TABLEAU_COLUMN_SIZE; ++column) {
            array_bits_column_card[column].clear();
            if (! array_field_above[column].is_card())
                continue;
            for (Card card=array_field_above[column]; card.is_card(); card=array_card_above[card.get_id()])
                array_bits_column_card[column].set_bit(card);
        }
        int ncolumn_origin = 0;
        for (int column1=0; column1<TABLEAU_COLUMN_SIZE; ++column1) {
            if (! m_array_bits_column_card[column1])
                continue;
            ncolumn_origin += 1;
            bool is_same = false;
            for (int column2=0; column2<TABLEAU_COLUMN_SIZE; ++column2)
                if (array_bits_column_card[column2] == m_array_bits_column_card[column1])
                    is_same = true;
            if (! is_same)
                throw E(__LINE__);
        }
        if (ncolumn != ncolumn_origin)
            throw E(__LINE__);

        ncard_freecell = 0;
        for (int freecell=0; freecell<FREECELL_SIZE; ++freecell) {
            int count = 0;
            for (Card above=array_freecell_above[freecell]; above; above=array_card_above[above.get_id()]) {
                if (! above.is_card()) 
                    throw E(__LINE__);
                ncard2[above.get_id()] += 1U;
                count += 1;
                ncard_freecell += 1;
                if (bits_freecell.is_set_bit(above)) 
                    throw E(__LINE__);
                bits_freecell.set_bit(above); 
            }
            if (count > 1) 
                throw E(__LINE__); 
        }
        if (bits_freecell != m_bits_freecell) 
            throw E(__LINE__);
        if (ncard_freecell != m_ncard_freecell) 
            throw E(__LINE__);

        bits_freecell.clear();
        for (int freecell=0; freecell<FREECELL_SIZE; ++freecell) {
            Card card = m_array_freecell[freecell];
            if (! card) 
                continue;
            if (! card.is_card())                    
                throw E(__LINE__);
            if (bits_freecell.is_set_bit(card.get_id())) 
                throw E(__LINE__);
            bits_freecell.set_bit(card.get_id()); 
        }
        if (bits_freecell != m_bits_freecell) 
            throw E(__LINE__);

        Bits bits_all_cards;
        for (int id=0; id<CARD_SIZE; ++id) {
            int from = m_array_location[id];
            if (from >= 16) 
                throw E(__LINE__);
            else if (from >= 12) {
                if (! m_bits_homecell.is_set_bit(id))
                    throw E(__LINE__);
                if (Card::suit(id) != from - 12)
                    throw E(__LINE__);
                if (Card::rank(id) > m_array_homecell[from - 12].rank())
                    throw E(__LINE__);
            }
            else if (from >= 8) {
                if (! m_bits_freecell.is_set_bit(id))
                    throw E(__LINE__);
                if (m_array_freecell[from - 8] != id) 
                    throw E(__LINE__);
            }
            else {
                if (from < 0)
                    throw E(__LINE__);
                bool is_contained = false;
#ifdef TEST_ZKEY
                for (Card below=m_array_column_top[from]; below.is_card(); below=m_union_array_card_below.get_array_card_below(below))
                    if (below == id)
                        is_contained = true;
#else                
                for (Card below=m_array_column_top[from]; below.is_card(); below=m_array_card_below[below.get_id()])
                    if (below == id)
                        is_contained = true;
#endif                    
                if (! is_contained)
                    throw E(__LINE__);
            }
            bits_all_cards.set_bit(id); 
        }
        if (bits_all_cards != Bits::full()) 
            throw E(__LINE__);

        for (int homecell=0; homecell<HOMECELL_SIZE; ++homecell) {
            for (Card above=array_homecell_above[homecell]; above; above=array_card_above[above.get_id()]) {
                if (! above.is_card()) 
                    throw E(__LINE__);
                ncard2[above.get_id()] += 1U;
                if (bits_homecell.is_set_bit(above)) 
                    throw E(__LINE__);
                array_homecell[above.suit()] = above;
                bits_homecell.set_bit(above); 
            }
        }
        if (bits_homecell != m_bits_homecell) 
            throw E(__LINE__);
        for (int suit=0; suit<SUIT_SIZE; ++suit)
            if (array_homecell[suit] != m_array_homecell[suit]) 
                throw E(__LINE__);

        bits_column_top.clear();
        int ncard_tableau = 0;
        for (int column=0; column<ncolumn; ++column) {
            Card top;
            for (Card above=array_field_above[column]; above; above=array_card_above[above.get_id()]) {
                if (! above.is_card()) 
                    throw E(__LINE__);
                ncard2[above.get_id()] += 1U;
                ncard_tableau += 1;
                top = above; 
            }
            if (bits_column_top.is_set_bit(top)) 
                throw E(__LINE__);
            bits_column_top.set_bit(top); 
        }
        if (bits_column_top != m_bits_column_top) 
            throw E(__LINE__);
        if (ncard_tableau != m_ncard_tableau) 
            throw E(__LINE__);

        for (int id=0; id<CARD_SIZE; ++id) {
            if (ncard1[id] != ncard2[id])	
                throw E(__LINE__);                    
            if (ncard1[id] > 1) 
                throw E(__LINE__); 
        }

        bits_column_top.clear();
        for (int column=0; column<TABLEAU_COLUMN_SIZE; ++column) {
            Card column_top = m_array_column_top[column];
            if (! column_top) {
                if (m_array_bits_column_next[column]) 
                    throw E(__LINE__);
                continue; 
            }
            if (! column_top.is_card()) 
                throw E(__LINE__);
            if (bits_column_top.is_set_bit(column_top)) 
                throw E(__LINE__);
            if (Bits::placeable(column_top) != m_array_bits_column_next[column])
                throw E(__LINE__);
            bits_column_top.set_bit(column_top); 
        }
        if (bits_column_top != m_bits_column_top) 
            throw E(__LINE__);

        Bits bits_homecell_next;
        for (int suit=0; suit<SUIT_SIZE; ++suit) {
            Card card = array_homecell[suit];
            if (! card.is_card()) 
                bits_homecell_next.set_bit(Card(suit, 0));
            else if (card.rank() < 12) 
                bits_homecell_next.set_bit(card.next()); 
        }
        if (bits_homecell_next != m_bits_homecell_next) 
            throw E(__LINE__);

        nsingle_suit_cycle = 0;
        bits_single_suit_cycle.clear();
        for (int column=0; column<TABLEAU_COLUMN_SIZE; ++column) {
#ifdef TEST_ZKEY
            for (Card card=m_array_column_top[column]; card.is_card(); card=m_union_array_card_below.get_array_card_below(card)) {
                Bits bits_same_suit_small_rank = Bits::same_suit_small_rank(card);
                for (Card below=m_union_array_card_below.get_array_card_below(card); below.is_card(); below=m_union_array_card_below.get_array_card_below(below)) {
                    if (! (Bits(below) & bits_same_suit_small_rank))
                        continue;
                    array_single_suit_cycle[nsingle_suit_cycle++] = card;
                    bits_single_suit_cycle.set_bit(card);
                    break;
                }
            }
#else
            for (Card card=m_array_column_top[column]; card.is_card(); card=m_array_card_below[card.get_id()]) {
                Bits bits_same_suit_small_rank = Bits::same_suit_small_rank(card);
                for (Card below=m_array_card_below[card.get_id()]; below.is_card(); below=m_array_card_below[below.get_id()]) {
                    if (! (Bits(below) & bits_same_suit_small_rank))
                        continue;
                    array_single_suit_cycle[nsingle_suit_cycle++] = card;
                    bits_single_suit_cycle.set_bit(card);
                    break;
                }
            }
#endif
        }
        if (nsingle_suit_cycle != m_nsingle_suit_cycle)
            throw E(__LINE__);
        for (int i=0; i<nsingle_suit_cycle; ++i) {
            int check = 0;
            for (int j=0; j<m_nsingle_suit_cycle; ++j) {
                if (array_single_suit_cycle[i] == m_array_single_suit_cycle[j]) {
                    check = 1;
                    break;
                }
            }
            if (! check)
                throw E(__LINE__);
        }

        ntwo_suit_cycle = 0;
        memset(count_in_two_suit_cycle, 0, sizeof(count_in_two_suit_cycle));
        for (int column1=0; column1<TABLEAU_COLUMN_SIZE; ++column1) {
#ifdef TEST_ZKEY            
            for (Card card1=m_array_column_top[column1]; card1.is_card(); card1=m_union_array_card_below.get_array_card_below(card1)) {
                if (Bits(card1) & bits_single_suit_cycle)
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
                    for (Card card2=m_array_column_top[column2]; card2!=prev_card1; card2=m_union_array_card_below.get_array_card_below(card2)) {
                        if (bits_explored_card2.is_set_bit(card2))
                            continue;
                        if (Bits(card2) & bits_single_suit_cycle)
                            continue;
                        if (card2.suit() == suit_card1)
                            continue;
                        bits_explored_card2.set_bit(card2);
                        Bits bits_same_suit_small_rank_card2 = Bits::same_suit_small_rank(card2);
                        for (Card below1=m_union_array_card_below.get_array_card_below(card1); below1.is_card(); below1=m_union_array_card_below.get_array_card_below(below1)) {
                            if (! (Bits(below1) & bits_same_suit_small_rank_card2))
                                continue;
                            array_two_suit_cycle[ntwo_suit_cycle++] = Two_suit_cycle(card1, card2, true);
                            ++count_in_two_suit_cycle[card1.get_id()];
                            ++count_in_two_suit_cycle[card2.get_id()];
                            break;
                        }
                    }
                }
            }
#else
            for (Card card1=m_array_column_top[column1]; card1.is_card(); card1=m_array_card_below[card1.get_id()]) {
                if (Bits(card1) & bits_single_suit_cycle)
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
                    for (Card card2=m_array_column_top[column2]; card2!=prev_card1; card2=m_array_card_below[card2.get_id()]) {
                        if (bits_explored_card2.is_set_bit(card2))
                            continue;
                        if (Bits(card2) & bits_single_suit_cycle)
                            continue;
                        if (card2.suit() == suit_card1)
                            continue;
                        bits_explored_card2.set_bit(card2);
                        Bits bits_same_suit_small_rank_card2 = Bits::same_suit_small_rank(card2);
                        for (Card below1=m_array_card_below[card1.get_id()]; below1.is_card(); below1=m_array_card_below[below1.get_id()]) {
                            if (! (Bits(below1) & bits_same_suit_small_rank_card2))
                                continue;
                            array_two_suit_cycle[ntwo_suit_cycle++] = Two_suit_cycle(card1, card2, true);
                            ++count_in_two_suit_cycle[card1.get_id()];
                            ++count_in_two_suit_cycle[card2.get_id()];
                            break;
                        }
                    }
                }
            }
#endif                        
        }
        if (ntwo_suit_cycle != (int)m_ntwo_suit_cycle)
            throw E(__LINE__);
        for (int i=0; i<ntwo_suit_cycle; ++i) {
            int check = 0;
            for (int j=0; j<(int)m_ntwo_suit_cycle; ++j) {
                if ((array_two_suit_cycle[i].get_card1() == m_array_two_suit_cycle[j].get_card1()) && (array_two_suit_cycle[i].get_card2() == m_array_two_suit_cycle[j].get_card2())) {
                    check = 1;
                    break;
                }
                else if ((array_two_suit_cycle[i].get_card1() == m_array_two_suit_cycle[j].get_card2()) && (array_two_suit_cycle[i].get_card1() == m_array_two_suit_cycle[j].get_card2())) {
                    check = 1;
                    break;
                }
            }
            if (! check)
                throw E(__LINE__);
        }
        for (int id=0; id<CARD_SIZE; ++id)
            if (count_in_two_suit_cycle[id] != m_count_in_two_suit_cycle[id])
                throw E(__LINE__);
        
    }
    catch (const char *cstr) {
        cerr << cstr << endl;
        return false; 
    }
    return true;  
}

void Position::initialize(const Card (&field)[TABLEAU_COLUMN_SIZE][64], const Card (&array_homecell)[HOMECELL_SIZE], const Card (&array_freecell)[FREECELL_SIZE]) noexcept {
#ifdef TEST_ZKEY
    m_union_array_card_below = Position::Union_array_card();
#endif
    m_zobrist_key = 0ULL;
    m_ncard_tableau = m_ncard_freecell = 0;
    m_bits_column_top.clear();
    fill_n(m_array_column_top, TABLEAU_COLUMN_SIZE, Card());
    fill_n(m_array_location, CARD_SIZE, Position::bad_location);
    for (int column=0; column<TABLEAU_COLUMN_SIZE; ++column) {
        m_array_bits_column_card[column].clear();
        m_array_bits_column_next[column].clear();
        if (! field[column][0].is_card()) 
            continue;
        int h = 0;
        m_array_bits_column_card[column] |= Bits(field[column][h]);
#ifdef TEST_ZKEY
        m_union_array_card_below.set_array_card_below(field[column][h], Card::field());
#else        
        m_array_card_below[field[column][h].get_id()] = Card::field();
#endif        
        m_array_location[field[column][h].get_id()] = column;
        m_zobrist_key ^= table.get(field[column][h], Card::field());
        for (h=1; field[column][h]; ++h) {
            m_array_bits_column_card[column] |= Bits(field[column][h]);
#ifdef TEST_ZKEY
            m_union_array_card_below.set_array_card_below(field[column][h], field[column][h - 1]);
#else            
            m_array_card_below[field[column][h].get_id()] = field[column][h - 1];
#endif            
            m_array_location[field[column][h].get_id()] = column; 
            m_zobrist_key ^= table.get(field[column][h], field[column][h - 1]); 
        }
        m_ncard_tableau += h;
        Card top = field[column][h - 1];
        m_array_column_top[column] = top;
        m_bits_column_top.set_bit(top);
        m_array_bits_column_next[column] = Bits::placeable(top); 
    }

    m_bits_freecell.clear();
    for (int freecell=0; freecell<FREECELL_SIZE; ++freecell) {
        Card card = array_freecell[freecell];
        m_array_freecell[freecell] = card;
        if (! card) 
            continue;
        assert(card.is_card());
        m_ncard_freecell += 1;
#ifdef TEST_ZKEY
        m_union_array_card_below.set_array_card_below(card, Card::freecell());
#else        
        m_array_card_below[card.get_id()] = Card::freecell();
#endif
        m_array_location[card.get_id()] = freecell + 8;
        m_zobrist_key ^= table.get(card, Card::freecell());
        m_bits_freecell.set_bit(card); 
    }

    m_bits_homecell.clear();
    for (int suit=0; suit<SUIT_SIZE; ++suit) {
        Card card = array_homecell[suit];
        m_array_homecell[suit] = card;
        if (! card) 
            continue;
        assert(card.is_card() && (card.suit() == suit));
#ifdef TEST_ZKEY
        m_union_array_card_below.set_array_card_below(Card(suit, 0), Card::homecell());
#else        
        m_array_card_below[Card(suit, 0).get_id()] = Card::homecell();
#endif        
        m_array_location[Card(suit, 0).get_id()] = suit + 12;
        m_zobrist_key ^= table.get(Card(suit, 0), Card::homecell());
        m_bits_homecell.set_bit(Card(suit, 0).get_id());
        for (int rank=1; rank<=card.rank(); ++rank) {
#ifdef TEST_ZKEY
            m_union_array_card_below.set_array_card_below(Card(suit, rank), Card(suit, rank - 1));
#else            
            m_array_card_below[Card(suit, rank).get_id()] = Card(suit, rank - 1);
#endif            
            m_array_location[Card(suit, rank).get_id()] = suit + 12;
            m_zobrist_key ^= table.get(Card(suit, rank), Card(suit, rank - 1) );
            m_bits_homecell.set_bit(Card(suit, rank).get_id()); 
        } 
    }

    m_bits_homecell_next.clear();
    for (int suit=0; suit<SUIT_SIZE; ++suit) {
        Bits bits_next(Card(suit, 0).get_id());
        for (int homecell=0; homecell<HOMECELL_SIZE; ++homecell) {
            if (! m_array_homecell[homecell].is_card()) 
                break;
            if (m_array_homecell[homecell].suit() != suit) 
                continue;
            bits_next = Bits::next(m_array_homecell[homecell]);
            break; 
        }
        m_bits_homecell_next |= bits_next; 
    }

    find_cycle();
}

Position::Position(int seed) noexcept {
    int rest = CARD_SIZE;
    Card deck[CARD_SIZE];
    Card field[TABLEAU_COLUMN_SIZE][64];
    Card array_homecell[HOMECELL_SIZE], array_freecell[FREECELL_SIZE];

    for (int rank=0; rank<RANK_SIZE; ++rank) {
        deck[rank * 4 + 0] = Card(Card::club, rank);
        deck[rank * 4 + 1] = Card(Card::diamond, rank);
        deck[rank * 4 + 2] = Card(Card::heart, rank);
        deck[rank * 4 + 3] = Card(Card::spade, rank); 
    }
    for (int column=0; column<TABLEAU_COLUMN_SIZE; ++column)
        for (int h=0; h<64; ++h) 
            field[column][h] = Card();

    for (int i=0; i<CARD_SIZE; ++i) {
        seed = seed * 214013 + 2531011;
        int index = ((seed >> 16) & 32767) % rest;
        field[i % TABLEAU_COLUMN_SIZE][i / TABLEAU_COLUMN_SIZE] = deck[index];
        deck[index] = deck[--rest]; 
    }
    
    fill_n(array_homecell, HOMECELL_SIZE, Card());
    fill_n(array_freecell, FREECELL_SIZE, Card());
    initialize(field, array_freecell, array_homecell);
    assert(correct());
}

bool Position::correct_Action(const Action& action) const noexcept {
    if (! action.correct()) 
        return false;
    Card card;
    if (action.get_from() <= 7) 
        card = m_array_column_top[action.get_from()];
    else                   
        card = m_array_freecell[action.get_from() - TABLEAU_COLUMN_SIZE];
    
    if (! card) 
      return false;
    
    if (action.get_to() <= 7) {
        if (! m_array_column_top[action.get_to()]) 
            return true;
        return Bits(card) & m_array_bits_column_next[action.get_to()]; 
    }
    else if (action.get_to() <= 11) 
        return ! m_array_freecell[action.get_to() - TABLEAU_COLUMN_SIZE];
    else {
        if ((card.suit() + 12) != action.get_to()) 
            return false;
        return Bits(card) & m_bits_homecell_next; 
    }
}

void Position::find_cycle() noexcept {
    m_nsingle_suit_cycle = 0;
    m_bits_single_suit_cycle.clear();
    for (int column=0; column<TABLEAU_COLUMN_SIZE; ++column) {
#ifdef TEST_ZKEY        
        for (Card card=m_array_column_top[column]; card.is_card(); card=m_union_array_card_below.get_array_card_below(card)) {
            Bits bits_same_suit_small_rank = Bits::same_suit_small_rank(card);
            for (Card below=m_union_array_card_below.get_array_card_below(card); below.is_card(); below=m_union_array_card_below.get_array_card_below(below)) {
                if (! (Bits(below) & bits_same_suit_small_rank))
                    continue;
                m_array_single_suit_cycle[m_nsingle_suit_cycle++] = card;
                m_bits_single_suit_cycle.set_bit(card);
                break;
            }
        }
#else
        for (Card card=m_array_column_top[column]; card.is_card(); card=m_array_card_below[card.get_id()]) {
            Bits bits_same_suit_small_rank = Bits::same_suit_small_rank(card);
            for (Card below=m_array_card_below[card.get_id()]; below.is_card(); below=m_array_card_below[below.get_id()]) {
                if (! (Bits(below) & bits_same_suit_small_rank))
                    continue;
                m_array_single_suit_cycle[m_nsingle_suit_cycle++] = card;
                m_bits_single_suit_cycle.set_bit(card);
                break;
            }
        }
#endif                
    }
    assert(m_nsingle_suit_cycle <= MAX_SINGLE_SUIT_CYCLE_SIZE);

    m_ntwo_suit_cycle = 0;
    memset(m_count_in_two_suit_cycle, 0, sizeof(m_count_in_two_suit_cycle));
    for (int column1=0; column1<TABLEAU_COLUMN_SIZE; ++column1) {
#ifdef TEST_ZKEY        
        for (Card card1=m_array_column_top[column1]; card1.is_card(); card1=m_union_array_card_below.get_array_card_below(card1)) {
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
                for (Card card2=m_array_column_top[column2]; card2!=prev_card1; card2=m_union_array_card_below.get_array_card_below(card2)) {
                    if (bits_explored_card2.is_set_bit(card2))
                        continue;
                    if (Bits(card2) & m_bits_single_suit_cycle)
                        continue;
                    if (card2.suit() == suit_card1)
                        continue;
                    bits_explored_card2.set_bit(card2);
                    Bits bits_same_suit_small_rank_card2 = Bits::same_suit_small_rank(card2);
                    for (Card below1=m_union_array_card_below.get_array_card_below(card1); below1.is_card(); below1=m_union_array_card_below.get_array_card_below(below1)) {
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
#else
        for (Card card1=m_array_column_top[column1]; card1.is_card(); card1=m_array_card_below[card1.get_id()]) {
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
                for (Card card2=m_array_column_top[column2]; card2!=prev_card1; card2=m_array_card_below[card2.get_id()]) {
                    if (bits_explored_card2.is_set_bit(card2))
                        continue;
                    if (Bits(card2) & m_bits_single_suit_cycle)
                        continue;
                    if (card2.suit() == suit_card1)
                        continue;
                    bits_explored_card2.set_bit(card2);
                    Bits bits_same_suit_small_rank_card2 = Bits::same_suit_small_rank(card2);
                    for (Card below1=m_array_card_below[card1.get_id()]; below1.is_card(); below1=m_array_card_below[below1.get_id()]) {
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
#endif                
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

    if (Bits::same_suit_small_rank(card1) & m_array_bits_column_card[m_array_location[card1.get_id()]]) {
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
#ifdef TEST_ZKEY
        for (Card card2=m_array_column_top[column2]; card2!=prev_card1; card2=m_union_array_card_below.get_array_card_below(card2)) {
            if (bits_explored_card2.is_set_bit(card2))
                continue;
            if (Bits(card2) & m_bits_single_suit_cycle)
                continue;
            if (card2.suit() == suit_card1)
                continue;
            bits_explored_card2.set_bit(card2);
            Bits bits_same_suit_small_rank_card2 = Bits::same_suit_small_rank(card2);
            for (Card below1=m_union_array_card_below.get_array_card_below(card1); below1.is_card(); below1=m_union_array_card_below.get_array_card_below(below1)) {
                if (! (Bits(below1) & bits_same_suit_small_rank_card2))
                    continue;
                m_array_two_suit_cycle[m_ntwo_suit_cycle++] = Two_suit_cycle(card1, card2, true);
                ++m_count_in_two_suit_cycle[card1.get_id()];
                ++m_count_in_two_suit_cycle[card2.get_id()];
                break;
            }
        }
#else
        for (Card card2=m_array_column_top[column2]; card2!=prev_card1; card2=m_array_card_below[card2.get_id()]) {
            if (bits_explored_card2.is_set_bit(card2))
                continue;
            if (Bits(card2) & m_bits_single_suit_cycle)
                continue;
            if (card2.suit() == suit_card1)
                continue;
            bits_explored_card2.set_bit(card2);
            Bits bits_same_suit_small_rank_card2 = Bits::same_suit_small_rank(card2);
            for (Card below1=m_array_card_below[card1.get_id()]; below1.is_card(); below1=m_array_card_below[below1.get_id()]) {
                if (! (Bits(below1) & bits_same_suit_small_rank_card2))
                    continue;
                m_array_two_suit_cycle[m_ntwo_suit_cycle++] = Two_suit_cycle(card1, card2, true);
                ++m_count_in_two_suit_cycle[card1.get_id()];
                ++m_count_in_two_suit_cycle[card2.get_id()];
                break;
            }
        }
#endif
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
    size += dfs(index, 0, CARD_SIZE);

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
    int array_index_deleted_cycle[CARD_SIZE];
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

int Position::gen_actions(Action (&actions)[MAX_ACTION_SIZE]) const noexcept {
    assert(correct());
    int naction = 0;
    Bits bits_from = m_bits_freecell | m_bits_column_top;
    Bits bits_possible;

    for (int column=0; column<TABLEAU_COLUMN_SIZE; ++column) {
        bits_possible = bits_from & m_array_bits_column_next[column];
        for (Card card=bits_possible.pop(); card; card=bits_possible.pop())
            actions[naction++] = Action(m_array_location[card.get_id()], column); 
    }
      
    bits_possible = bits_from & m_bits_homecell_next;
    for (Card card=bits_possible.pop(); card; card=bits_possible.pop())
        actions[naction++] = Action(m_array_location[card.get_id()], card.suit() + 12 );

    int freecell_empty = find_freecell_empty();
    if (freecell_empty < FREECELL_SIZE)
        for (int column=0; column<TABLEAU_COLUMN_SIZE; ++column)
            if (m_array_column_top[column])
                actions[naction++] = Action(column, freecell_empty + TABLEAU_COLUMN_SIZE);

    int column_empty = find_column_empty();
    if (column_empty < TABLEAU_COLUMN_SIZE) {
        for (int freecell=0; freecell<FREECELL_SIZE; ++freecell)
            if (m_array_freecell[freecell])
                actions[naction++] = Action(freecell + TABLEAU_COLUMN_SIZE, column_empty);

        for (int column=0; column<TABLEAU_COLUMN_SIZE; ++column) {
            Card card = m_array_column_top[column];
#ifdef TEST_ZKEY
            if (card.is_card() && m_union_array_card_below.get_array_card_below(card).is_card())
                actions[naction++] = Position::Action(column, column_empty);
#else            
            if (card.is_card() && m_array_card_below[card.get_id()].is_card())
                actions[naction++] = Action(column, column_empty);
#endif       
        } 
    }
    return naction; 
}

int Position::move_auto(Action* history) noexcept {
    assert(correct());
    int naction = 0;
    Bits bits_from = m_bits_freecell | m_bits_column_top;
    Bits bits_possible = bits_from & m_bits_homecell_next;

    for (Card card=bits_possible.pop(); card; card=bits_possible.pop()) {
        if (card.rank() > 0) {
            Bits bits_placeable_in_homecell = m_bits_homecell & Bits::placeable(card);
            if (bits_placeable_in_homecell.popu() < 2) 
                continue; 
        }
        history[naction] = Action(m_array_location[card.get_id()], card.suit() + 12);
        make(history[naction++]);
        bits_from = m_bits_freecell | m_bits_column_top;
        bits_possible = bits_from & m_bits_homecell_next;
    }
    return naction;
}

void Position::make(const Action& action) noexcept {
    assert(correct_Action(action));
    
    if ((action.get_from() <= 7) && (action.get_to() >= 12)) {
        int column = action.get_from();
        Card card = m_array_column_top[column];
#ifdef TEST_ZKEY
        Card below = m_union_array_card_below.get_array_card_below(card);
#else        
        Card below = m_array_card_below[card.get_id()];
#endif        

        m_array_bits_column_card[column].clear_bit(card);
        m_bits_homecell.set_bit(card);
        m_bits_homecell_next ^= Bits(card) | Bits::next(card);
        m_bits_column_top.clear_bit(card);
        m_array_location[card.get_id()] = card.suit() + 12;
        m_ncard_tableau -= 1;
        m_array_homecell[card.suit()] = card;
        m_array_column_top[column] = below;
        update_array_card_below(card, obtain_below_homecell(card));
        if (below.is_card()) {
            m_bits_column_top.set_bit(below);
            m_array_bits_column_next[column] = Bits::placeable(below);
            m_array_column_top[column] = below;
            delete_cycle(card);
        }
        else {
            m_array_column_top[column] = Card();
            m_array_bits_column_next[column].clear(); 
        }
    }

    else if (action.get_to() >= 12) {
        int freecell = action.get_from() - TABLEAU_COLUMN_SIZE;
        Card card = m_array_freecell[freecell];

        m_bits_freecell.clear_bit(card.get_id());
        m_bits_homecell.set_bit(card.get_id());
        m_bits_homecell_next ^= Bits(card) | Bits::next(card);
        m_array_homecell[card.suit()] = card;
        m_array_freecell[freecell] = Card();
        update_array_card_below(card, obtain_below_homecell(card));
        m_ncard_freecell -= 1;
        m_array_location[card.get_id()] = card.suit() + 12;
    }
    
    else if ((action.get_from() <= 7) && (action.get_to() <= 7)) {
        int column_from = action.get_from();
        int column_to   = action.get_to();
        Card card = m_array_column_top[column_from];
        Card top_to = m_array_column_top[column_to];
#ifdef TEST_ZKEY
        Card below_from = m_union_array_card_below.get_array_card_below(card);
#else        
        Card below_from = m_array_card_below[card.get_id()];
#endif        

        m_array_bits_column_card[column_from].clear_bit(card);
        m_array_bits_column_card[column_to].set_bit(card);
        m_array_bits_column_next[column_to] = Bits::placeable(card);
        m_array_column_top[column_to] = card;
        m_array_location[card.get_id()] = column_to;
        if (below_from.is_card()) {
            m_array_bits_column_next[column_from] = Bits::placeable(below_from);
            m_bits_column_top.set_bit(below_from);
            m_array_column_top[column_from] = below_from;
        }
        else {
            m_array_bits_column_next[column_from].clear();
            m_array_column_top[column_from] = Card(); 
        }

        if (top_to.is_card()) {
            update_array_card_below(card, top_to);
            m_bits_column_top.clear_bit(top_to);
        }
        else {
            update_array_card_below(card, Card::field() ); 
        }

        if (below_from.is_card())
            delete_cycle(card);
        if (top_to.is_card())
            add_cycle(card);
    }
    
    else if (action.get_to() <= 7) {
        int freecell = action.get_from() - TABLEAU_COLUMN_SIZE;
        int column = action.get_to();
        Card card = m_array_freecell[freecell];
        Card top = m_array_column_top[column];

        m_array_bits_column_card[column].set_bit(card);
        m_bits_freecell.clear_bit(card);
        m_bits_column_top.set_bit(card);
        m_array_bits_column_next[column] ^= Bits::placeable(card);
        m_array_freecell[freecell] = Card();
        m_array_column_top[column] = card;
        m_array_location[card.get_id()] = column;
        m_ncard_tableau += 1;
        m_ncard_freecell -= 1;
        if (top.is_card()) {
            m_bits_column_top.clear_bit(top);
            m_array_bits_column_next[column] ^= Bits::placeable(top);
            update_array_card_below(card, top);
            add_cycle(card);
        }
        else {
            update_array_card_below(card, Card::field() ); 
        }
    }
    
    else {
        assert((action.get_from() <= 7) && (action.get_to() <= 11));
        int freecell = action.get_to() - TABLEAU_COLUMN_SIZE;
        int column = action.get_from();
        Card card = m_array_column_top[column];
#ifdef TEST_ZKEY
        Card below = m_union_array_card_below.get_array_card_below(card);
#else        
        Card below = m_array_card_below[card.get_id()];
#endif        

        m_array_bits_column_card[column].clear_bit(card);
        m_bits_freecell.set_bit(card);
        m_bits_column_top.clear_bit(card);
        m_array_bits_column_next[column] ^= Bits::placeable(card);
        update_array_card_below(card, Card::freecell() );
        m_array_freecell[freecell] = card;
        m_ncard_tableau -= 1;
        m_ncard_freecell += 1;
        m_array_location[card.get_id()] = action.get_to();
        if (below.is_card()) {
            m_bits_column_top.set_bit(below);
            m_array_bits_column_next[column] ^= Bits::placeable(below);
            m_array_column_top[column] = below;
            delete_cycle(card);
        }
        else {
            m_array_column_top[column] = Card(); 
        } 
    }

    assert(correct());
}

void Position::unmake(const Action& action) noexcept {
    if ((action.get_from() <= 7) && (action.get_to() >= 12)) {
        int column = action.get_from();
        Card card = m_array_homecell[action.get_to() - 12];
        Card below = m_array_column_top[column];

        m_array_bits_column_card[column].set_bit(card);
        m_array_bits_column_next[column] = Bits::placeable(card);
        m_bits_homecell.clear_bit(card);
        m_bits_homecell_next ^= Bits(card) | Bits::next(card);
        m_bits_column_top.set_bit(card);
        m_array_location[card.get_id()] = column;
        m_ncard_tableau += 1;
        m_array_homecell[card.suit()] = card.prev();
        m_array_column_top[column] = card;
        if (below.is_card()) {
            m_bits_column_top.clear_bit(below);
            update_array_card_below(card, below);
            add_cycle(card);
        }
        else {
            update_array_card_below(card, Card::field()); 
        } 
    }

    else if (action.get_to() >= 12) {
        int freecell = action.get_from() - TABLEAU_COLUMN_SIZE;
        Card card = m_array_homecell[action.get_to() - 12];

        m_bits_freecell.set_bit(card.get_id());
        m_bits_homecell.clear_bit(card.get_id());
        m_bits_homecell_next ^= Bits(card) | Bits::next(card);
        m_array_homecell[card.suit()] = card.prev();
        m_array_freecell[freecell] = card;
        update_array_card_below(card, Card::freecell());
        m_ncard_freecell += 1;
        m_array_location[card.get_id()] = action.get_from(); 
    }
    
    else if ((action.get_from() <= 7) && (action.get_to() <= 7)) {
        int column_from = action.get_from();
        int column_to   = action.get_to();
        Card card = m_array_column_top[column_to];
#ifdef TEST_ZKEY
        Card top_to = m_union_array_card_below.get_array_card_below(card);
#else        
        Card top_to = m_array_card_below[card.get_id()];
#endif        
        Card below_from = m_array_column_top[column_from];

        m_array_bits_column_card[column_from].set_bit(card);
        m_array_bits_column_card[column_to].clear_bit(card);
        m_array_bits_column_next[column_from] = Bits::placeable(card);
        m_array_column_top[column_from] = card;
        m_array_location[card.get_id()] = column_from;
        if (below_from.is_card()) {
            m_bits_column_top.clear_bit(below_from);
            update_array_card_below(card, below_from);
        }
        else {
            update_array_card_below(card, Card::field() ); 
        }

        if (top_to.is_card()) {
            m_array_bits_column_next[column_to] = Bits::placeable(top_to);
            m_bits_column_top.set_bit(top_to);
            m_array_column_top[column_to] = top_to;
            delete_cycle(card);
        }
        else {
            m_array_bits_column_next[column_to].clear();
            m_array_column_top[column_to] = Card(); 
        }

        if (below_from.is_card())
            add_cycle(card);
    }
    
    // フリーセルから列
    else if (action.get_to() <= 7) {
        int freecell = action.get_from() - TABLEAU_COLUMN_SIZE;
        int column = action.get_to();
        Card card = m_array_column_top[column];
#ifdef TEST_ZKEY
        Card top = m_union_array_card_below.get_array_card_below(card);
#else        
        Card top = m_array_card_below[card.get_id()];
#endif        

        m_array_bits_column_card[column].clear_bit(card);
        m_bits_freecell.set_bit(card);
        m_bits_column_top.clear_bit(card);
        m_array_freecell[freecell] = card;
        update_array_card_below(card, Card::freecell());
        m_array_location[card.get_id()] = action.get_from();
        m_ncard_tableau -= 1;
        m_ncard_freecell += 1;
        if (top.is_card()) {
            m_array_bits_column_next[column] = Bits::placeable(top);
            m_bits_column_top.set_bit(top);
            m_array_column_top[column] = top;
            delete_cycle(card);
        }
        else {
            m_array_bits_column_next[column].clear();
            m_array_column_top[column] = Card(); 
        } 
    }
    
    else {
        assert((action.get_from() <= 7) && (action.get_to() <= 11));
        int freecell = action.get_to() - TABLEAU_COLUMN_SIZE;
        int column = action.get_from();
        Card card = m_array_freecell[freecell];
        Card below = m_array_column_top[column];

        m_array_bits_column_card[column].set_bit(card);     
        m_bits_freecell.clear_bit(card);
        m_bits_column_top.set_bit(card);
        m_array_bits_column_next[column] = Bits::placeable(card);
        m_array_column_top[column] = card;
        m_array_freecell[freecell] = Card();
        m_ncard_tableau += 1;
        m_ncard_freecell -= 1;
        m_array_location[card.get_id()] = action.get_from();
        if (below.is_card()) {
            update_array_card_below(card, below);
            m_bits_column_top.clear_bit(below);
            add_cycle(card);
        }
        else {
            update_array_card_below(card, Card::field()); 
        } 
    }

    assert(correct());
    assert(correct_Action(action));
}