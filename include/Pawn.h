#pragma once

#include "BitboardUtils.h"
#include "PieceType.h"

class Pawn
{
public:

    static Bitboard attacksSlow(int square, Side side);

    static void initPawnAttacks();

    static Bitboard attacks(int square, Side side);

private:

    static Bitboard PawnAttackTable[2][64];
};