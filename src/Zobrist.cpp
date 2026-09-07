#include "Zobrist.h"
#include <random>

uint64_t Zobrist::pieceKeys[12][64];
uint64_t Zobrist::sideKey;
uint64_t Zobrist::castlingKeys[16];
uint64_t Zobrist::enPassantFileKeys[8];

void Zobrist::init()
{
    std::mt19937_64 rng(0x9E3779B97F4A7C15ULL);

    for (int piece = 0; piece < 12; piece++)
        for (int square = 0; square < 64; square++)
            pieceKeys[piece][square] = rng();

    sideKey = rng();

    for (int i = 0; i < 16; i++)
        castlingKeys[i] = rng();

    for (int i = 0; i < 8; i++)
        enPassantFileKeys[i] = rng();
}
