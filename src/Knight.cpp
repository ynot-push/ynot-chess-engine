#include "Knight.h"
#include "Board.h"
#include "Move.h"
#include "Piece.h"
#include "square.h"
#include <vector>
std::vector<Move> generateKnightMoves(const Board &board, int squareIndex) {
    Piece piece = board.getPiece(squareIndex);
    if (piece != WKNIGHT && piece != BKNIGHT) {
        return {};
    }
    std::vector<Move> moves;
    const int directions[8] = {17, 15, 10, 6, -17, -15, -10, -6};
    for(int i = 0; i < 8; i++) {
        int targetSquare = squareIndex + directions[i];
        if (!isValid(targetSquare)) {
            continue;
        }
        int oldCol = getColumn(squareIndex);
        int newCol = getColumn(targetSquare);
        int oldRow = getRow(squareIndex);
        int newRow = getRow(targetSquare);
        int rowdiff = abs(oldRow - newRow);
        int coldiff = abs(oldCol - newCol);
        if (!(coldiff == 1 && rowdiff == 2) && !(coldiff == 2 && rowdiff == 1)) {
            continue;
        }
        Piece targetPiece = board.getPiece(targetSquare);
        if (targetPiece == EMPTY || isEnemyPiece(piece, targetPiece)) {
            moves.emplace_back(squareIndex, targetSquare);
        }
    }
    return moves;
}