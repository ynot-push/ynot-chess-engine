#pragma once

#include "BitboardUtils.h"

class Queen {

public:
    static Bitboard QueenMask(int square);
    static Bitboard attacks(int square,Bitboard occupied);

};