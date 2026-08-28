#pragma once

#include "BitboardUtils.h"

class Bishop
{
public:


static Bitboard BishopMask(int square);

static Bitboard setOccupancy(
    int index,
    int bitsInMask,
    Bitboard attackMask);

static Bitboard attacksSlow(
    int square,
    Bitboard occupied);

static void initBishopAttacks();

static Bitboard attacks(
    int square,
    Bitboard occupied);

private:

static Bitboard BishopAttackTable[64][4096];

static Bitboard BishopMagics[64];

static int BishopRelevantBits[64];

};
