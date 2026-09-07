#pragma once

#include "BitboardUtils.h"

class Knight
{
public:

    static Bitboard attacksSlow(int square);

    static void initKnightAttacks();

    static Bitboard attacks(int square);

private:

    static Bitboard KnightAttackTable[64];
};