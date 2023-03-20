#include "position.hpp"
Position::Table Position::table;
constexpr int Position::bad_location;

bool Position::Entry_tt::correct() const {
    try {
        if (! ((m_h_cost >= 0U) && (m_h_cost <= MAX_H_COST)))
            throw E(__LINE__);

        for (int i=0; i<SUIT_SIZE; ++i) {
            if (m_candidate_homecell_next[i]) {
                if (! m_candidate_homecell_next[i].is_card())
                    throw E(__LINE__);
            }
            else {
                break;
            }
        }

#ifdef TEST_ZKEY
        for (int id=0; id<CARD_SIZE; ++id) {
            Card bottom = Card(id);
            for (Card below=m_union_array_card_below.get_array_card_below(id); below.is_card(); below=m_union_array_card_below.get_array_card_below(below))
                bottom = below;
            if (! bottom.is_card())
                throw E(__LINE__);
            if ((m_union_array_card_below.get_array_card_below(bottom) != Card::homecell()) && (m_union_array_card_below.get_array_card_below(bottom) != Card::freecell()) && (m_union_array_card_below.get_array_card_below(bottom) != Card::field()))
                throw E(__LINE__);
        }
#endif
    }
    catch (const char *cstr) {
        cerr << cstr << endl;
        return false; 
    }
    return true; 
}

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
        Bits bits_column_top, bits_homecell, bits_freecell, bits_deadlocked;
		Bits array_bits_column_card[TABLEAU_COLUMN_SIZE];
        unsigned char ncard1[CARD_SIZE], ncard2[CARD_SIZE];
    	Card array_homecell_above[HOMECELL_SIZE];
    	Card array_freecell_above[FREECELL_SIZE];
    	Card array_field_above[TABLEAU_COLUMN_SIZE];
    	Card array_card_above[CARD_SIZE];
    	Card array_homecell[HOMECELL_SIZE];
        unsigned char array_ncard_not_deadlocked_below_and[CARD_SIZE];
    	int ncard_homecell = 0;
    	int ncard_tableau = 0;
        int ncard_freecell = 0;
        int ncard_deadlocked = 0;
    	int ncolumn = 0;
    	uint64_t zobrist_key = 0;

    	fill_n(ncard1, CARD_SIZE, 0);
    	fill_n(ncard2, CARD_SIZE, 0);
    	fill_n(array_homecell, HOMECELL_SIZE, Card());
    	fill_n(array_homecell_above, HOMECELL_SIZE, Card());
    	fill_n(array_freecell_above, FREECELL_SIZE, Card());
    	fill_n(array_field_above, TABLEAU_COLUMN_SIZE, Card());
    	fill_n(array_card_above, CARD_SIZE, Card());
        fill_n(array_ncard_not_deadlocked_below_and, CARD_SIZE, 0U);

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

        bits_deadlocked.clear();
        for (int id=0; id<CARD_SIZE; ++id) {
            if (m_array_location[id] >= 8)
                continue;
            Bits bits_deadlock = Bits::same_suit_small_rank(id);
#ifdef TEST_ZKEY
            for (Card below=m_union_array_card_below.get_array_card_below(id); below.is_card(); below=m_union_array_card_below.get_array_card_below(below)) {
                if (bits_deadlock.is_set_bit(below)) {
                    bits_deadlocked.set_bit(id);
                    ncard_deadlocked += 1;
                    break;
                }
            }
#else
            for (Card below=m_array_card_below[id]; below.is_card(); below=m_array_card_below[below.get_id()]) {
                if (bits_deadlock.is_set_bit(below)) {
                    bits_deadlocked.set_bit(id);
                    ncard_deadlocked += 1;
                    break;
                }
            }
#endif
        }
        if (bits_deadlocked != m_bits_deadlocked)
            throw E(__LINE__);
        if (ncard_deadlocked != m_ncard_deadlocked)
            throw E(__LINE__);

        for (int id=0; id<CARD_SIZE; ++id) {
            if (m_array_location[id] >= 8)
                continue;
#ifdef TEST_ZKEY
            for (Card below=Card(id); below.is_card(); below=m_union_array_card_below.get_array_card_below(below)) {
                bool is_deadlocked = false;
                Bits bits_deadlock = Bits::same_suit_small_rank(below);
                for (Card below_below=m_union_array_card_below.get_array_card_below(below); below_below.is_card(); below_below=m_union_array_card_below.get_array_card_below(below_below)) {
                    if (bits_deadlock.is_set_bit(below_below)) {
                        is_deadlocked = true;
                        break;
                    }
                }
                if (! is_deadlocked)
                    array_ncard_not_deadlocked_below_and[id] += 1U;
            }
#else
            for (Card below=Card(id); below.is_card(); below=m_array_card_below[below.get_id()]) {
                bool is_deadlocked = false;
                Bits bits_deadlock = Bits::same_suit_small_rank(below);
                for (Card below_below=m_array_card_below[below.get_id()]; below_below.is_card(); below_below=m_array_card_below[below_below.get_id()]) {
                    if (bits_deadlock.is_set_bit(below_below)) {
                        is_deadlocked = true;
                        break;
                    }
                }
                if (! is_deadlocked)
                    array_ncard_not_deadlocked_below_and[id] += 1U;
            }
#endif
        }
        for (int id=0; id<CARD_SIZE; ++id)
            if (array_ncard_not_deadlocked_below_and[id] != m_array_ncard_not_deadlocked_below_and[id])
                throw E(__LINE__);
    }
    catch (const char *cstr) {
        cerr << cstr << endl;
        return false; 
    }
    return true;  
}

bool Position::correct_for_h() const {
    try { 
        Bits bits_column_top, bits_homecell, bits_freecell, bits_deadlocked;
        unsigned char ncard1[CARD_SIZE], ncard2[CARD_SIZE];
        Card array_homecell_above[HOMECELL_SIZE];
        Card array_freecell_above[CARD_SIZE];
        Card array_field_above[TABLEAU_COLUMN_SIZE];
        Card array_card_above[CARD_SIZE];
        Card array_homecell[HOMECELL_SIZE];
        unsigned char array_ncard_not_deadlocked_below_and[CARD_SIZE];
        int ncard_homecell = 0;
        int ncard_tableau = 0;
        int ncard_freecell = 0;
        int ncard_deadlocked = 0;
        int ncolumn = 0;
        uint64_t zobrist_key = 0ULL;

        fill_n(ncard1, CARD_SIZE, 0U);
        fill_n(ncard2, CARD_SIZE, 0U);
        fill_n(array_homecell, HOMECELL_SIZE, Card());
        fill_n(array_homecell_above, HOMECELL_SIZE, Card());
        fill_n(array_freecell_above, CARD_SIZE, Card());
        fill_n(array_field_above, TABLEAU_COLUMN_SIZE, Card());
        fill_n(array_card_above, CARD_SIZE, Card());
        fill_n(array_ncard_not_deadlocked_below_and, CARD_SIZE, 0U);

#ifdef TEST_ZKEY
        for (int id=0; id<CARD_SIZE; ++id)
            zobrist_key ^= Position::table.get(Card(id), m_union_array_card_below.get_array_card_below(id));
#else
        for (int id=0; id<CARD_SIZE; ++id)
            zobrist_key ^= Position::table.get(Card(id), m_array_card_below[id]);
#endif
        if (zobrist_key != m_zobrist_key) 
            throw E(__LINE__);
    
        for (int id=0; id<CARD_SIZE; ++id) {
#ifdef TEST_ZKEY
            Card below = m_union_array_card_below.get_array_card_below(id);
#else
            Card below = m_array_card_below[id];
#endif
            if (! below) 
                throw E(__LINE__);

            ncard1[id] += 1U;
            if (below == Card::freecell()) {
                if (ncard_freecell >= CARD_SIZE) 
                    throw E(__LINE__);
                array_freecell_above[ncard_freecell++] = Card(id); 
            }
            else if (below == Card::homecell()) {
                if (ncard_homecell >= HOMECELL_SIZE) 
                    throw E(__LINE__);
                array_homecell_above[ncard_homecell++] = Card(id); 
            }
            else if (below == Card::field()) {
                if (ncolumn >= TABLEAU_COLUMN_SIZE) 
                    throw E(__LINE__);
                array_field_above[ncolumn++] = Card(id); 
            }
            else {
                if (! below.is_card()) 
                    throw E(__LINE__);
                array_card_above[below.get_id()] = Card(id); 
            } 
        }
        if (ncard_freecell != m_ncard_freecell) 
            throw E(__LINE__);

        ncard_freecell = 0;
        for (int freecell=0; freecell<CARD_SIZE; ++freecell) {
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
            if (! card) { 
                bits_homecell_next.set_bit(Card(suit, 0));
            }
            else {
                if (! card.is_card())
                    throw E(__LINE__);
                if (card.rank() < 12)
                    bits_homecell_next.set_bit(card.next()); 
            }
        }
        if (bits_homecell_next != m_bits_homecell_next) 
            throw E(__LINE__);

        bits_deadlocked.clear();
        for (int id=0; id<CARD_SIZE; ++id) {
            if (m_array_location[id] >= 8)
                continue;
            Bits bits_deadlock = Bits::same_suit_small_rank(id);
#ifdef TEST_ZKEY
            for (Card below=m_union_array_card_below.get_array_card_below(id); below.is_card(); below=m_union_array_card_below.get_array_card_below(below)) {
                if (bits_deadlock.is_set_bit(below)) {
                    bits_deadlocked.set_bit(id);
                    ncard_deadlocked += 1;
                    break;
                }
            }
#else
            for (Card below=m_array_card_below[id]; below.is_card(); below=m_array_card_below[below.get_id()]) {
                if (bits_deadlock.is_set_bit(below)) {
                    bits_deadlocked.set_bit(id);
                    ncard_deadlocked += 1;
                    break;
                }
            }
#endif
        }
        if (bits_deadlocked != m_bits_deadlocked)
            throw E(__LINE__);
        if (ncard_deadlocked != m_ncard_deadlocked)
            throw E(__LINE__);

        for (int id=0; id<CARD_SIZE; ++id) {
            if (m_array_location[id] >= 8)
                continue;
#ifdef TEST_ZKEY
            for (Card below=Card(id); below.is_card(); below=m_union_array_card_below.get_array_card_below(below)) {
                bool is_deadlocked = false;
                Bits bits_deadlock = Bits::same_suit_small_rank(below);
                for (Card below_below=m_union_array_card_below.get_array_card_below(below); below_below.is_card(); below_below=m_union_array_card_below.get_array_card_below(below_below)) {
                    if (bits_deadlock.is_set_bit(below_below)) {
                        is_deadlocked = true;
                        break;
                    }
                }
                if (! is_deadlocked)
                    array_ncard_not_deadlocked_below_and[id] += 1U;
            }
#else
            for (Card below=Card(id); below.is_card(); below=m_array_card_below[below.get_id()]) {
                bool is_deadlocked = false;
                Bits bits_deadlock = Bits::same_suit_small_rank(below);
                for (Card below_below=m_array_card_below[below.get_id()]; below_below.is_card(); below_below=m_array_card_below[below_below.get_id()]) {
                    if (bits_deadlock.is_set_bit(below_below)) {
                        is_deadlocked = true;
                        break;
                    }
                }
                if (! is_deadlocked)
                    array_ncard_not_deadlocked_below_and[id] += 1U;
            }
#endif
        }
        for (int id=0; id<CARD_SIZE; ++id)
            if (array_ncard_not_deadlocked_below_and[id] != m_array_ncard_not_deadlocked_below_and[id])
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
    m_ncard_tableau = m_ncard_freecell = m_ncard_deadlocked = 0;
    m_bits_column_top.clear();
    fill_n(m_array_column_top, TABLEAU_COLUMN_SIZE, Card());
    fill_n(m_array_location, CARD_SIZE, Position::bad_location);
    fill_n(m_array_ncard_not_deadlocked_below_and, CARD_SIZE, 0U);
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
        m_zobrist_key ^= table.get(field[column][h], Card::field());
        m_array_location[field[column][h].get_id()] = column;
        for (h=1; field[column][h]; ++h) {
            m_array_bits_column_card[column] |= Bits(field[column][h]);
#ifdef TEST_ZKEY
            m_union_array_card_below.set_array_card_below(field[column][h], field[column][h - 1]);
#else
            m_array_card_below[field[column][h].get_id()] = field[column][h - 1];
#endif
            m_zobrist_key ^= table.get(field[column][h], field[column][h - 1] );
            m_array_location[field[column][h].get_id()] = column; 
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
        m_array_location[card.get_id()] = freecell + 8;
#ifdef TEST_ZKEY
        m_union_array_card_below.set_array_card_below(card, Card::freecell());
#else
        m_array_card_below[card.get_id()] = Card::freecell();
#endif
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
        m_zobrist_key ^= table.get(Card(suit, 0), Card::homecell() );
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

    m_bits_deadlocked.clear();
    for (int id=0; id<CARD_SIZE; ++id) {
        if (m_array_location[id] >= 8)
            continue;
        m_array_ncard_not_deadlocked_below_and[id] = 1U;
        Bits bits_deadlock = Bits::same_suit_small_rank(id);
#ifdef TEST_ZKEY
        for (Card below=m_union_array_card_below.get_array_card_below(id); below.is_card(); below=m_union_array_card_below.get_array_card_below(below)) {
            if (bits_deadlock.is_set_bit(below)) {
                m_bits_deadlocked.set_bit(id);
                ++m_ncard_deadlocked;
                m_array_ncard_not_deadlocked_below_and[id] = 0U;
                break;
            }
        }
#else
        for (Card below=m_array_card_below[id]; below.is_card(); below=m_array_card_below[below.get_id()]) {
            if (bits_deadlock.is_set_bit(below)) {
                m_bits_deadlocked.set_bit(id);
                ++m_ncard_deadlocked;
                m_array_ncard_not_deadlocked_below_and[id] = 0U;
                break;
            }
        }
#endif
    }

    Card stack[MAX_TABLEAU_STACK_SIZE];
    int nstack;
    for (int column=0; column<TABLEAU_COLUMN_SIZE; ++column) {
        nstack = 0;
#ifdef TEST_ZKEY
        for (Card below=m_array_column_top[column]; below.is_card(); below=m_union_array_card_below.get_array_card_below(below))
            stack[nstack++] = below;
#else
        for (Card below=m_array_column_top[column]; below.is_card(); below=m_array_card_below[below.get_id()])
            stack[nstack++] = below;
#endif
        for (int h=nstack-2; h>=0; --h)
            m_array_ncard_not_deadlocked_below_and[stack[h].get_id()] += m_array_ncard_not_deadlocked_below_and[stack[h + 1].get_id()];
    }
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

bool Position::correct_Action(const Position::Action& action) const noexcept {
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
    else if (action.get_to() <= 11) {
        return ! m_array_freecell[action.get_to() - TABLEAU_COLUMN_SIZE];
    }
    else {
        if ((card.suit() + 12) != action.get_to()) 
            return false;
        return Bits(card) & m_bits_homecell_next; 
    }
}

bool Position::correct_Action(const Position::Action_for_h& action) const noexcept {
    if (! action.correct()) 
        return false;

    const Card& card = action.get_card();
    int from = action.get_from();
    int to = action.get_to();
    if (from != m_array_location[card.get_id()])
        return false;

    if (to == 8) {
        if (m_array_column_top[from] != card)
            return false;
        if (! m_bits_column_top.is_set_bit(card))
            return false;
        if (m_bits_freecell.is_set_bit(card))
            return false;
        if (m_ncard_freecell >= CARD_SIZE)
            return false;
        if (m_bits_homecell_next.is_set_bit(card))
            return false;
    }
    else {
        Card prev = card.prev();
        if (prev.is_card())
            if (! m_bits_homecell.is_set_bit(prev))
                return false;
        if (m_bits_homecell.is_set_bit(card))
            return false;
        if (card.suit() != to - 12)
            return false;
        if (m_array_homecell[to - 12].is_card()) {
            if (card != m_array_homecell[to - 12].next())
                return false;
        }
        else {
            if (card != Card(to - 12, 0))
                return false;
        }
        if (! m_bits_homecell_next.is_set_bit(card))
            return false;

        if (from >= 8) {
            if (! m_bits_freecell.is_set_bit(card))
                return false;
        }
        else {
            if (m_array_column_top[from] != card)
                return false;
            if (! m_bits_column_top.is_set_bit(card))
                return false;
            if (m_bits_deadlocked.is_set_bit(card))
                return false;
        }
    }
    return true;
} 

int Position::obtain_lower_h_cost(Card* candidate_homecell_next) noexcept {
    if (m_ncard_freecell + m_ncard_tableau == 0)
        return 0;
    assert(correct_for_h());
    assert(m_bits_homecell_next);
    pair<int, Card> a[4];
    int n1 = 0, n2 = 0;
    int c = 0;
    Card next;
    constexpr unsigned char tbl[TABLEAU_COLUMN_SIZE] = {1U << 0, 1U << 1, 1U << 2, 1U << 3, 1U << 4, 1U << 5, 1U << 6, 1U << 7};
    
    for (int suit=0; suit<SUIT_SIZE; ++suit) {
        if (m_array_homecell[suit].is_card())
            next = m_array_homecell[suit].next();
        else
            next = Card(suit, 0);
        if (! next.is_card())
            continue;
        a[n1++] = {m_array_ncard_not_deadlocked_below_and[m_array_column_top[m_array_location[next.get_id()]].get_id()] - m_array_ncard_not_deadlocked_below_and[next.get_id()], next};
    }
    sort(a, a + n1, [](const pair<int, Card> &a, const pair<int, Card> &b){ return a.first < b.first; });
    for (int i=0; i<n1; ++i) {
        if (c & tbl[m_array_location[a[i].second.get_id()]])
            continue;
        c |= tbl[m_array_location[a[i].second.get_id()]];
        candidate_homecell_next[n2++] = a[i].second;
    }
    if (n2 < HOMECELL_SIZE)
        candidate_homecell_next[n2] = Card();
    return m_ncard_freecell + m_ncard_tableau + m_ncard_deadlocked + a[0].first;
}

int Position::calc_h_cost() noexcept {
    assert(correct_for_h());

    Position::Action_for_h path[MAX_H_COST];
    int naction = move_auto(path);
    int h_cost = naction;
    int th;

    auto it = m_tt.find(m_zobrist_key);
    if (it != m_tt.end()) {
#ifdef TEST_ZKEY
        if (! it->second.identify(*this)) {
            cerr << "Zoblist Key Conflict" << endl;
            terminate(); 
        }
#endif
        th = it->second.get_h_cost();
    }
    else {
        Card candidate_homecell_next[HOMECELL_SIZE];
        th = obtain_lower_h_cost(candidate_homecell_next);
        auto pair = m_tt.emplace(piecewise_construct, forward_as_tuple(m_zobrist_key), forward_as_tuple(th, *this, candidate_homecell_next));
        it = pair.first;
    }

    if (! it->second.get_is_decided()) {
        m_is_solved = false;
        while (true) {
            th = dfstt1(th, path + naction, it->second);
            if (m_is_solved)
                break;
        }
    }

    h_cost += th;
    back_to_parent(path + naction, naction);
    assert(correct_for_h());
    return h_cost;
}

int Position::dfstt1(int th, Position::Action_for_h* path, Position::Entry_tt& entry_parent) noexcept {
    if (check_goal()) {
        m_is_solved = true;
        entry_parent.set_is_decided(true);
        assert(th == 0);
        return 0;
    }

    int th_child, new_th = MAX_H_COST;
    int ncard_rest_and_deadlocked = ncard_rest_for_h() + get_ncard_deadlocked();
    const Card* candidate_homecell_next_parent = entry_parent.get_candidate_homecell_next();
    for (int i=0; i<SUIT_SIZE; ++i) {
        Card next = candidate_homecell_next_parent[i];
        if (! next)
            break;

        int th_estimate = ncard_rest_and_deadlocked + obtain_ncard_not_deadlocked_above(next);
        if (th_estimate > th) {
            new_th = min(new_th, th_estimate);
            break;
        }

        int cost = move_to_homecell_next(next, path);
        cost += move_auto(path + cost);

        auto it = m_tt.find(m_zobrist_key);
        if (it != m_tt.end()) {
#ifdef TEST_ZKEY
            if (! it->second.identify(*this)) {
                cerr << "Zoblist Key Conflict" << endl;
                terminate(); 
            }
#endif
            th_child = it->second.get_h_cost();
        }
        else {
            Card candidate_homecell_next_child[HOMECELL_SIZE];
            th_child = obtain_lower_h_cost(candidate_homecell_next_child);
            auto pair = m_tt.emplace(piecewise_construct, forward_as_tuple(m_zobrist_key), forward_as_tuple(th_child, *this, candidate_homecell_next_child));
            it = pair.first;
        }

        assert(cost + th_child >= th);
        if (cost + th_child <= th) {
            if (it->second.get_is_decided()) {
                assert(cost + th_child == th);
                new_th = th;
                m_is_solved = true;
            }
            else {
                new_th = min(new_th, cost + dfstt1(th - cost, path + cost, it->second));
            }
        }
        else {
            new_th = min(new_th, cost + th_child);
        }

        back_to_parent(path + cost, cost);
        assert(correct_for_h());

        if (m_is_solved) {
            assert(th == new_th);
            entry_parent.set_is_decided(true);
            break;
        }
    }

    assert(new_th >= entry_parent.get_h_cost());
    entry_parent.set_h_cost(new_th);
    return new_th;
}  

int Position::gen_actions(Position::Action (&actions)[MAX_ACTION_SIZE]) const noexcept {
    assert(correct());
    int naction = 0;
    Bits bits_from = m_bits_freecell | m_bits_column_top;
    Bits bits_possible;

    for (int column=0; column<TABLEAU_COLUMN_SIZE; ++column) {
        bits_possible = bits_from & m_array_bits_column_next[column];
        for (Card card=bits_possible.pop(); card; card=bits_possible.pop())
            actions[naction++] = Position::Action(m_array_location[card.get_id()], column); 
    }
      
    bits_possible = bits_from & m_bits_homecell_next;
    for (Card card=bits_possible.pop(); card; card=bits_possible.pop())
        actions[naction++] = Position::Action(m_array_location[card.get_id()], card.suit() + 12 );

    int freecell_empty = find_freecell_empty();
    if (freecell_empty < FREECELL_SIZE)
        for (int column=0; column<TABLEAU_COLUMN_SIZE; ++column)
            if (m_array_column_top[column])
                actions[naction++] = Position::Action(column, freecell_empty + TABLEAU_COLUMN_SIZE);

    int column_empty = find_column_empty();
    if (column_empty < TABLEAU_COLUMN_SIZE) {
        for (int freecell=0; freecell<FREECELL_SIZE; ++freecell)
            if (m_array_freecell[freecell])
                actions[naction++] = Position::Action(freecell + TABLEAU_COLUMN_SIZE, column_empty);

        for (int column=0; column<TABLEAU_COLUMN_SIZE; ++column) {
            Card card = m_array_column_top[column];
#ifdef TEST_ZKEY
            if (card.is_card() && m_union_array_card_below.get_array_card_below(card).is_card())
                actions[naction++] = Position::Action(column, column_empty);
#else
            if (card.is_card() && m_array_card_below[card.get_id()].is_card())
                actions[naction++] = Position::Action(column, column_empty);
#endif
        } 
    }
    return naction; 
}

int Position::move_to_homecell_next(const Card& next, Position::Action_for_h* history) noexcept {
    assert(m_bits_homecell_next.is_set_bit(next));
    int naction = 0;
    
    unsigned char column = m_array_location[next.get_id()];
    for (Card card=m_array_column_top[column]; card!=next; card=m_array_column_top[column]) {
        assert(card.is_card());
        history[naction] = Position::Action_for_h(card, column, 8);
        make(history[naction++]);
    }
    history[naction] = Position::Action_for_h(next, column, next.suit() + 12);
    make(history[naction++]);
    assert(correct_for_h());
    return naction;
}

int Position::move_auto(Position::Action* history) noexcept {
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
        history[naction] = Position::Action(m_array_location[card.get_id()], card.suit() + 12);
        make(history[naction++]);
        bits_from = m_bits_freecell | m_bits_column_top;
        bits_possible = bits_from & m_bits_homecell_next;
    }
    return naction;
}

int Position::move_auto(Position::Action_for_h* history) noexcept {
    Card card;
    int naction = 0;
    Bits bits_from = m_bits_freecell | m_bits_column_top;
    Bits bits_possible = bits_from & m_bits_homecell_next;
    Bits bits_deadlocked_top = m_bits_column_top & m_bits_deadlocked;

    while (bits_possible || bits_deadlocked_top) {
        if (bits_possible) {
            card = bits_possible.pop();
            history[naction] = Position::Action_for_h(card, m_array_location[card.get_id()], card.suit() + 12);
            make(history[naction++]); 
        }
        if (bits_deadlocked_top) {
            card = bits_deadlocked_top.pop();
            history[naction] = Position::Action_for_h(card, m_array_location[card.get_id()], 8);
            make(history[naction++]); 
        }
        bits_from = m_bits_freecell | m_bits_column_top;
        bits_possible = bits_from & m_bits_homecell_next;
        bits_deadlocked_top = m_bits_column_top & m_bits_deadlocked;

    }
    assert(correct_for_h());
    return naction;
}

void Position::make(const Position::Action& action) noexcept {
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
        m_array_ncard_not_deadlocked_below_and[card.get_id()] = 0U;
        update_array_card_below(card, obtain_top_in_homecell(card));
        if (below.is_card()) {
            m_bits_column_top.set_bit(below);
            m_array_bits_column_next[column] = Bits::placeable(below);
            m_array_column_top[column] = below;
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
        update_array_card_below(card, obtain_top_in_homecell(card));
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
            m_array_ncard_not_deadlocked_below_and[card.get_id()] = m_array_ncard_not_deadlocked_below_and[top_to.get_id()];
        }
        else {
            update_array_card_below(card, Card::field());
            m_array_ncard_not_deadlocked_below_and[card.get_id()] = 0U;
        }

        if (m_bits_deadlocked.is_set_bit(card)) {
            if (! (m_array_bits_column_card[column_to] & Bits::same_suit_small_rank(card))) {
                m_bits_deadlocked.clear_bit(card);
                m_ncard_deadlocked -= 1;
            }
        }
        else {
            assert(! m_bits_deadlocked.is_set_bit(card));
            if (m_array_bits_column_card[column_to] & Bits::same_suit_small_rank(card)) {
                m_bits_deadlocked.set_bit(card);
                m_ncard_deadlocked += 1;
            }
        }

        if (! m_bits_deadlocked.is_set_bit(card))
            m_array_ncard_not_deadlocked_below_and[card.get_id()] += 1U;
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
            m_array_ncard_not_deadlocked_below_and[card.get_id()] = m_array_ncard_not_deadlocked_below_and[top.get_id()];
        }
        else {
            update_array_card_below(card, Card::field()); 
        }
        
        if (m_array_bits_column_card[column] & Bits::same_suit_small_rank(card)) {
            m_bits_deadlocked.set_bit(card);
            m_ncard_deadlocked += 1;
        }
        else {
            m_array_ncard_not_deadlocked_below_and[card.get_id()] += 1U;
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
        m_array_ncard_not_deadlocked_below_and[card.get_id()] = 0U;
        if (below.is_card()) {
            m_bits_column_top.set_bit(below);
            m_array_bits_column_next[column] ^= Bits::placeable(below);
            m_array_column_top[column] = below;
        }
        else {
            m_array_column_top[column] = Card(); 
        }

        if (m_bits_deadlocked.is_set_bit(card)) {
            m_bits_deadlocked.clear_bit(card);
            m_ncard_deadlocked -= 1;
        }
    }

    assert(correct());
}

void Position::make(const Position::Action_for_h& action) noexcept {
    assert(action.correct());
    assert(correct_Action(action));
    Card card = action.get_card();
    int from = action.get_from();
    int to = action.get_to();
    
    if ((from <= 7) && (to >= 12)) {
#ifdef TEST_ZKEY
        Card below = m_union_array_card_below.get_array_card_below(card);
#else
        Card below = m_array_card_below[card.get_id()];
#endif

        m_array_bits_column_card[from].clear_bit(card);
        m_bits_homecell.set_bit(card);
        m_bits_homecell_next ^= Bits(card) | Bits::next(card);
        m_bits_column_top.clear_bit(card);
        m_array_homecell[to - 12] = card;
        m_array_location[card.get_id()] = to;
        m_array_ncard_not_deadlocked_below_and[card.get_id()] = 0U;
        update_array_card_below(card, obtain_top_in_homecell(card));
        m_ncard_tableau -= 1;
        if (below.is_card()) {
            m_bits_column_top.set_bit(below);
            m_array_bits_column_next[from] = Bits::placeable(below);
            m_array_column_top[from] = below;
        }
        else {
            m_array_column_top[from] = Card();
            m_array_bits_column_next[from].clear(); 
        }
    }

    else if (to >= 12) {
        assert(from >= 8);
        m_bits_freecell.clear_bit(card);
        m_bits_homecell.set_bit(card);
        m_bits_homecell_next ^= Bits(card) | Bits::next(card);
        update_array_card_below(card, obtain_top_in_homecell(card));
        m_array_homecell[to - 12] = card;
        m_array_location[card.get_id()] = to;
        m_ncard_freecell -= 1;
    }
    
    else {
        assert((from <= 7) && (to == 8));
#ifdef TEST_ZKEY
        Card below = m_union_array_card_below.get_array_card_below(card);
#else
        Card below = m_array_card_below[card.get_id()];
#endif

        m_array_bits_column_card[from].clear_bit(card);
        m_bits_freecell.set_bit(card);
        m_bits_column_top.clear_bit(card);
        m_array_bits_column_next[from] ^= Bits::placeable(card);
        m_array_ncard_not_deadlocked_below_and[card.get_id()] = 0U;
        update_array_card_below(card, Card::freecell());
        m_array_location[card.get_id()] = to;
        m_ncard_tableau -= 1;
        m_ncard_freecell += 1;
        if (below.is_card()) {
            m_bits_column_top.set_bit(below);
            m_array_bits_column_next[from] ^= Bits::placeable(below);
            m_array_column_top[from] = below;
        }
        else {
            m_array_column_top[from] = Card(); 
        }

        if (m_bits_deadlocked.is_set_bit(card)) {
            m_bits_deadlocked.clear_bit(card);
            m_ncard_deadlocked -= 1;
        }
    } 
    assert(correct_for_h());
}

void Position::unmake(const Position::Action& action) noexcept {
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
            m_array_ncard_not_deadlocked_below_and[card.get_id()] = m_array_ncard_not_deadlocked_below_and[below.get_id()] + 1U;
        }
        else {
            update_array_card_below(card, Card::field());
            m_array_ncard_not_deadlocked_below_and[card.get_id()] = 1U; 
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
            m_array_ncard_not_deadlocked_below_and[card.get_id()] = m_array_ncard_not_deadlocked_below_and[below_from.get_id()];
        }
        else {
            update_array_card_below(card, Card::field());
            m_array_ncard_not_deadlocked_below_and[card.get_id()] = 0U;
        }

        if (top_to.is_card()) {
            m_array_bits_column_next[column_to] = Bits::placeable(top_to);
            m_bits_column_top.set_bit(top_to);
            m_array_column_top[column_to] = top_to;
        }
        else {
            m_array_bits_column_next[column_to].clear();
            m_array_column_top[column_to] = Card(); 
        }

        if (m_bits_deadlocked.is_set_bit(card)) {
            if (! (m_array_bits_column_card[column_from] & Bits::same_suit_small_rank(card))) {
                m_bits_deadlocked.clear_bit(card);
                m_ncard_deadlocked -= 1;
            }
        }
        else {
            assert(! m_bits_deadlocked.is_set_bit(card));
            if (m_array_bits_column_card[column_from] & Bits::same_suit_small_rank(card)) {
                m_bits_deadlocked.set_bit(card);
                m_ncard_deadlocked += 1;
            }
        }

        if (! m_bits_deadlocked.is_set_bit(card))
            m_array_ncard_not_deadlocked_below_and[card.get_id()] += 1U;        
    }
    
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
        m_array_ncard_not_deadlocked_below_and[card.get_id()] = 0U;
        if (top.is_card()) {
            m_array_bits_column_next[column] = Bits::placeable(top);
            m_bits_column_top.set_bit(top);
            m_array_column_top[column] = top;
        }
        else {
            m_array_bits_column_next[column].clear();
            m_array_column_top[column] = Card(); 
        }

        if (m_bits_deadlocked.is_set_bit(card)) {
            m_bits_deadlocked.clear_bit(card);
            m_ncard_deadlocked -= 1;
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
            m_array_ncard_not_deadlocked_below_and[card.get_id()] = m_array_ncard_not_deadlocked_below_and[below.get_id()];
        }
        else {
            update_array_card_below(card, Card::field()); 
        }

        if (m_array_bits_column_card[column] & Bits::same_suit_small_rank(card)) {
            m_bits_deadlocked.set_bit(card);
            m_ncard_deadlocked += 1;
        }
        else {
            m_array_ncard_not_deadlocked_below_and[card.get_id()] += 1U;
        }
    }

    assert(correct());
    assert(correct_Action(action));
}

void Position::unmake(const Position::Action_for_h& action) noexcept {
    assert(action.correct());
    Card card = action.get_card();
    int from = action.get_from();
    int to = action.get_to();
    assert(to == m_array_location[card.get_id()]);

    if ((from <= 7) && (to >= 12)) {
        assert(m_bits_homecell.is_set_bit(card));
        assert(card == m_array_homecell[to - 12]);
        Card below = m_array_column_top[from];

        m_array_bits_column_card[from].set_bit(card);
        m_array_bits_column_next[from] = Bits::placeable(card);
        m_bits_homecell.clear_bit(card);
        m_bits_homecell_next ^= Bits(card) | Bits::next(card);
        m_bits_column_top.set_bit(card);
        m_array_homecell[to - 12] = card.prev();;
        m_array_location[card.get_id()] = from;
        m_array_column_top[from] = card;
        m_ncard_tableau += 1;
        if (below.is_card()) {
            m_bits_column_top.clear_bit(below);
            update_array_card_below(card, below);
            m_array_ncard_not_deadlocked_below_and[card.get_id()] = m_array_ncard_not_deadlocked_below_and[below.get_id()] + 1U;
        }
        else {
            update_array_card_below(card, Card::field());
            m_array_ncard_not_deadlocked_below_and[card.get_id()] = 1U; 
        }
    }

    else if (to >= 12) {
        assert(from >= 8);
        assert(m_bits_homecell.is_set_bit(card));
        assert(card == m_array_homecell[to - 12]);
        m_bits_freecell.set_bit(card);
        m_bits_homecell.clear_bit(card);
        m_bits_homecell_next ^= Bits(card) | Bits::next(card);
        m_array_homecell[to - 12] = card.prev();
        m_array_location[card.get_id()] = from; 
        update_array_card_below(card, Card::freecell());
        m_ncard_freecell += 1;
    }
    
    else {
        assert((from <= 7) && (to == 8));
        assert(m_bits_freecell.is_set_bit(card));
        Card below = m_array_column_top[from];
      
        m_array_bits_column_card[from].set_bit(card);
        m_bits_freecell.clear_bit(card);
        m_bits_column_top.set_bit(card);
        m_array_bits_column_next[from] = Bits::placeable(card);
        m_array_column_top[from] = card;
        m_array_location[card.get_id()] = from;
        m_ncard_tableau += 1;
        m_ncard_freecell -= 1;
        if (below.is_card()) {
            update_array_card_below(card, below);
            m_bits_column_top.clear_bit(below);
            m_array_ncard_not_deadlocked_below_and[card.get_id()] = m_array_ncard_not_deadlocked_below_and[below.get_id()];
        }
        else {
            update_array_card_below(card, Card::field()); 
        }

        if (m_array_bits_column_card[from] & Bits::same_suit_small_rank(card)) {
            m_bits_deadlocked.set_bit(card);
            m_ncard_deadlocked += 1;
        }
        else {
            m_array_ncard_not_deadlocked_below_and[card.get_id()] += 1U;
        }
    }

    assert(correct_for_h());
    assert(correct_Action(action));
}
