#pragma once

#include "BitboardUtils.h"

class Rook {

public:
    static Bitboard RookMask(int square);
    static Bitboard attacks(int square,Bitboard occupied);

};