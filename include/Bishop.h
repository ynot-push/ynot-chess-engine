#pragma once

#include "BitboardUtils.h"

class Bishop {

public:
    static Bitboard BishopMask(int square);
    static Bitboard attacks(int square,Bitboard occupied);

};