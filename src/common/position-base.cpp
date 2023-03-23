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


void Position_base::initialize(const Card (&field)[TABLEAU_SIZE][64],
			       const Card (&array_homecell)[HOMECELL_SIZE],
			       const Card (&array_freecell)[FREECELL_SIZE]) noexcept {
  m_zobrist_key = 0ULL;
  m_ncard_tableau = m_ncard_freecell = 0;
  m_bits_pile_top.clear();
  fill_n(m_array_location, DECK_SIZE, BAD_LOCATION);
  for (int pile=0; pile<TABLEAU_SIZE; ++pile) {
    m_array_bits_pile_card[pile].clear();
    m_array_bits_pile_next[pile].clear();
    if (! field[pile][0].is_card()) continue;
    
    int h = 0;
    m_array_bits_pile_card[pile].set_bit(field[pile][h]);
    m_row_data.set_below(field[pile][h], Card::field());
    m_array_location[field[pile][h].get_id()] = pile;
    m_zobrist_key ^= table.get(field[pile][h], Card::field());
    for (h=1; field[pile][h]; ++h) {
      m_array_bits_pile_card[pile].set_bit(field[pile][h]);
      m_row_data.set_below(field[pile][h], field[pile][h-1]);
      m_array_location[field[pile][h].get_id()] = pile; 
      m_zobrist_key ^= table.get(field[pile][h], field[pile][h-1]); }
    m_ncard_tableau += h;
    Card top = field[pile][h-1];
    m_array_pile_top[pile] = top;
    m_bits_pile_top.set_bit(top);
    m_array_bits_pile_next[pile] = Bits::placeable(top); }

    m_bits_freecell.clear();
    for (int freecell=0; freecell<FREECELL_SIZE; ++freecell) {
      Card card = array_freecell[freecell];
      m_array_freecell[freecell] = card;
      if (! card)  continue;
      assert(card.is_card());
      m_ncard_freecell += 1;
      m_row_data.set_below(card, Card::freecell());
      m_array_location[card.get_id()] = freecell + 8;
      m_zobrist_key ^= table.get(card, Card::freecell());
      m_bits_freecell.set_bit(card); }

    m_bits_homecell.clear();
    for (int suit=0; suit<N_SUIT; ++suit) {
        Card card = array_homecell[suit];
        m_array_homecell[suit] = card;
        if (! card) continue;
	
        assert(card.is_card() && (card.suit() == suit));
        m_row_data.set_below(Card(suit, 0), Card::homecell());
        m_array_location[Card(suit, 0).get_id()] = suit + 12;
        m_zobrist_key ^= table.get(Card(suit, 0), Card::homecell());
        m_bits_homecell.set_bit(Card(suit, 0).get_id());
        for (int rank=1; rank<=card.rank(); ++rank) {
	  m_row_data.set_below(Card(suit, rank), Card(suit, rank - 1));
	  m_array_location[Card(suit, rank).get_id()] = suit + 12;
	  m_zobrist_key ^= table.get(Card(suit, rank), Card(suit, rank - 1) );
	  m_bits_homecell.set_bit(Card(suit, rank).get_id()); } }

    m_bits_homecell_next.clear();
    for (int suit=0; suit<N_SUIT; ++suit) {
      Bits bits_next(Card(suit, 0).get_id());
      if (m_array_homecell[suit].is_card()) {
	assert(m_array_homecell[suit].suit() == suit);
	bits_next = Bits::next(m_array_homecell[suit]); }
      m_bits_homecell_next |= bits_next; } }

Position_base::Position_base(int seed) noexcept {
    int rest = DECK_SIZE;
    Card deck[DECK_SIZE];
    Card field[TABLEAU_SIZE][64], array_homecell[HOMECELL_SIZE], array_freecell[FREECELL_SIZE];

    for (int rank=0; rank<N_RANK; ++rank) {
      deck[rank * 4 + 0] = Card(Card::club,    rank);
      deck[rank * 4 + 1] = Card(Card::diamond, rank);
      deck[rank * 4 + 2] = Card(Card::heart,   rank);
      deck[rank * 4 + 3] = Card(Card::spade,   rank); }
    
    for (int i=0; i<DECK_SIZE; ++i) {
      seed = seed * 214013 + 2531011;
      int index = ((seed >> 16) & 32767) % rest;
      field[i % TABLEAU_SIZE][i / TABLEAU_SIZE] = deck[index];
      deck[index] = deck[--rest]; }
    
    initialize(field, array_homecell, array_freecell); }

bool Position_base::is_legal(const Action& action) const noexcept {
    if (! action.ok()) return false;
    Card card;
    int from = action.get_from();
    if (from <= 7) card = m_array_pile_top[from];
    else           card = m_array_freecell[from - TABLEAU_SIZE];
    if (! card) return false;
    
    int to = action.get_to();
    if (to <= 7) {
      if (! m_array_pile_top[to]) return true;
      return m_array_bits_pile_next[to].is_set_bit(card); }
    if (to <= 11) return ! m_array_freecell[to - TABLEAU_SIZE];

    if (to != card.suit() + 12) return false;
    return m_bits_homecell_next.is_set_bit(card); }

int Position_base::gen_actions(Action (&actions)[MAX_ACTION_SIZE]) const noexcept {
    int naction = 0;
    Bits bits_from = m_bits_freecell | m_bits_pile_top;
    Bits bits_possible;

    for (int pile=0; pile<TABLEAU_SIZE; ++pile) {
      bits_possible = bits_from & m_array_bits_pile_next[pile];
      for (Card card=bits_possible.pop(); card; card=bits_possible.pop())
	actions[naction++] = Action(m_array_location[card.get_id()], pile); }
      
    bits_possible = bits_from & m_bits_homecell_next;
    for (Card card=bits_possible.pop(); card; card=bits_possible.pop())
      actions[naction++] = Action(m_array_location[card.get_id()], card.suit() + 12 );

    int freecell_empty = find_freecell_empty();
    if (freecell_empty < FREECELL_SIZE)
      for (int pile=0; pile<TABLEAU_SIZE; ++pile)
	if (m_array_pile_top[pile])
	  actions[naction++] = Action(pile, freecell_empty + TABLEAU_SIZE);

    int pile_empty = find_pile_empty();
    if (pile_empty < TABLEAU_SIZE) {
      for (int freecell=0; freecell<FREECELL_SIZE; ++freecell)
	if (m_array_freecell[freecell])
	  actions[naction++] = Action(freecell + TABLEAU_SIZE, pile_empty);
      for (int pile=0; pile<TABLEAU_SIZE; ++pile) {
	Card card = m_array_pile_top[pile];

	if (card.is_card() && m_row_data.get_below(card).is_card())
	  actions[naction++] = Action(pile, pile_empty); } }
    
    return naction; }

void Position_base::make(const Action& action) noexcept {
  assert(is_legal(action));

  Card card;
  if (action.get_from() <= 7) {
    int pile = action.get_from();
    card = m_array_pile_top[pile];
    Card below = m_row_data.get_below(card);
    m_array_bits_pile_card[pile].clear_bit(card);
    m_bits_pile_top.clear_bit(card);
    m_ncard_tableau -= 1;
    m_array_pile_top[pile] = below;
    if (below.is_card()) {
      m_bits_pile_top.set_bit(below);
      m_array_bits_pile_next[pile] = Bits::placeable(below);
      m_array_pile_top[pile] = below; }
    else {
      m_array_pile_top[pile] = Card();
      m_array_bits_pile_next[pile].clear(); } }
  else {
    int freecell = action.get_from() - TABLEAU_SIZE;
    card = m_array_freecell[freecell];
    
    m_bits_freecell.clear_bit(card.get_id());
    m_array_freecell[freecell] = Card();
    m_ncard_freecell -= 1; }


  if (action.get_to() <= 7) {
    int pile = action.get_to();
    Card top = m_array_pile_top[pile];
    
    m_array_bits_pile_card[pile].set_bit(card);
    m_bits_pile_top.set_bit(card);
    m_array_bits_pile_next[pile] ^= Bits::placeable(card);
    m_array_pile_top[pile] = card;
    m_array_location[card.get_id()] = pile;
    m_ncard_tableau += 1;
    if (top.is_card()) {
      m_bits_pile_top.clear_bit(top);
      m_array_bits_pile_next[pile] ^= Bits::placeable(top);
      update_array_card_below(card, top); }
    else update_array_card_below(card, Card::field()); }
  
  else if (action.get_to() <= 11) {
    int freecell = action.get_to() - TABLEAU_SIZE;
    
    m_bits_freecell.set_bit(card);
    update_array_card_below(card, Card::freecell() );
    m_array_freecell[freecell] = card;
    m_ncard_freecell += 1;
    m_array_location[card.get_id()] = action.get_to(); }

  else {
    m_bits_homecell.set_bit(card);
    m_bits_homecell_next ^= Bits(card) | Bits::next(card);
    m_array_location[card.get_id()] = card.suit() + 12;
    m_array_homecell[card.suit()] = card;
    update_array_card_below(card, obtain_below_homecell(card)); }

  assert(ok()); }

bool Position_base::ok() const noexcept {
  try { 
    Bits bits_pile_top, bits_homecell, bits_freecell;
    Bits array_bits_pile_card[TABLEAU_SIZE];
    unsigned char ncard[DECK_SIZE];
    Card array_homecell_above[HOMECELL_SIZE];
    Card array_freecell_above[FREECELL_SIZE];
    Card array_pile_above[TABLEAU_SIZE];
    Card array_card_above[DECK_SIZE];
    Card array_homecell[HOMECELL_SIZE];
    int ncard_homecell = 0;
    int ncard_freecell = 0;
    int npile = 0;

    fill_n(ncard, DECK_SIZE, 0);
    uint64_t zobrist_key = 0;
    for (int id=0; id<DECK_SIZE; ++id)
      zobrist_key ^= table.get(Card(id), m_row_data.get_below(id));
    if (zobrist_key != m_zobrist_key) throw E(__LINE__);
    
    for (int above=0; above<DECK_SIZE; ++above) {
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
      if (! m_array_bits_pile_card[pile1]) continue;
      npile_origin += 1;
      int pile2;
      for (pile2=0; pile2<TABLEAU_SIZE; ++pile2)
	if (array_bits_pile_card[pile2] == m_array_bits_pile_card[pile1]) break;
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

    for (int id=0; id<DECK_SIZE; ++id) {
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
	for (Card below=m_array_pile_top[from]; below.is_card();
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
    
    for (int suit=0; suit<N_SUIT; ++suit)
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
    if (bits_pile_top != m_bits_pile_top) throw E(__LINE__);
    if (ncard_tableau != m_ncard_tableau) throw E(__LINE__);

    for (int id=0; id<DECK_SIZE; ++id)
      if (ncard[id] != 1) throw E(__LINE__);

    bits_pile_top.clear();
    for (int pile=0; pile<TABLEAU_SIZE; ++pile) {
      Card pile_top = m_array_pile_top[pile];
      if (! pile_top) {
	if (m_array_bits_pile_next[pile]) throw E(__LINE__);
	continue; }
      if (! pile_top.is_card()) throw E(__LINE__);
      if (bits_pile_top.is_set_bit(pile_top)) throw E(__LINE__);
      if (Bits::placeable(pile_top) != m_array_bits_pile_next[pile]) throw E(__LINE__);
      bits_pile_top.set_bit(pile_top); }
    if (bits_pile_top != m_bits_pile_top) throw E(__LINE__);

    Bits bits_homecell_next;
    for (int suit=0; suit<N_SUIT; ++suit) {
      Card card = array_homecell[suit];
      if (! card.is_card()) bits_homecell_next.set_bit(Card(suit, 0));
      else if (card.rank() < 12) bits_homecell_next.set_bit(card.next()); }
    if (bits_homecell_next != m_bits_homecell_next) throw E(__LINE__); }
  catch (const char *cstr) {
    cerr << cstr << endl;
    return false; }
  return true; }

