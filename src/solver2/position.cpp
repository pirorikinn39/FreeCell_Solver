#include "position.hpp"

bool Position::Entry_tt::correct() const {
    try {
        if (! ((m_h_cost >= 0U) && (m_h_cost <= MAX_H_COST)))
            throw E(__LINE__);

        for (int i=0; i<N_SUIT; ++i) {
            if (m_candidate_homecell_next[i]) {
                if (! m_candidate_homecell_next[i].is_card())
                    throw E(__LINE__);
            }
            else {
                break;
            }
        }

#ifdef TEST_ZKEY
        for (int id=0; id<DECK_SIZE; ++id) {
            Card bottom = Card(id);
            for (Card below=m_row_data.get_below(id); below.is_card(); below=m_row_data.get_below(below))
                bottom = below;
            if (! bottom.is_card())
                throw E(__LINE__);
            if ((m_row_data.get_below(bottom) != Card::homecell()) && (m_row_data.get_below(bottom) != Card::freecell()) && (m_row_data.get_below(bottom) != Card::field()))
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

bool Position::correct() const noexcept {
  try {
    Bits bits_deadlocked;
    int ncard_deadlocked = 0;
    unsigned char array_ncard_not_deadlocked_below_and[DECK_SIZE];
    
    fill_n(array_ncard_not_deadlocked_below_and, DECK_SIZE, 0U);
    for (int id=0; id<DECK_SIZE; ++id) {
      if (m_array_location[id] >= 8) continue;
      Bits bits_deadlock = Bits::same_suit_small_rank(id);
      for (Card below=m_row_data.get_below(id); below.is_card();
	   below=m_row_data.get_below(below)) {
	if (bits_deadlock.is_set_bit(below)) {
	  bits_deadlocked.set_bit(id);
	  ncard_deadlocked += 1;
	  break; } } }
    if (bits_deadlocked != m_bits_deadlocked) throw E(__LINE__);
    if (ncard_deadlocked != m_ncard_deadlocked) throw E(__LINE__);
    
    for (int id=0; id<DECK_SIZE; ++id) {
      if (m_array_location[id] >= 8) continue;
      for (Card below=Card(id); below.is_card(); below=m_row_data.get_below(below)) {
	bool is_deadlocked = false;
	Bits bits_deadlock = Bits::same_suit_small_rank(below);
	for (Card below_below=m_row_data.get_below(below); below_below.is_card();
	     below_below=m_row_data.get_below(below_below)) {
	  if (bits_deadlock.is_set_bit(below_below)) {
	    is_deadlocked = true;
	    break; } }
	if (! is_deadlocked) array_ncard_not_deadlocked_below_and[id] += 1U; } }
    
    for (int id=0; id<DECK_SIZE; ++id)
      if (array_ncard_not_deadlocked_below_and[id]
	  != m_array_ncard_not_deadlocked_below_and[id]) throw E(__LINE__); }
  catch (const char *cstr) {
    cerr << cstr << endl;
    return false; }
  return true; }

bool Position::correct_for_h() const {
    try { 
        Bits bits_column_top, bits_homecell, bits_freecell, bits_deadlocked;
        unsigned char ncard1[DECK_SIZE], ncard2[DECK_SIZE];
        Card array_homecell_above[HOMECELL_SIZE];
        Card array_freecell_above[DECK_SIZE];
        Card array_field_above[TABLEAU_SIZE];
        Card array_card_above[DECK_SIZE];
        Card array_homecell[HOMECELL_SIZE];
        unsigned char array_ncard_not_deadlocked_below_and[DECK_SIZE];
        int ncard_homecell = 0;
        int ncard_tableau = 0;
        int ncard_freecell = 0;
        int ncard_deadlocked = 0;
        int ncolumn = 0;
        uint64_t zobrist_key = 0ULL;

        fill_n(ncard1, DECK_SIZE, 0U);
        fill_n(ncard2, DECK_SIZE, 0U);
        fill_n(array_homecell, HOMECELL_SIZE, Card());
        fill_n(array_homecell_above, HOMECELL_SIZE, Card());
        fill_n(array_freecell_above, DECK_SIZE, Card());
        fill_n(array_field_above, TABLEAU_SIZE, Card());
        fill_n(array_card_above, DECK_SIZE, Card());
        fill_n(array_ncard_not_deadlocked_below_and, DECK_SIZE, 0U);

        for (int id=0; id<DECK_SIZE; ++id)
            zobrist_key ^= Position::table.get(Card(id), m_row_data.get_below(id));
        if (zobrist_key != m_zobrist_key) 
            throw E(__LINE__);
    
        for (int id=0; id<DECK_SIZE; ++id) {
            Card below = m_row_data.get_below(id);
            if (! below) 
                throw E(__LINE__);

            ncard1[id] += 1U;
            if (below == Card::freecell()) {
                if (ncard_freecell >= DECK_SIZE) 
                    throw E(__LINE__);
                array_freecell_above[ncard_freecell++] = Card(id); 
            }
            else if (below == Card::homecell()) {
                if (ncard_homecell >= HOMECELL_SIZE) 
                    throw E(__LINE__);
                array_homecell_above[ncard_homecell++] = Card(id); 
            }
            else if (below == Card::field()) {
                if (ncolumn >= TABLEAU_SIZE) 
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
        for (int freecell=0; freecell<DECK_SIZE; ++freecell) {
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
        for (int id=0; id<DECK_SIZE; ++id) {
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
                for (Card below=m_array_pile_top[from]; below.is_card(); below=m_row_data.get_below(below))
                    if (below == id)
                        is_contained = true;
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
        for (int suit=0; suit<N_SUIT; ++suit)
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
        if (bits_column_top != m_bits_pile_top) 
            throw E(__LINE__);
        if (ncard_tableau != m_ncard_tableau) 
            throw E(__LINE__);

        for (int id=0; id<DECK_SIZE; ++id) {
            if (ncard1[id] != ncard2[id])    
                throw E(__LINE__);                    
            if (ncard1[id] > 1) 
                throw E(__LINE__); 
        }

        bits_column_top.clear();
        for (int column=0; column<TABLEAU_SIZE; ++column) {
            Card column_top = m_array_pile_top[column];
            if (! column_top) {
                if (m_array_bits_pile_next[column]) 
                    throw E(__LINE__);
                continue; 
            }
            if (! column_top.is_card()) 
                throw E(__LINE__);
            if (bits_column_top.is_set_bit(column_top)) 
                throw E(__LINE__);
            if (Bits::placeable(column_top) != m_array_bits_pile_next[column])
                throw E(__LINE__);
            bits_column_top.set_bit(column_top); 
        }
        if (bits_column_top != m_bits_pile_top)
            throw E(__LINE__);

        Bits bits_homecell_next;
        for (int suit=0; suit<N_SUIT; ++suit) {
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
        for (int id=0; id<DECK_SIZE; ++id) {
            if (m_array_location[id] >= 8)
                continue;
            Bits bits_deadlock = Bits::same_suit_small_rank(id);
            for (Card below=m_row_data.get_below(id); below.is_card(); below=m_row_data.get_below(below)) {
                if (bits_deadlock.is_set_bit(below)) {
                    bits_deadlocked.set_bit(id);
                    ncard_deadlocked += 1;
                    break;
                }
            }
        }
        if (bits_deadlocked != m_bits_deadlocked)
            throw E(__LINE__);
        if (ncard_deadlocked != m_ncard_deadlocked)
            throw E(__LINE__);

        for (int id=0; id<DECK_SIZE; ++id) {
            if (m_array_location[id] >= 8)
                continue;
            for (Card below=Card(id); below.is_card(); below=m_row_data.get_below(below)) {
                bool is_deadlocked = false;
                Bits bits_deadlock = Bits::same_suit_small_rank(below);
                for (Card below_below=m_row_data.get_below(below); below_below.is_card(); below_below=m_row_data.get_below(below_below)) {
                    if (bits_deadlock.is_set_bit(below_below)) {
                        is_deadlocked = true;
                        break;
                    }
                }
                if (! is_deadlocked)
                    array_ncard_not_deadlocked_below_and[id] += 1U;
            }
        }
        for (int id=0; id<DECK_SIZE; ++id)
            if (array_ncard_not_deadlocked_below_and[id] != m_array_ncard_not_deadlocked_below_and[id])
                throw E(__LINE__);
    }
    catch (const char *cstr) {
        cerr << cstr << endl;
        return false; 
    }
    return true;
}

void Position::initialize() noexcept {
    m_ncard_deadlocked = 0;
    fill_n(m_array_ncard_not_deadlocked_below_and, DECK_SIZE, 0U);
    m_bits_deadlocked.clear();
    for (int id=0; id<DECK_SIZE; ++id) {
        if (m_array_location[id] >= 8)
            continue;
        m_array_ncard_not_deadlocked_below_and[id] = 1U;
        Bits bits_deadlock = Bits::same_suit_small_rank(id);

        for (Card below=m_row_data.get_below(id); below.is_card(); below=m_row_data.get_below(below)) {
            if (bits_deadlock.is_set_bit(below)) {
                m_bits_deadlocked.set_bit(id);
                ++m_ncard_deadlocked;
                m_array_ncard_not_deadlocked_below_and[id] = 0U;
                break;
            }
        }
    }

    Card stack[52];
    int nstack;
    for (int column=0; column<TABLEAU_SIZE; ++column) {
        nstack = 0;
        for (Card below=m_array_pile_top[column]; below.is_card(); below=m_row_data.get_below(below))
            stack[nstack++] = below;
        for (int h=nstack-2; h>=0; --h)
            m_array_ncard_not_deadlocked_below_and[stack[h].get_id()] += m_array_ncard_not_deadlocked_below_and[stack[h + 1].get_id()];
    }
}

Position::Position(int seed) noexcept : Position_base::Position_base(seed) {
    initialize();
    assert(correct());
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
        if (m_array_pile_top[from] != card)
            return false;
        if (! m_bits_pile_top.is_set_bit(card))
            return false;
        if (m_bits_freecell.is_set_bit(card))
            return false;
        if (m_ncard_freecell >= DECK_SIZE)
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
            if (m_array_pile_top[from] != card)
                return false;
            if (! m_bits_pile_top.is_set_bit(card))
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
    constexpr unsigned char tbl[TABLEAU_SIZE] = {1U << 0, 1U << 1, 1U << 2, 1U << 3, 1U << 4, 1U << 5, 1U << 6, 1U << 7};
    
    for (int suit=0; suit<N_SUIT; ++suit) {
        if (m_array_homecell[suit].is_card())
            next = m_array_homecell[suit].next();
        else
            next = Card(suit, 0);
        if (! next.is_card())
            continue;
        a[n1++] = {m_array_ncard_not_deadlocked_below_and[m_array_pile_top[m_array_location[next.get_id()]].get_id()] - m_array_ncard_not_deadlocked_below_and[next.get_id()], next};
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
  if (ncard_rest() == 0) {
        m_is_solved = true;
        entry_parent.set_is_decided(true);
        assert(th == 0);
        return 0;
    }

    int th_child, new_th = MAX_H_COST;
    int ncard_rest_and_deadlocked = ncard_rest_for_h() + get_ncard_deadlocked();
    const Card* candidate_homecell_next_parent = entry_parent.get_candidate_homecell_next();
    for (int i=0; i<N_SUIT; ++i) {
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

int Position::move_to_homecell_next(const Card& next, Position::Action_for_h* history) noexcept {
    assert(m_bits_homecell_next.is_set_bit(next));
    int naction = 0;
    
    unsigned char column = m_array_location[next.get_id()];
    for (Card card=m_array_pile_top[column]; card!=next; card=m_array_pile_top[column]) {
        assert(card.is_card());
        history[naction] = Position::Action_for_h(card, column, 8);
        make2(history[naction++]); }
    history[naction] = Position::Action_for_h(next, column, next.suit() + 12);
    make2(history[naction++]);
    assert(correct_for_h());
    return naction;
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
        history[naction] = Action(card, m_array_location[card.get_id()], card.suit() + 12);
        make(history[naction++]);
        bits_from = m_bits_freecell | m_bits_pile_top;
        bits_possible = bits_from & m_bits_homecell_next;
    }
    return naction;
}

int Position::move_auto(Position::Action_for_h* history) noexcept {
    Card card;
    int naction = 0;
    Bits bits_from = m_bits_freecell | m_bits_pile_top;
    Bits bits_possible = bits_from & m_bits_homecell_next;
    Bits bits_deadlocked_top = m_bits_pile_top & m_bits_deadlocked;

    while (bits_possible || bits_deadlocked_top) {
        if (bits_possible) {
            card = bits_possible.pop();
            history[naction] = Position::Action_for_h(card, m_array_location[card.get_id()], card.suit() + 12);
            make2(history[naction++]); 
        }
        if (bits_deadlocked_top) {
            card = bits_deadlocked_top.pop();
            history[naction] = Position::Action_for_h(card, m_array_location[card.get_id()], 8);
            make2(history[naction++]); 
        }
        bits_from = m_bits_freecell | m_bits_pile_top;
        bits_possible = bits_from & m_bits_homecell_next;
        bits_deadlocked_top = m_bits_pile_top & m_bits_deadlocked;

    }
    assert(correct_for_h());
    return naction;
}

void Position::make(const Action& action) noexcept {
  Card card = action.get_card();
  if (action.get_from() <= 7) {
    m_array_ncard_not_deadlocked_below_and[card.get_id()] = 0U;
    if (m_bits_deadlocked.is_set_bit(card)) {
      m_bits_deadlocked.clear_bit(card);
      m_ncard_deadlocked -= 1; } }
  
  if (action.get_to() <= 7) {
    int pile = action.get_to();
    Card top = m_array_pile_top[pile];
    if (top.is_card())
      m_array_ncard_not_deadlocked_below_and[card.get_id()]
	= m_array_ncard_not_deadlocked_below_and[top.get_id()];
    
    if (m_array_bits_pile_card[pile] & Bits::same_suit_small_rank(card)) {
      m_bits_deadlocked.set_bit(card);
      m_ncard_deadlocked += 1; }
    else m_array_ncard_not_deadlocked_below_and[card.get_id()] += 1U; }
  
  Position_base::make(action);
  assert(correct()); }

void Position::make2(const Position::Action_for_h& action) noexcept {
    assert(action.correct());
    assert(correct_Action(action));
    Card card = action.get_card();
    int from = action.get_from();
    int to = action.get_to();
    
    if ((from <= 7) && (to >= 12)) {
        Card below = m_row_data.get_below(card);

        m_array_bits_pile_card[from].clear_bit(card);
        m_bits_homecell.set_bit(card);
        m_bits_homecell_next ^= Bits(card) | Bits::next(card);
        m_bits_pile_top.clear_bit(card);
        m_array_homecell[to - 12] = card;
        m_array_location[card.get_id()] = to;
        m_array_ncard_not_deadlocked_below_and[card.get_id()] = 0U;
        update_array_card_below(card, obtain_below_homecell(card));
        m_ncard_tableau -= 1;
        if (below.is_card()) {
            m_bits_pile_top.set_bit(below);
            m_array_bits_pile_next[from] = Bits::placeable(below);
            m_array_pile_top[from] = below;
        }
        else {
            m_array_pile_top[from] = Card();
            m_array_bits_pile_next[from].clear(); 
        }
    }

    else if (to >= 12) {
        assert(from >= 8);
        m_bits_freecell.clear_bit(card);
        m_bits_homecell.set_bit(card);
        m_bits_homecell_next ^= Bits(card) | Bits::next(card);
        update_array_card_below(card, obtain_below_homecell(card));
        m_array_homecell[to - 12] = card;
        m_array_location[card.get_id()] = to;
        m_ncard_freecell -= 1;
    }
    
    else {
        assert((from <= 7) && (to == 8));
        Card below = m_row_data.get_below(card);

        m_array_bits_pile_card[from].clear_bit(card);
        m_bits_freecell.set_bit(card);
        m_bits_pile_top.clear_bit(card);
        m_array_bits_pile_next[from] ^= Bits::placeable(card);
        m_array_ncard_not_deadlocked_below_and[card.get_id()] = 0U;
        update_array_card_below(card, Card::freecell());
        m_array_location[card.get_id()] = to;
        m_ncard_tableau -= 1;
        m_ncard_freecell += 1;
        if (below.is_card()) {
            m_bits_pile_top.set_bit(below);
            m_array_bits_pile_next[from] ^= Bits::placeable(below);
            m_array_pile_top[from] = below;
        }
        else {
            m_array_pile_top[from] = Card(); 
        }

        if (m_bits_deadlocked.is_set_bit(card)) {
            m_bits_deadlocked.clear_bit(card);
            m_ncard_deadlocked -= 1;
        }
    } 
    assert(correct_for_h());
}

void Position::unmake(const Action& action) noexcept {
  assert(action.ok());

  Card card = action.get_card();
  if (action.get_to() <= 7) {
    m_array_ncard_not_deadlocked_below_and[card.get_id()] = 0U;
    if (m_bits_deadlocked.is_set_bit(card)) {
      m_bits_deadlocked.clear_bit(card);
      m_ncard_deadlocked -= 1; } }
  
  if (action.get_from() <= 7) {
    int column = action.get_from();
    Card below = m_array_pile_top[column];
    
    if (below.is_card())
      m_array_ncard_not_deadlocked_below_and[card.get_id()]
	= m_array_ncard_not_deadlocked_below_and[below.get_id()];
    
    if (m_array_bits_pile_card[column] & Bits::same_suit_small_rank(card)) {
      m_bits_deadlocked.set_bit(card);
      m_ncard_deadlocked += 1; }
    else m_array_ncard_not_deadlocked_below_and[card.get_id()] += 1U; }

  Position_base::unmake(action);
  assert(correct());
  assert(is_legal(action)); }

void Position::unmake2(const Position::Action_for_h& action) noexcept {
    assert(action.correct());
    Card card = action.get_card();
    int from = action.get_from();
    int to = action.get_to();
    assert(to == m_array_location[card.get_id()]);

    if ((from <= 7) && (to >= 12)) {
        assert(m_bits_homecell.is_set_bit(card));
        assert(card == m_array_homecell[to - 12]);
        Card below = m_array_pile_top[from];

        m_array_bits_pile_card[from].set_bit(card);
        m_array_bits_pile_next[from] = Bits::placeable(card);
        m_bits_homecell.clear_bit(card);
        m_bits_homecell_next ^= Bits(card) | Bits::next(card);
        m_bits_pile_top.set_bit(card);
        m_array_homecell[to - 12] = card.prev();;
        m_array_location[card.get_id()] = from;
        m_array_pile_top[from] = card;
        m_ncard_tableau += 1;
        if (below.is_card()) {
            m_bits_pile_top.clear_bit(below);
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
        Card below = m_array_pile_top[from];
      
        m_array_bits_pile_card[from].set_bit(card);
        m_bits_freecell.clear_bit(card);
        m_bits_pile_top.set_bit(card);
        m_array_bits_pile_next[from] = Bits::placeable(card);
        m_array_pile_top[from] = card;
        m_array_location[card.get_id()] = from;
        m_ncard_tableau += 1;
        m_ncard_freecell -= 1;
        if (below.is_card()) {
            update_array_card_below(card, below);
            m_bits_pile_top.clear_bit(below);
            m_array_ncard_not_deadlocked_below_and[card.get_id()] = m_array_ncard_not_deadlocked_below_and[below.get_id()];
        }
        else {
            update_array_card_below(card, Card::field()); 
        }

        if (m_array_bits_pile_card[from] & Bits::same_suit_small_rank(card)) {
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
