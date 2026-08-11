#pragma once

#include <bit>
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
}