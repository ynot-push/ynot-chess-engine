#include "Bishop.h"

#include <random>

Bitboard Bishop::BishopAttackTable[64][4096];
Bitboard Bishop::BishopMagics[64];
int      Bishop::BishopRelevantBits[64];

namespace
{
    Bitboard findBishopMagic(int square, int relevantBits);

    constexpr Bitboard kHighByteMask = 0xFF00000000000000ULL;
    constexpr int       kMinHighByteBits = 6;

    Bitboard randomSparse64()
    {
        static std::mt19937_64 rng(std::random_device{}());
        return rng() & rng() & rng();
    }
}

Bitboard Bishop::BishopMask(int square)
{
    const int rank = square / 8;
    const int file = square % 8;

    Bitboard mask = 0ULL;

    for (int r = rank + 1, f = file + 1; r <= 6 && f <= 6; r++, f++)
        mask |= 1ULL << (r * 8 + f);

    for (int r = rank - 1, f = file - 1; r >= 1 && f >= 1; r--, f--)
        mask |= 1ULL << (r * 8 + f);

    for (int r = rank - 1, f = file + 1; r >= 1 && f <= 6; r--, f++)
        mask |= 1ULL << (r * 8 + f);

    for (int r = rank + 1, f = file - 1; r <= 6 && f >= 1; r++, f--)
        mask |= 1ULL << (r * 8 + f);

    return mask;
}

Bitboard Bishop::setOccupancy(int index, int bitsInMask, Bitboard attackMask)
{
    Bitboard occupancy = 0ULL;

    for (int count = 0; count < bitsInMask; count++)
    {
        const int square = __builtin_ctzll(attackMask);
        attackMask &= attackMask - 1;

        if (index & (1 << count))
            occupancy |= 1ULL << square;
    }

    return occupancy;
}

void Bishop::initBishopAttacks()
{
    for (int square = 0; square < 64; square++)
    {
        const Bitboard mask = BishopMask(square);
        const int relevantBits = __builtin_popcountll(mask);

        BishopRelevantBits[square] = relevantBits;
        BishopMagics[square] = findBishopMagic(square, relevantBits);

        const int possibleOccupancies = 1 << relevantBits;

        for (int index = 0; index < possibleOccupancies; index++)
        {
            const Bitboard occupancy = setOccupancy(index, relevantBits, mask);
            const Bitboard attack = attacksSlow(square, occupancy);

            const int magicIndex =
                (occupancy * BishopMagics[square]) >> (64 - relevantBits);

            BishopAttackTable[square][magicIndex] = attack;
        }
    }
}

Bitboard Bishop::attacksSlow(int square, Bitboard occupied)
{
    const int rank = square / 8;
    const int file = square % 8;

    Bitboard attacks = 0ULL;

    for (int r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++)
    {
        const int sq = r * 8 + f;
        attacks |= 1ULL << sq;
        if (occupied & (1ULL << sq))
            break;
    }

    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--)
    {
        const int sq = r * 8 + f;
        attacks |= 1ULL << sq;
        if (occupied & (1ULL << sq))
            break;
    }

    for (int r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++)
    {
        const int sq = r * 8 + f;
        attacks |= 1ULL << sq;
        if (occupied & (1ULL << sq))
            break;
    }

    for (int r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--)
    {
        const int sq = r * 8 + f;
        attacks |= 1ULL << sq;
        if (occupied & (1ULL << sq))
            break;
    }

    return attacks;
}

namespace
{
    Bitboard findBishopMagic(int square, int relevantBits)
    {
        const Bitboard mask = Bishop::BishopMask(square);
        const int numOccupancies = 1 << relevantBits;

        Bitboard occupancies[4096];
        Bitboard attacks[4096];

        for (int index = 0; index < numOccupancies; index++)
        {
            occupancies[index] = Bishop::setOccupancy(index, relevantBits, mask);
            attacks[index] = Bishop::attacksSlow(square, occupancies[index]);
        }

        while (true)
        {
            const Bitboard magic = randomSparse64();

            if (__builtin_popcountll((mask * magic) & kHighByteMask) < kMinHighByteBits)
                continue;

            Bitboard used[4096];
            for (int i = 0; i < numOccupancies; i++)
                used[i] = 0xFFFFFFFFFFFFFFFFULL;

            bool collision = false;

            for (int i = 0; i < numOccupancies; i++)
            {
                const int index = (occupancies[i] * magic) >> (64 - relevantBits);

                if (used[index] == 0xFFFFFFFFFFFFFFFFULL)
                {
                    used[index] = attacks[i];
                }
                else if (used[index] != attacks[i])
                {
                    collision = true;
                    break;
                }
            }

            if (!collision)
                return magic;
        }
    }
}

Bitboard Bishop::attacks(int square, Bitboard occupied)
{
    const Bitboard mask = BishopMask(square);
    occupied &= mask;

    const int index =
        (occupied * BishopMagics[square]) >> (64 - BishopRelevantBits[square]);

    return BishopAttackTable[square][index];
}