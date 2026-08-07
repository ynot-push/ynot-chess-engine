#include "King.h"
#include "Board.h"
#include "Move.h"
#include "Piece.h"
#include "square.h"
#include <vector>
std::vector<Move> generateKingMoves(const Board &board, int squareIndex) {
    Piece piece= board.getPiece(squareIndex);
    std::vector<Move> moves;
    
    if(piece!=WKING && piece!=BKING){
        return {};
    }
    
    if(piece==WKING){
          if(board.canWhiteCastleKingside() && board.getPiece(squareIndex+1)==EMPTY && board.getPiece(squareIndex+2)==EMPTY ){
              moves.emplace_back(squareIndex,squareIndex+2);
          }
          if(board.canWhiteCastleQueenside() && board.getPiece(squareIndex-1)==EMPTY && board.getPiece(squareIndex-2)==EMPTY && board.getPiece(squareIndex-3)==EMPTY){
            moves.emplace_back(squareIndex,squareIndex-2); 
          }
    }
    if(piece==BKING){
          if(board.canBlackCastleKingside() && board.getPiece(squareIndex+1)==EMPTY && board.getPiece(squareIndex+2)==EMPTY ){
             moves.emplace_back(squareIndex,squareIndex+2);
          }
          if(board.canBlackCastleQueenside() && board.getPiece(squareIndex-1)==EMPTY && board.getPiece(squareIndex-2)==EMPTY && board.getPiece(squareIndex-3)==EMPTY){
             moves.emplace_back(squareIndex,squareIndex-2);
          }
    }
    
    int directions[8] = {1, -1, 8, -8, 9, -9, 7, -7};
    for(int i=0;i<8;i++){
        int targetSquare = squareIndex + directions[i];
        if(!isValid(targetSquare)){
            continue;
        }
        int oldCol = getColumn(squareIndex);
        int newCol = getColumn(targetSquare);
        int oldRow = getRow(squareIndex);
        int newRow = getRow(targetSquare);
        int rowdiff = abs(oldRow - newRow);
        int coldiff = abs(oldCol - newCol);
        if((coldiff > 1) || (rowdiff > 1)){
            continue;
        }
        Piece targetPiece = board.getPiece(targetSquare);
        if(targetPiece==EMPTY || isEnemyPiece(piece,targetPiece)){
            moves.emplace_back(squareIndex,targetSquare);
        }

    }
    
    return moves;   
}