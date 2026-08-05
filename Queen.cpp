#include "Queen.h"

#include "Board.h"

#include "Move.h"

#include "square.h"

std::vector<Move> generateQueenMoves(const Board &board, int squareIndex) {
    std::vector<Move> moves;
    const int directions[] = {1, -1, 8, -8, 9, 7, -9, -7};
    return generateSlidingMoves(board, squareIndex, directions, 8);
}