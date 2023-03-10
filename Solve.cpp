#include "Solve.h"

Solve::Entry_tt::Entry_tt(int h_cost, const Position& position) noexcept : m_h_cost(h_cost), m_is_contained_route(false) {
    assert((h_cost >= 0) && (h_cost <= UCHAR_MAX));
#ifdef TEST_ZKEY
    m_union_array_card_below.set_array_card_belows(position);
#endif
}

void Solve::Entry_tt::set_h_cost(int h_cost) noexcept {
    assert((h_cost >= 0) && (h_cost <= UCHAR_MAX));
    m_h_cost = h_cost;
}

void Solve::Entry_tt::set_is_contained_route(bool is_contained_route) noexcept {
    m_is_contained_route = is_contained_route;
}

int Solve::Entry_tt::get_h_cost() const noexcept {
    return m_h_cost;
}

int Solve::Entry_tt::get_is_contained_route() const noexcept {
    return m_is_contained_route;
}

#ifdef TEST_ZKEY
bool Solve::Entry_tt::identify(const Position& position) const noexcept {
    return m_union_array_card_below.identify(position);
}
#endif

int Solve::dfstt1(int th, int g_cost, Position::Action* path, Solve::Entry_tt& entry_m_position) noexcept {
    if(m_position.ncard_rest() == 0) {
        m_is_solved = true;
        copy_n(path, g_cost, m_answer);
        assert(th == 0);
        return 0;
    }

    if(entry_m_position.get_is_contained_route()) {
        entry_m_position.set_is_contained_route(false);
        assert(entry_m_position.get_h_cost() == lookup(m_position).first);
        return UCHAR_MAX;
    }
    entry_m_position.set_is_contained_route(true);

    Position::Action actions[MAX_ACTION_SIZE];
    int new_th;
    int naction = m_position.gen_actions(actions);
    assert(naction <= MAX_ACTION_SIZE);
    if(naction == 0) {
        new_th = UCHAR_MAX;
    }
    else {
        new_th = UCHAR_MAX;
        for(int i=0; i<naction; ++i) {
            path[g_cost] = actions[i];
            m_position.make(actions[i]);

            int nauto = m_position.move_auto(path + g_cost + 1);
            int cost = 1 + nauto;
            int new_g_cost = g_cost + cost;
            assert(new_g_cost <= UCHAR_MAX);

            auto pair_lookup = lookup(m_position);
            int h_cost = pair_lookup.first;
            assert((h_cost >= m_position.ncard_rest()) && (h_cost <= UCHAR_MAX));
            assert(cost + h_cost >= th);
            if(cost + h_cost <= th)
                new_th = min(new_th, cost + dfstt1(th - cost, new_g_cost, path, pair_lookup.second));
            else
                new_th = min(new_th, cost + h_cost);

            for(int j=new_g_cost-1; j>=g_cost; --j)
                m_position.unmake(path[j]);

            if(m_is_solved) {
                assert(th == new_th);
                break;
            }
        }
    }

    assert(new_th >= entry_m_position.get_h_cost());
    assert(entry_m_position.get_is_contained_route());
    entry_m_position.set_h_cost(new_th);
    entry_m_position.set_is_contained_route(false);
    return new_th;
}

Solve::Solve(int game_id) noexcept : m_game_id(game_id), m_position(game_id) {
    Position::Action path[UCHAR_MAX];
    int nauto = m_position.move_auto(path);
    int g_cost = nauto;
    int th = m_position.calc_h_cost();
    auto pair = m_tt.emplace(piecewise_construct, forward_as_tuple(m_position.get_zobrist_key()), forward_as_tuple(th, m_position));
    assert(pair.second);
    Solve::Entry_tt& entry = (pair.first)->second;
    m_is_solved = false;
    while(true) {
        th = dfstt1(th, g_cost, path, entry);
        if((th == UCHAR_MAX) || (m_is_solved))
            break;
    }
    if(! m_is_solved) {
        cout << "couldn't solve" << endl;
        terminate();
    }
    int min_f_cost = g_cost + th;
    cout << "shortest path length : " << endl;
    cout << min_f_cost << endl;

    cout << "shortest path : " << endl;
    for(int i=0; i<min_f_cost; ++i)
        cout << m_answer[i].gen_SN() << " ";
    cout << endl << endl;
}