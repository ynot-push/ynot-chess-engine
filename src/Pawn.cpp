#include "Pawn.h"

Bitboard Pawn::attacks(int square)
{
   Bitboard attacks = 0;
   int rank = square / 8;
   int file = square % 8;

   if (rank + 1 < 8) {
       attacks |= 1ULL << ((rank+1)*8 + file);         

       if (file - 1 >= 0)
           attacks |= 1ULL << ((rank+1)*8 + (file-1)); 

       if (file + 1 < 8)
           attacks |= 1ULL << ((rank+1)*8 + (file+1));  
   }

   if (rank == 1) {
       attacks |= 1ULL << ((rank+2)*8 + file);   
   }

   return attacks;
}