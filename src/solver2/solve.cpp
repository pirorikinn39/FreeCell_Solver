#include <exception>
#include <iostream>
#include <utility>
#include <climits>
#include "../common/utility.hpp"
#include "position.hpp"
#include "solve.hpp"

class Entry_TT_52f {
#ifdef TEST_ZKEY
  Position_row m_row_data;
#endif
  unsigned char m_lower_bound;
  bool m_is_solved;
  Card m_candidate_homecell_next[HOMECELL_SIZE];
  
public:
  Entry_TT_52f(int lower_bound, const Position_row& row_data,
	       const Card* candidate_homecell_next) noexcept : 
#ifdef TEST_ZKEY
    m_row_data(row_data), m_lower_bound(lower_bound), m_is_solved(false)
#else
    m_lower_bound(lower_bound), m_is_solved(false)
#endif
  {
    copy_n(candidate_homecell_next, N_SUIT, m_candidate_homecell_next);
    assert(ok()); }
  void update_lower_bound(int lower_bound) noexcept {
    assert(m_lower_bound <= lower_bound && lower_bound <= MAX_F_COST_52F);
    m_lower_bound = lower_bound; }
  void set_solved() noexcept {
    assert(! m_is_solved);
    m_is_solved = true; }
  
  int get_lower_bound() const noexcept { return m_lower_bound; }
  bool is_solved() const noexcept { return m_is_solved; }
  const Card* get_candidate_homecell_next() const noexcept {
    return m_candidate_homecell_next; }
    void test_zobrist_key(const Position_row& row_data) const noexcept {
#ifdef TEST_ZKEY
      if (m_row_data == row_data) return;
      cerr << "Zobrist Key Conflict" << endl;
      terminate();
#endif
    }
  bool ok() const noexcept;
};

class Entry_TT_4f {
#ifdef TEST_ZKEY
  Position_row m_row_data;
#endif
  unsigned char m_lower_bound;
  bool m_is_visiting;
public:
  Entry_TT_4f(int lower_bound, const Position& position) noexcept :
#ifdef TEST_ZKEY
    m_row_data(position.get_row_data()), m_lower_bound(lower_bound), m_is_visiting(false)
#else
    m_lower_bound(lower_bound), m_is_visiting(false)
#endif
  { assert(0 <= lower_bound && lower_bound <= UCHAR_MAX); }
  void update_lower_bound(int lower_bound) noexcept {
    assert(m_lower_bound <= lower_bound && lower_bound <= UCHAR_MAX );
    m_lower_bound = lower_bound; }
  void set_visiting() noexcept { m_is_visiting = true; }
  void clear_visiting() noexcept { m_is_visiting = false; }

  int get_lower_bound() const noexcept { return m_lower_bound; }
  int is_visiting() const noexcept { return m_is_visiting; }
  void test_zobrist_key(const Position& position) const noexcept {
#ifdef TEST_ZKEY
    if (m_row_data == position.get_row_data()) return;
    cerr << "Zobrist Key Conflict" << endl;
    terminate(); 
#endif
  }
  bool ok() const noexcept {
    if (UCHAR_MAX < m_lower_bound) return false;
    
#ifdef TEST_ZKEY
    return m_row_data.ok();
#else
    return true;
#endif
  }
};

class Solve {
  Position m_position;
  bool m_is_solved;
  Action m_solution[UCHAR_MAX];
  unordered_map<uint64_t, Entry_TT_4f>  m_tt_4f;
  unordered_map<uint64_t, Entry_TT_52f> m_tt_52f;
  
  int dfstt1(int, int, Action*, Entry_TT_4f&) noexcept;

public:
  Solve(int) noexcept;
};

int Solve::dfstt1(int th, int g_cost, Action* path, Entry_TT_4f& entry_parent) noexcept {
  if (m_position.ncard_rest() == 0) {
    assert(th == 0);
    m_is_solved = true;
    copy_n(path, g_cost, m_solution);
    return 0; }

  entry_parent.set_visiting();

  Action actions[MAX_ACTION_SIZE];
  int new_bound = UCHAR_MAX;
  int naction = m_position.gen_actions(actions);
  assert(naction <= MAX_ACTION_SIZE);
  for (int i=0; i<naction; ++i) {
    path[g_cost] = actions[i];
    m_position.make(actions[i]);
    
    int nauto = m_position.move_auto(path + g_cost + 1);
    int cost = 1 + nauto;
    int new_g_cost = g_cost + cost;
    assert(new_g_cost <= UCHAR_MAX);

    int bound_child;
    auto it = m_tt_4f.find(m_position.get_zobrist_key());
    if (it != m_tt_4f.end()) {
      it->second.test_zobrist_key(m_position);
      bound_child = it->second.get_lower_bound(); }
    else {
      bound_child = m_position.solve_52f(new_bound);
      it = m_tt_4f.emplace(piecewise_construct,
			   forward_as_tuple(m_position.get_zobrist_key()),
			   forward_as_tuple(bound_child, m_position)).first; }
    auto& entry_child = it->second;
    assert(entry_child.ok());
    assert(m_position.ncard_rest() <= bound_child);
    assert(cost + bound_child >= th);
    if (entry_child.is_visiting());
    else if (cost + bound_child <= th)
      new_bound = min(new_bound, cost + dfstt1(th - cost, new_g_cost, path, entry_child));
    else
      new_bound = min(new_bound, cost + bound_child);
    
    m_position.unmake_n(path + new_g_cost, cost);
    if (m_is_solved) {
      assert(th == new_bound);
      break; } }

  assert(new_bound >= entry_parent.get_lower_bound());
  assert(entry_parent.is_visiting());
  entry_parent.update_lower_bound(new_bound);
  entry_parent.clear_visiting();
  return new_bound; }

Solve::Solve(int game_id) noexcept : m_position(game_id) {
  Action path[UCHAR_MAX];
  int g_cost = m_position.move_auto(path); 
  int esti = m_position.solve_52f(UCHAR_MAX);
  Entry_TT_4f& entry = m_tt_4f.emplace(piecewise_construct,
				       forward_as_tuple(m_position.get_zobrist_key()),
				       forward_as_tuple(esti, m_position)).first->second;
  assert(entry.ok());
  
  int th = esti;
  m_is_solved = false;
  while (true) {
    th = dfstt1(th, g_cost, path, entry);
    if (th == UCHAR_MAX || m_is_solved) break; }
  
  if (! m_is_solved) { cout << "Solutions are too long, or there is no solution." << endl; }
  else {
    int min_f_cost = g_cost + th;
    cout << ", " << min_f_cost << ", ";
    cout << gen_solution(game_id, m_solution, min_f_cost);
    cout << ", " << m_tt_4f.size() << ", " << m_position.m_tt_size(); } }

void solve(int game_id) noexcept { Solve solve(game_id); }
