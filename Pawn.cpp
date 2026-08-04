#include "Pawn.h"

#include "Board.h"

#include "Move.h"

#include "square.h"

std::vector<Move> generatePawnMoves(const Board &board, int squareIndex) {
    std::vector<Move> moves;
    Piece piece = board.getPiece(squareIndex);
    if (piece != WPAWN && piece != BPAWN) {
        return {};
    }
    
    int direction =(piece==WPAWN) ? -1 : 1;
    int startRow = (piece == WPAWN) ? 6 : 1;
    int promotionRow = (piece == WPAWN) ? 0 : 7;
     int oneStepForward= squareIndex+direction*8;
           if(board.getPiece(oneStepForward)==EMPTY){
            moves.emplace_back(squareIndex,oneStepForward);
    if (getRow(squareIndex)==startRow){
            int twoStepForward= squareIndex +direction*16;
            if(board.getPiece(twoStepForward)==EMPTY){
                moves.emplace_back(squareIndex,twoStepForward);
            }
           } 
    }
        int captureLeft=squareIndex+direction*8-1;
           int captureRight=squareIndex+direction*8+1;
           if(getColumn(squareIndex)>0 && isEnemyPiece(piece,board.getPiece(captureLeft))){
                moves.emplace_back(squareIndex,captureLeft);
           }
           if(getColumn(squareIndex)<7 && isEnemyPiece(piece,board.getPiece(captureRight))){
                moves.emplace_back(squareIndex,captureRight);
           }
    return moves;
}