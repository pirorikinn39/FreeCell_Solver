#include <climits>
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

bool Position::correct_Action(const Action& action) const noexcept {
    if (! action.ok()) 
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

int Position::calc_h_cost_52f(Card* candidate_homecell_next) noexcept {
  assert(candidate_homecell_next);
  if (ncard_rest() == 0) return 0; // delete

  assert(m_bits_homecell & Bits::not_kings());
  Bits bits_homecell_next = m_bits_homecell_next;
  pair<int, Card> a[5], tmp;
  int n = 0;
  a[0].first = INT_MAX;
  for (Card next = bits_homecell_next.pop(); next; next = bits_homecell_next.pop()) {
    // if (next.rank() == 12) continue; // evaluate
    
    int pile = m_array_location[next.get_id()];
    assert(m_array_pile_top[pile] != next);
    int v = m_array_ncard_not_deadlocked_below_and[m_array_pile_top[pile].get_id()]
      - m_array_ncard_not_deadlocked_below_and[next.get_id()];
    v = v*16 + next.rank(); // evaluate
    
    // sort by v and delete duplicates in terms of pile.
    int i;
    for (i = 0; a[i].first <= v; ++i)
      if (m_array_location[a[i].second.get_id()] == m_array_location[next.get_id()]) break;
    if (a[i].first <= v) continue;
    
    tmp = {v, next};
    while (true) {
      a[i++].swap(tmp);
      if (tmp.first == INT_MAX) { a[i].first = INT_MAX; n += 1; break; }
      if (m_array_location[tmp.second.get_id()] == m_array_location[next.get_id()]) break; } }
  
  for (int i=0; i<n; ++i) candidate_homecell_next[i] = a[i].second;
  if (n < HOMECELL_SIZE) candidate_homecell_next[n] = Card();
  
  return m_ncard_freecell + m_ncard_tableau + m_ncard_deadlocked + a[0].first / 16; }

int Position::calc_h_cost() noexcept {
    Action path[MAX_H_COST];
    int naction = move_auto_52f(path);
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
        th = calc_h_cost_52f(candidate_homecell_next);
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
    return h_cost;
}

int Position::dfstt1(int th, Action* path, Position::Entry_tt& entry_parent) noexcept {
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
        cost += move_auto_52f(path + cost);

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
            th_child = calc_h_cost_52f(candidate_homecell_next_child);
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

int Position::move_to_homecell_next(const Card& next, Action* history) noexcept {
    assert(m_bits_homecell_next.is_set_bit(next));
    int naction = 0;
    
    unsigned char column = m_array_location[next.get_id()];
    for (Card card=m_array_pile_top[column]; card!=next; card=m_array_pile_top[column]) {
        assert(card.is_card());
        history[naction] = Action(card, column, 8);
        make(history[naction++]); }
    history[naction] = Action(next, column, next.suit() + 12);
    make(history[naction++]);
    return naction;
}

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

int Position::move_auto_52f(Action* history) noexcept {
    Card card;
    int naction = 0;
    Bits bits_from = m_bits_freecell | m_bits_pile_top;
    Bits bits_possible = bits_from & m_bits_homecell_next;
    Bits bits_deadlocked_top = m_bits_pile_top & m_bits_deadlocked;

    while (bits_possible || bits_deadlocked_top) {
        if (bits_possible) {
            card = bits_possible.pop();
            history[naction] = Action(card, m_array_location[card.get_id()], card.suit() + 12);
            make(history[naction++]);
        }
        if (bits_deadlocked_top) {
            card = bits_deadlocked_top.pop();
            history[naction] = Action(card, m_array_location[card.get_id()], 8);
            make(history[naction++]);
        }
        bits_from = m_bits_freecell | m_bits_pile_top;
        bits_possible = bits_from & m_bits_homecell_next;
        bits_deadlocked_top = m_bits_pile_top & m_bits_deadlocked;

    }
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
