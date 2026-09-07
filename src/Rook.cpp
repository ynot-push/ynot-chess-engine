#include "Rook.h"

Bitboard Rook::RookAttackTable[64][4096];
Bitboard Rook::RookMagics[64];
int Rook::RookRelevantBits[64];

Bitboard Rook::RookMask(int square)
{
    const int rank = square / 8;
    const int file = square % 8;
    Bitboard mask = 0ULL;

    for (int r = rank + 1; r <= 6; r++) mask |= 1ULL << (r * 8 + file);
    for (int r = 1; r < rank; r++)      mask |= 1ULL << (r * 8 + file);
    for (int f = file + 1; f <= 6; f++) mask |= 1ULL << (rank * 8 + f);
    for (int f = 1; f < file; f++)      mask |= 1ULL << (rank * 8 + f);

    return mask;
}

Bitboard Rook::attacksSlow(int square, Bitboard occupied)
{
    const int rank = square / 8;
    const int file = square % 8;
    Bitboard attacks = 0ULL;

    for (int r = rank + 1; r <= 7; r++)
    {
        attacks |= 1ULL << (r * 8 + file);
        if (occupied & (1ULL << (r * 8 + file))) break;
    }
    for (int r = rank - 1; r >= 0; r--)
    {
        attacks |= 1ULL << (r * 8 + file);
        if (occupied & (1ULL << (r * 8 + file))) break;
    }
    for (int f = file + 1; f <= 7; f++)
    {
        attacks |= 1ULL << (rank * 8 + f);
        if (occupied & (1ULL << (rank * 8 + f))) break;
    }
    for (int f = file - 1; f >= 0; f--)
    {
        attacks |= 1ULL << (rank * 8 + f);
        if (occupied & (1ULL << (rank * 8 + f))) break;
    }

    return attacks;
}

void Rook::initRookAttacks()
{
    for (int square = 0; square < 64; square++)
    {
        const Bitboard mask = RookMask(square);
        const int relevantBits = BitboardUtils::popcount(mask);

        RookRelevantBits[square] = relevantBits;
        RookMagics[square] = BitboardUtils::findMagic(square, relevantBits, mask, attacksSlow);

        const int possibleOccupancies = 1 << relevantBits;

        for (int index = 0; index < possibleOccupancies; index++)
        {
            const Bitboard occupancy = BitboardUtils::setOccupancy(index, relevantBits, mask);
            const Bitboard attack = attacksSlow(square, occupancy);

            const int magicIndex =
                static_cast<int>((occupancy * RookMagics[square]) >> (64 - relevantBits));

            RookAttackTable[square][magicIndex] = attack;
        }
    }
}

Bitboard Rook::attacks(int square, Bitboard occupied)
{
    const Bitboard mask = RookMask(square);
    occupied &= mask;

    const int index =
        static_cast<int>((occupied * RookMagics[square]) >> (64 - RookRelevantBits[square]));

    return RookAttackTable[square][index];
}