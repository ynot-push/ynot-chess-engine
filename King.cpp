#include "King.h"
#include "Board.h"
#include "Move.h"
#include "Piece.h"
#include "square.h"
#include <vector>
std::vector<Move> generateKingMoves(const Board &board, int squareIndex) {
    Piece piece= board.getPiece(squareIndex);
    if(piece!=WKING && piece!=BKING){
        return {};
    }
    std::vector<Move> moves;
    int directions[8] = {1, -1, 8, -8, 9, -9, 7, -7};
    for(int i=0;i<8;i++){
        int targetSquare = squareIndex + directions[i];
        if(!isValid(targetSquare)){
            continue;
        }
        if((directions[i]==1 && getColumn(targetSquare)==0) || (directions[i]==-1 && getColumn(targetSquare)==7) || (directions[i]==9 && getColumn(targetSquare)==0) || (directions[i]==-9 && getColumn(targetSquare)==7) || (directions[i]==7 && getColumn(targetSquare)==7) || (directions[i]==-7 && getColumn(targetSquare)==0)){
            continue;
        }
        Piece targetPiece = board.getPiece(targetSquare);
        if(targetPiece==EMPTY || isEnemyPiece(piece,targetPiece)){
            moves.emplace_back(squareIndex,targetSquare);
        }

    }
    return moves;   
}