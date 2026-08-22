
#include "Queen.h"
#include "Rook.h"
#include "Bishop.h"

 Bitboard Queen::QueenMask (int square){
   Bitboard RookM=Rook::RookMask(square);
   Bitboard bishopM= Bishop::BishopMask(square);
   return bishopM | RookM;

 }


Bitboard Queen::attacks(int square,Bitboard occupied)
{
   Bitboard RookAtt=Rook::attacks(square,occupied);
   Bitboard bishopAtt= Bishop::attacks(square,occupied);
   return RookAtt | bishopAtt;
}