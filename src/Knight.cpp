#include "Knight.h"

Bitboard Knight::attacks(int square)
{
   Bitboard attacks=0;
   int rank=square/8;
   int file=square % 8;
    
   // +2 rank, +1 file

    if (rank + 2 < 8 && file + 1 < 8)
     attacks |= 1ULL<<((2+rank)*8+(file+1));
   // +2 rank, -1 file
   if (rank + 2 < 8 && file - 1 >= 0)
      attacks |= 1ULL<<((2+rank)*8+(file-1));
   // -2 rank, +1 file
    if (rank - 2 >= 0 && file + 1 < 8)
    attacks |= 1ULL<<((rank-2)*8+(file+1));
   // -2 rank, -1 file
    if (rank - 2 >= 0 && file - 1 >= 0)
    attacks |= 1ULL<<((rank-2)*8+(file-1));
     // +1 rank, +2 file
    if (rank + 1 < 8 && file + 2 < 8)
     attacks |= 1ULL<<((rank+1)*8+(file+2));
      // +1 rank, -2 file
    if (rank + 1 < 8 && file - 2 >= 0)  
      attacks |= 1ULL<<((rank+1)*8+(file-2));
       // -1 rank, +2 file
     if (rank - 1 >= 0 && file + 2 < 8)  
       attacks |= 1ULL<<((rank-1)*8+(file+2));
        // -1 rank, -2 file
     if (rank - 1 >= 0 && file - 2 >= 0)
    attacks |= 1ULL << ((rank-1)*8+(file-2));

   return attacks;

}
Bitboard KnightAttacks[64];

void initKnightAttacks()
{
    for (int square = 0; square < 64; square++)
    {
        KnightAttacks[square] =
            Knight::attacks(square);
    }
}