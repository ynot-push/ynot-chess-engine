#include <Bishop.h>
Bitboard Bishop::BishopMask(int square){
    int rank=square/8;
    int file=square%8;
    Bitboard mask=0;
    for(int r = rank + 1, f = file + 1;r<=6 && f<=6;r++,f++){
        mask |=1ull<<(r*8+f);
    }
   for(int r = 1 , f =  1;r<rank && f<file;r++,f++){
         mask |=1ull<<(r*8+f);
    }
     for(int r = rank+1 , f = 1;r<=6 && f<file;r++,f++){
         mask |=1ull<<(r*8+f);
    }
    for(int r = 1 , f = file+1;r<rank && f<=6;r++,f++){
         mask |=1ull<<(r*8+f);
    }



    return mask;
}
Bitboard Bishop::attacks(int square, Bitboard occupied)
{
    int rank = square / 8;
    int file = square % 8;

    Bitboard attacks = 0;


    for (int r = rank + 1, f = file + 1;
         r <= 7 && f <= 7;
         r++, f++)
    {
        int sq = r * 8 + f;
        attacks |= 1ULL << sq;

        if (occupied & (1ULL << sq))
            break;
    }

    for (int r = rank - 1, f = file - 1;
         r >= 0 && f >= 0;
         r--, f--)
    {
        int sq = r * 8 + f;
        attacks |= 1ULL << sq;

        if (occupied & (1ULL << sq))
            break;
    }


    for (int r = rank - 1, f = file + 1;
         r >= 0 && f <= 7;
         r--, f++)
    {
        int sq = r * 8 + f;
        attacks |= 1ULL << sq;

        if (occupied & (1ULL << sq))
            break;
    }

    for (int r = rank + 1, f = file - 1;
         r <= 7 && f >= 0;
         r++, f--)
    {
        int sq = r * 8 + f;
        attacks |= 1ULL << sq;

        if (occupied & (1ULL << sq))
            break;
    }

    return attacks;
}