#pragma once

#include <bit>
#include <random>
#include <cstdlib>
#include "Bitboard.h"

namespace BitboardUtils {

    inline Bitboard setBit(Bitboard board, int square) {
        return board | (1ULL << square);
    }

    inline Bitboard clearBit(Bitboard board, int square) {
        return board & ~(1ULL << square);
    }

    inline Bitboard toggleBit(Bitboard board, int square) {
        return board ^ (1ULL << square);
    }

    inline bool getBit(Bitboard board, int square) {
        return board & (1ULL << square);
    }

    inline int popcount(Bitboard board) {
        return std::popcount(board);
    }

    inline int getLSB(Bitboard board) {
        return std::countr_zero(board);
    }

    inline Bitboard popLSB(Bitboard& board) {
        Bitboard lsb = board & -board;
        board &= board - 1;
        return lsb;
    }


    inline Bitboard squaresBetween(int a, int b)
    {
        const int ar = a / 8, af = a % 8;
        const int br = b / 8, bf = b % 8;

        int stepR = (br > ar) - (br < ar);
        int stepF = (bf > af) - (bf < af);

        const bool sameRank = (ar == br);
        const bool sameFile = (af == bf);
        const bool sameDiag = (std::abs(br - ar) == std::abs(bf - af));

        if (!sameRank && !sameFile && !sameDiag)
            return 0ULL;

        Bitboard result = 0ULL;
        int r = ar + stepR, f = af + stepF;
        while (r != br || f != bf)
        {
            result |= 1ULL << (r * 8 + f);
            r += stepR;
            f += stepF;
        }
        return result;
    }

    inline Bitboard setOccupancy(int index, int bitsInMask, Bitboard attackMask)
    {
        Bitboard occupancy = 0ULL;

        for (int count = 0; count < bitsInMask; count++)
        {
            const int square = getLSB(attackMask);
            attackMask &= attackMask - 1;

            if (index & (1 << count))
                occupancy |= 1ULL << square;
        }

        return occupancy;
    }

    inline Bitboard randomSparse64()
    {
        static std::mt19937_64 rng(std::random_device{}());
        return rng() & rng() & rng();
    }

    template <typename SlowAttacksFn>
    inline Bitboard findMagic(int square, int relevantBits, Bitboard mask, SlowAttacksFn slowAttacks)
    {
        constexpr Bitboard kHighByteMask = 0xFF00000000000000ULL;
        constexpr int kMinHighByteBits = 6;

        const int numOccupancies = 1 << relevantBits;

        Bitboard occupancies[4096];
        Bitboard attacks[4096];
        Bitboard used[4096];

        for (int index = 0; index < numOccupancies; index++)
        {
            occupancies[index] = setOccupancy(index, relevantBits, mask);
            attacks[index] = slowAttacks(square, occupancies[index]);
        }

        while (true)
        {
            const Bitboard magic = randomSparse64();

            if (popcount((mask * magic) & kHighByteMask) < kMinHighByteBits)
                continue;

            for (int i = 0; i < numOccupancies; i++)
                used[i] = 0xFFFFFFFFFFFFFFFFULL;

            bool collision = false;

            for (int i = 0; i < numOccupancies; i++)
            {
                const int index = static_cast<int>((occupancies[i] * magic) >> (64 - relevantBits));

                if (used[index] == 0xFFFFFFFFFFFFFFFFULL)
                    used[index] = attacks[i];
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