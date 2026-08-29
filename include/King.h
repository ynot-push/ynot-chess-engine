#pragma once

#include "BitboardUtils.h"

class King {

public:

    static Bitboard attacks(int square);
private:
    static Bitboard KingAttacks[64];
};