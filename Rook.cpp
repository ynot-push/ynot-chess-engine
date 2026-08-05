#include "Rook.h"

#include "Board.h"

#include "Move.h"

#include "square.h"

std::vector<Move> generateRookMoves(const Board &board, int squareIndex) {

 std::vector<Move> moves;
 const int directions[] = {1, -1, 8, -8};

return generateSlidingMoves(board, squareIndex, directions, 4);
}