#include "Bits.h"
Bits::Table Bits::table;

uint64_t Bits::get_bits() const noexcept {
    assert(correct());
    return m_bits; 
}

Bits Bits::operator|=(const Bits& bits) noexcept {
    assert(correct() && bits.correct());
    m_bits |= bits.m_bits;
    return *this; 
}

