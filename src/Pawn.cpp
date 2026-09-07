#include "Pawn.h"

#include "Board.h"

#include "Move.h"

#include "square.h"
#include "Piece.h"

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
        if(getRow(oneStepForward)==promotionRow){
            if(piece==BPAWN){
            moves.emplace_back(squareIndex,oneStepForward,BQUEEN);
            moves.emplace_back(squareIndex, oneStepForward, BROOK);
            moves.emplace_back(squareIndex, oneStepForward, BBISHOP);
            moves.emplace_back(squareIndex, oneStepForward, BKNIGHT);
            }
            else{
            moves.emplace_back(squareIndex,oneStepForward,WQUEEN);
            moves.emplace_back(squareIndex, oneStepForward, WROOK);
            moves.emplace_back(squareIndex, oneStepForward, WBISHOP);
            moves.emplace_back(squareIndex, oneStepForward, WKNIGHT);
            }
            
            

        }
        else{
            moves.emplace_back(squareIndex,oneStepForward);
            }
        if (getRow(squareIndex)==startRow){
            int twoStepForward= squareIndex +direction*16;
            if(board.getPiece(twoStepForward)==EMPTY){
                moves.emplace_back(squareIndex,twoStepForward);
            }
           } 
        
        
        
        }
    
  





    int captureLeft=squareIndex+direction*8-1;
    int captureRight=squareIndex+direction*8+1;
   if (getColumn(squareIndex) > 0 &&
    captureLeft == board.getEnPassantSquare()) {
    moves.emplace_back(squareIndex, captureLeft);
}

if (getColumn(squareIndex) < 7 &&
    captureRight == board.getEnPassantSquare()) {
    moves.emplace_back(squareIndex, captureRight);
}

    if(getColumn(squareIndex)>0 && isEnemyPiece(piece,board.getPiece(captureLeft))){
        if(getRow(captureLeft)==promotionRow){
            if(piece==BPAWN){
            moves.emplace_back(squareIndex,captureLeft,BQUEEN);
            moves.emplace_back(squareIndex, captureLeft, BROOK);
            moves.emplace_back(squareIndex, captureLeft, BBISHOP);
            moves.emplace_back(squareIndex, captureLeft, BKNIGHT);
            }
            else{
            moves.emplace_back(squareIndex,captureLeft,WQUEEN);
            moves.emplace_back(squareIndex, captureLeft, WROOK);
            moves.emplace_back(squareIndex, captureLeft, WBISHOP);
            moves.emplace_back(squareIndex, captureLeft, WKNIGHT);
            }
            
            

        }
        else{

            moves.emplace_back(squareIndex,captureLeft);
           }}
    if(getColumn(squareIndex)<7 && isEnemyPiece(piece,board.getPiece(captureRight))){
          if(getRow(captureRight)==promotionRow){
            if(piece==BPAWN){
            moves.emplace_back(squareIndex,captureRight,BQUEEN);
            moves.emplace_back(squareIndex, captureRight, BROOK);
            moves.emplace_back(squareIndex, captureRight, BBISHOP);
            moves.emplace_back(squareIndex, captureRight, BKNIGHT);
            }
            else{
            moves.emplace_back(squareIndex,captureRight,WQUEEN);
            moves.emplace_back(squareIndex, captureRight, WROOK);
            moves.emplace_back(squareIndex, captureRight, WBISHOP);
            moves.emplace_back(squareIndex, captureRight, WKNIGHT);
            }
            
            

        }
        else{    



                moves.emplace_back(squareIndex,captureRight);
           }}
    


    return moves;      
    }



