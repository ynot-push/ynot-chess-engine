#pragma once

#include "BitboardUtils.h"

class King
{
public:

    static Bitboard attacksSlow(int square);

    static void initKingAttacks();

    static Bitboard attacks(int square);

private:

    static Bitboard KingAttackTable[64];
};