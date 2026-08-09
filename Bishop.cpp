#include "Bishop.h"

#include "Board.h"

#include "Move.h"

#include "Piece.h"

std::vector<Move> generateBishopMoves(const Board &board, int squareIndex){
    std::vector<Move>moves;
    const int directions[] = {9, 7, -9, -7};
    return generateSlidingMoves(board,squareIndex,directions,4);
}