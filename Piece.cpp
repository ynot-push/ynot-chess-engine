#include "Piece.h"
#include "square.h"
bool isWhitePiece(Piece piece){
    return piece == WPAWN || piece == WKNIGHT || piece == WBISHOP || piece == WROOK || piece == WQUEEN || piece == WKING;
}    
bool isBlackPiece(Piece piece){
    return piece == BPAWN || piece == BKNIGHT || piece == BBISHOP || piece == BROOK || piece == BQUEEN || piece == BKING;
}    
bool isEnemyPiece(Piece movingPiece, Piece targetPiece){
    return (isWhitePiece(movingPiece) && isBlackPiece(targetPiece)) || (isBlackPiece(movingPiece) && isWhitePiece(targetPiece));
}


std::vector<Move> generateSlidingMoves(const Board &board, int squareIndex, const int directions[], int count){
   std::vector<Move> moves;
   Piece piece= board.getPiece(squareIndex);
   for(int i=0;i<count;i++){
     int currentSquare=squareIndex;
     while(true){
        int previousSquare=currentSquare;
        currentSquare+=directions[i];
        if(!isValid(currentSquare)){
            break;
        }
        int previousCol=getColumn(previousSquare);
        int currentCol=getColumn(currentSquare);
        int previousRow=getRow(previousSquare);
        int currentRow=getRow(currentSquare);
        if(abs(previousCol-currentCol)>1 || abs(previousRow-currentRow)>1){
            break;
        }
        Piece currentPiece = board.getPiece(currentSquare);
        if(currentPiece==EMPTY){
            moves.emplace_back(squareIndex,currentSquare);
            continue;
        }
        if(isEnemyPiece(piece,currentPiece)){
            moves.emplace_back(squareIndex,currentSquare);
            break;
        }
     }    


}
return moves;
}
