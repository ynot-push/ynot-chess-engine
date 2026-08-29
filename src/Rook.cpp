#include "Rook.h"

#include <cstdint>
#include <random>
#include <cstring>

Bitboard findRookMagic(int square, int relevantBits);
Bitboard randomSparse64();

Bitboard Rook::RookAttackTable[64][4096];
Bitboard Rook::RookMagics[64];
int Rook::RookRelevantBits[64];

Bitboard randomSparse64()
{
    static std::mt19937_64 rng(std::random_device{}());

    return rng() & rng() & rng();
}

Bitboard Rook::RookMask(int square){
    int rank=square/8;
    int file=square%8;
    Bitboard mask=0;
    for(int i=rank+1;i<=6;i++){
        mask |=1ull<<(i*8+file);
    }
    for(int i=1;i<rank;i++){
         mask |=1ull<<(i*8+file);
    }
     for(int i=file+1;i<=6;i++){
         mask |=1ull<<(rank*8+i);
    }
     for(int i=1;i<file;i++){
         mask |=1ull<<(rank*8+i);
    }



    return mask;
}
Bitboard Rook::setOccupancy(int index, int bitsInMask, Bitboard attackMask){
    Bitboard Occupancy=0;
    for(int count = 0; count < bitsInMask; count++){
        int sqaure=__builtin_ctzll(attackMask);
        attackMask &=attackMask-1;
        if(index&(1<<count)){
            Occupancy|=1ULL<<sqaure;
        }
    }
    return Occupancy;
}

void Rook::initRookAttacks()
{
    for (int square = 0; square < 64; square++)
    {
        Bitboard mask = RookMask(square);

        int relevantBits = __builtin_popcountll(mask);

        RookRelevantBits[square] = relevantBits;

        // Find magic number for this square
        RookMagics[square] =
            findRookMagic(square, relevantBits);

        int possibleOccupancies = 1 << relevantBits;

        for (int index = 0;
             index < possibleOccupancies;
             index++)
        {
            Bitboard occupancy =
                setOccupancy(index, relevantBits, mask);

            Bitboard attack =
                attacksSlow(square, occupancy);

            int magicIndex =
                (occupancy * RookMagics[square])
                >> (64 - relevantBits);

            RookAttackTable[square][magicIndex] =
                attack;
        }
    }
}

Bitboard findRookMagic(int square, int relevantBits)
{
    Bitboard mask = Rook::RookMask(square);

    int numOccupancies = 1 << relevantBits;

    Bitboard occupancies[4096];
    Bitboard attacks[4096];

    // Generate every possible occupancy
    for (int index = 0; index < numOccupancies; index++)
    {
        occupancies[index] =
            Rook::setOccupancy(index, relevantBits, mask);

        attacks[index] =
            Rook::attacksSlow(square, occupancies[index]);
    }

    // Search for magic number
    while (true)
    {
        Bitboard magic = randomSparse64();

        // Quick rejection test
        if (__builtin_popcountll(
                (mask * magic) & 0xFF00000000000000ULL) < 6)
        {
            continue;
        }

        Bitboard used[4096];

        for (int i = 0; i < 4096; i++)
            used[i] = 0xFFFFFFFFFFFFFFFFULL;

        bool fail = false;

        for (int i = 0; i < numOccupancies; i++)
        {
            int index =
                (occupancies[i] * magic)
                >> (64 - relevantBits);

            if (used[index] == 0xFFFFFFFFFFFFFFFFULL)
            {
                used[index] = attacks[i];
            }
            else if (used[index] != attacks[i])
            {
                fail = true;
                break;
            }
        }

        if (!fail)
            return magic;
    }
}


Bitboard Rook::attacksSlow(int square,Bitboard occupied)
{
    int rank=square/8;
    int file=square%8;
    Bitboard attacks=0;
    int i=1;

    while(true){
        rank+=i;
        if(rank>7){
            break;
        }
        attacks|=1ULL<<rank*8+file;
        if(1ULL<<rank*8+file & occupied){
            break;
        }
    }
    rank=square/8;
    file=square%8;
     while(true){
        rank-=i;
        if(rank<0){
            break;
        }
        attacks|=1ULL<<rank*8+file;
        if(1ULL<<rank*8+file & occupied){
            break;
        }
        
    }
    rank=square/8;
    file=square%8;
     while(true){
        file+=i;
        if(file>7){
            break;
        }
        attacks|=1ULL<<rank*8+file;
        if(1ULL<<rank*8+file & occupied){
            break;
        }
    }
    rank=square/8;
    file=square%8;
     while(true){
        file-=i;
        if( file<0){
            break;
        }
        attacks|=1ULL<<rank*8+file;
        if(1ULL<<rank*8+file & occupied){
            break;
        }
        
    }
    return attacks;
}

Bitboard Rook::attacks(int square, Bitboard occupied)
{
    Bitboard mask = RookMask(square);

    occupied &= mask;

    int index =
        (occupied * RookMagics[square])
        >> (64 - RookRelevantBits[square]);

    return RookAttackTable[square][index];
}


