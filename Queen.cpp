#include "Queen.h"

#include "Board.h"

#include "Move.h"

#include "square.h"
#include "Piece.h"
std::vector<Move> generateQueenMoves(const Board &board, int squareIndex) {
    std::vector<Move> moves;
    Piece piece= board.getPiece(squareIndex);
    if(piece!=WQUEEN && piece!=BQUEEN){
        return {};
    }
    const int directions[] = {1, -1, 8, -8, 9, 7, -9, -7};
    return generateSlidingMoves(board, squareIndex, directions, 8);
}