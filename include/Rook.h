#pragma once

#include "BitboardUtils.h"

class Rook
{
public:

static Bitboard RookMask(int square);

static Bitboard setOccupancy(
    int index,
    int bitsInMask,
    Bitboard attackMask);

static Bitboard attacksSlow(
    int square,
    Bitboard occupied);

static void initRookAttacks();

static Bitboard attacks(
    int square,
    Bitboard occupied);

private:

static Bitboard RookAttackTable[64][4096];

static Bitboard RookMagics[64];

static int RookRelevantBits[64];

};
