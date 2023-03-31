#include <climits>
#include "position.hpp"

bool Position::Entry_TT_52f::ok() const noexcept {
  try {
#ifdef TEST_ZKEY
    if (! m_row_data.ok()) throw E(__LINE__);
#endif
    if (m_lower_bound < 0 || MAX_F_COST_52F < m_lower_bound) throw E(__LINE__);
    
    for (int i=0; i<N_SUIT; ++i) {
      if (m_candidate_homecell_next[i] == Card()) break;
      if (! m_candidate_homecell_next[i].is_card()) throw E(__LINE__); } }
  catch (const char *cstr) {
    cerr << cstr << endl;
    return false; }
  
  return true; }

void Position::one_suit_analysis(int &ncard_deadlocked, Bits &bits_deadlocked,
				 unsigned char *array_nbelow_not_deadlocked) const noexcept {
  bits_deadlocked.clear();
  ncard_deadlocked = 0;
    
  for (int id=0; id<DECK_SIZE; ++id) {
    if (m_array_location[id] >= 8) {
      array_nbelow_not_deadlocked[id] = 0;
      continue; }
      
    array_nbelow_not_deadlocked[id] = 1U;
    for (Card below=m_row_data.get_below(id); below.is_card();
	 below=m_row_data.get_below(below)) {
      if (! Bits::same_suit_small_rank(id).is_set_bit(below)) continue;
      bits_deadlocked.set_bit(id);
      ncard_deadlocked += 1;
      array_nbelow_not_deadlocked[id] = 0;
      break; } }
    
  for (int pile=0; pile<TABLEAU_SIZE; ++pile) {
    Card stack[DECK_SIZE];
    int nstack = 0;
    for (Card below=m_array_pile_top[pile]; below.is_card();
	 below=m_row_data.get_below(below)) stack[nstack++] = below;
    for (int h=nstack-2; h>=0; --h)
      array_nbelow_not_deadlocked[stack[h].get_id()]
	+= array_nbelow_not_deadlocked[stack[h + 1].get_id()]; } }

int Position::calc_nabove_not_deadlocked(const Card& card) const noexcept {
  assert(card.is_card());
  int pile = m_array_location[card.get_id()];
  assert(0 <= pile && pile < TABLEAU_SIZE);
  return m_array_nbelow_not_deadlocked[m_array_pile_top[pile].get_id()]
    - m_array_nbelow_not_deadlocked[card.get_id()]; }

bool Position::ok() const noexcept {
  try {
    int ncard_deadlocked;
    Bits bits_deadlocked;
    unsigned char array_nbelow_not_deadlocked[DECK_SIZE];
    one_suit_analysis(ncard_deadlocked, bits_deadlocked, array_nbelow_not_deadlocked);
    if (bits_deadlocked != m_bits_deadlocked) throw E(__LINE__);
    if (ncard_deadlocked != m_ncard_deadlocked) throw E(__LINE__);

    for (int id=0; id<DECK_SIZE; ++id)
      if (array_nbelow_not_deadlocked[id]
	  != m_array_nbelow_not_deadlocked[id]) throw E(__LINE__); }
  catch (const char *cstr) {
    cerr << cstr << endl;
    return false; }
  return true; }

Position::Position(int seed) noexcept : Position_base::Position_base(seed) {
  one_suit_analysis(m_ncard_deadlocked, m_bits_deadlocked, m_array_nbelow_not_deadlocked);
  assert(ok()); }

int Position::calc_h_cost_52f(Card* candidate_homecell_next) noexcept {
  assert(candidate_homecell_next);
  if (ncard_rest() == 0) return 0;

  Bits bits_homecell_next = m_bits_homecell_next;
  pair<int, Card> a[5], tmp;
  int n = 0;
  a[0].first = INT_MAX;

  for (Card next = bits_homecell_next.pop(); next; next = bits_homecell_next.pop()) {
    int v = calc_nabove_not_deadlocked(next);

    // sort by v and delete duplicates in terms of pile.
    int pile = m_array_location[next.get_id()];
    assert(m_array_pile_top[pile] != next);
    int i;
    for (i = 0; a[i].first <= v; ++i)
      if (m_array_location[a[i].second.get_id()] == pile) break;
    if (a[i].first <= v) continue;
    
    tmp = {v, next};
    while (true) {
      a[i++].swap(tmp);
      if (tmp.first == INT_MAX) { a[i].first = INT_MAX; n += 1; break; }
      if (m_array_location[tmp.second.get_id()] == pile) break; } }
  
  for (int i=0; i<n; ++i) candidate_homecell_next[i] = a[i].second;
  if (n < HOMECELL_SIZE) candidate_homecell_next[n] = Card();
  
  return m_ncard_freecell + m_ncard_tableau + m_ncard_deadlocked + a[0].first; }

int Position::solve_52f(int bound_max) noexcept {
  assert(0 <= bound_max);
  Action path[MAX_F_COST_52F];
  int naction = move_auto_52f(path);
    
  int bound_52f;
  auto it = m_tt.find(m_zobrist_key);
  if (it != m_tt.end()) {
    assert(it->second.ok());
    it->second.test_zobrist_key(get_row_data());
    bound_52f = it->second.get_lower_bound(); }
  else {
    Card candidate_homecell_next[HOMECELL_SIZE];
    bound_52f = calc_h_cost_52f(candidate_homecell_next);
    it = m_tt.emplace(piecewise_construct,
		      forward_as_tuple(m_zobrist_key),
		      forward_as_tuple(bound_52f, get_row_data(),
				       candidate_homecell_next)).first; }
  int th = bound_52f;
  if (! it->second.is_solved()) {
    m_is_solved = false;
    while (naction + th < bound_max) {
      th = dfstt1(th, path + naction, it->second);
      if (m_is_solved) break; } }
  
  unmake_n(path + naction, naction);
  
  return naction + th; }

int Position::dfstt1(int th, Action* path, Entry_TT_52f& entry_parent) noexcept {
  if (ncard_rest() == 0) {
    m_is_solved = true;
    entry_parent.set_solved();
    assert(th == 0);
    return 0; }
  
  int new_bound = UCHAR_MAX;
  int h1_cost = calc_h1_cost();
  const Card* candidate_homecell_next_parent = entry_parent.get_candidate_homecell_next();
  assert(candidate_homecell_next_parent[0].is_card());
  for (int i=0; i<N_SUIT; ++i) {
    Card next = candidate_homecell_next_parent[i];
    if (! next) break;

    int th_estimate = h1_cost + calc_nabove_not_deadlocked(next);
    if (th_estimate >= new_bound) break;
    
    /* break more aggressively
      if (th_estimate > th) {
        new_bound = min(new_bound, th_estimate);
        break; } */

    int bound_child;
    int cost = move_to_homecell(next, path);
    cost += move_auto_52f(path + cost);

    auto it = m_tt.find(m_zobrist_key);
    if (it != m_tt.end()) {
      assert(it->second.ok());
      it->second.test_zobrist_key(get_row_data());
      bound_child = it->second.get_lower_bound(); }
    else {
      Card candidate_homecell_next_child[HOMECELL_SIZE];
      bound_child = calc_h_cost_52f(candidate_homecell_next_child);
      it = m_tt.emplace(piecewise_construct,
			forward_as_tuple(m_zobrist_key),
			forward_as_tuple(bound_child, get_row_data(),
					 candidate_homecell_next_child)).first; }

    assert(cost + bound_child >= th);
    // assert( 'length of an optimal solution from this parent' >= th );
    if (cost + bound_child <= th && it->second.is_solved()) {
      new_bound = th;
      m_is_solved = true; }
    else if (cost + bound_child <= th)
      new_bound = min(new_bound, cost + dfstt1(th - cost, path + cost, it->second));
    else
      new_bound = min(new_bound, cost + bound_child);
    
    unmake_n(path + cost, cost);
    
    if (m_is_solved) {
      assert(th == new_bound && entry_parent.get_lower_bound() == new_bound);
      entry_parent.set_solved();
      break; } }
  
  entry_parent.update_lower_bound(new_bound);
  return new_bound; }

int Position::move_to_homecell(const Card& next, Action* history) noexcept {
  assert(m_bits_homecell_next.is_set_bit(next));
  assert(m_array_location[next.get_id()] < TABLEAU_SIZE);
  assert(history);
  int naction = 0;
    
  unsigned char pile = m_array_location[next.get_id()];
  for (Card card=m_array_pile_top[pile]; card!=next; card=m_array_pile_top[pile]) {
    assert(card.is_card());
    history[naction] = Action(card, pile, 8);
    make(history[naction++]); }
  history[naction] = Action(next, pile, next.suit() + 12);
  make(history[naction++]);
  return naction; }

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
  assert(history);
  Card card;
  int naction = 0;
  Bits bits_from = m_bits_freecell | m_bits_pile_top;
  Bits bits_possible = bits_from & m_bits_homecell_next;
  Bits bits_deadlocked_top = m_bits_pile_top & m_bits_deadlocked;

  while (bits_possible || bits_deadlocked_top) {
    if (bits_possible) {
      card = bits_possible.pop();
      history[naction] = Action(card, m_array_location[card.get_id()], card.suit() + 12);
      make(history[naction++]); }
    if (bits_deadlocked_top) {
      card = bits_deadlocked_top.pop();
      history[naction] = Action(card, m_array_location[card.get_id()], 8);
      make(history[naction++]);
    }
    bits_from = m_bits_freecell | m_bits_pile_top;
    bits_possible = bits_from & m_bits_homecell_next;
    bits_deadlocked_top = m_bits_pile_top & m_bits_deadlocked; }
  return naction; }

void Position::make(const Action& action) noexcept {
  Card card = action.get_card();
  if (action.get_from() <= 7) {
    m_array_nbelow_not_deadlocked[card.get_id()] = 0U;
    if (m_bits_deadlocked.is_set_bit(card)) {
      m_bits_deadlocked.clear_bit(card);
      m_ncard_deadlocked -= 1; } }
  
  if (action.get_to() <= 7) {
    int pile = action.get_to();
    Card top = m_array_pile_top[pile];
    if (top.is_card())
      m_array_nbelow_not_deadlocked[card.get_id()]
	= m_array_nbelow_not_deadlocked[top.get_id()];
    
    if (m_array_bits_pile_card[pile] & Bits::same_suit_small_rank(card)) {
      m_bits_deadlocked.set_bit(card);
      m_ncard_deadlocked += 1; }
    else m_array_nbelow_not_deadlocked[card.get_id()] += 1U; }
  
  Position_base::make(action);
  assert(ok()); }

void Position::unmake(const Action& action) noexcept {
  assert(action.ok());

  Card card = action.get_card();
  if (action.get_to() <= 7) {
    m_array_nbelow_not_deadlocked[card.get_id()] = 0U;
    if (m_bits_deadlocked.is_set_bit(card)) {
      m_bits_deadlocked.clear_bit(card);
      m_ncard_deadlocked -= 1; } }
  
  if (action.get_from() <= 7) {
    int column = action.get_from();
    Card below = m_array_pile_top[column];
    
    if (below.is_card())
      m_array_nbelow_not_deadlocked[card.get_id()]
	= m_array_nbelow_not_deadlocked[below.get_id()];
    
    if (m_array_bits_pile_card[column] & Bits::same_suit_small_rank(card)) {
      m_bits_deadlocked.set_bit(card);
      m_ncard_deadlocked += 1; }
    else m_array_nbelow_not_deadlocked[card.get_id()] += 1U; }

  Position_base::unmake(action);
  assert(ok());
  assert(is_legal(action)); }
