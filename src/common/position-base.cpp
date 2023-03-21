#include "position-base.hpp"
Position_base::Table Position_base::table;

string Action::gen_SN() const noexcept {
    constexpr char tbl[4] = {'a', 'b', 'c', 'd'};
    string str;

    assert(ok());
    if (m_from <= 7) str += to_string(m_from + 1);
    else             str += string(1, tbl[m_from - TABLEAU_COLUMN_SIZE]);
      
    if      (m_to <=  7) str += to_string(m_to + 1);
    else if (m_to <= 11) str += string(1, tbl[m_to - TABLEAU_COLUMN_SIZE]);
    else                 str += "h";
    return str; }

