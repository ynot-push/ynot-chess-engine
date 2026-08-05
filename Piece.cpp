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

std::vector<Move> generateJumpingMoves(const Board &board, int squareIndex,const int directions[], int count){
    std::vector<Move> moves;
    Piece piece =board.getPiece(squareIndex);
    for(int i=0;i<count;i++){
        int targetSquare = squareIndex + directions[i];
        if(!isValid(targetSquare)){
            continue;
        }
        Piece targetPiece = board.getPiece(targetSquare);
        if(targetPiece==EMPTY || isEnemyPiece(piece,targetPiece)){
            moves.emplace_back(squareIndex,targetSquare);
        }

    }
return moves;
}
std::vector<Move> generateSlidingMoves(const Board &board, int squareIndex, const int directions[], int count){
   std::vector<Move> moves;
   Piece piece= board.getPiece(squareIndex);
   for(int i=0;i<count;i++){
     int currentSquare=squareIndex;
     while(true){
        currentSquare+=directions[i];
        if(!isValid(currentSquare)){
            break;
        }
        if((directions[i]==1 && getColumn(currentSquare)==0) || (directions[i]==-1 && getColumn(currentSquare)==7) || (directions[i]==9 && getColumn(currentSquare)==0) || (directions[i]==-9 && getColumn(currentSquare)==7) || (directions[i]==7 && getColumn(currentSquare)==7) || (directions[i]==-7 && getColumn(currentSquare)==0)){
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
        if(!isEnemyPiece(piece,currentPiece)){
            break;
        }
     }    


}
return moves;
}
