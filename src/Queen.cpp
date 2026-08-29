#include "Queen.h"

#include "Rook.h"
#include "Bishop.h"

Bitboard Queen::QueenMask(int square)
{
Bitboard rookMask =
Rook::RookMask(square);

Bitboard bishopMask =
    Bishop::BishopMask(square);

return rookMask | bishopMask;


}

Bitboard Queen::attacks(
int square,
Bitboard occupied)
{
Bitboard rookAttacks =
Rook::attacks(
square,
occupied);


Bitboard bishopAttacks =
    Bishop::attacks(
        square,
        occupied);

return rookAttacks | bishopAttacks;


}
