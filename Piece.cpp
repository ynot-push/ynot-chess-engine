#include "Piece.h"

bool isWhitePiece(Piece piece){
    return piece == WPAWN || piece == WKNIGHT || piece == WBISHOP || piece == WROOK || piece == WQUEEN || piece == WKING;
}    
bool isBlackPiece(Piece piece){
    return piece == BPAWN || piece == BKNIGHT || piece == BBISHOP || piece == BROOK || piece == BQUEEN || piece == BKING;
}    
bool isEnemyPiece(Piece movingPiece, Piece targetPiece){
    return (isWhitePiece(movingPiece) && isBlackPiece(targetPiece)) || (isBlackPiece(movingPiece) && isWhitePiece(targetPiece));
}
