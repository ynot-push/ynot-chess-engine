#pragma once
enum Piece {
    WPAWN, WKNIGHT, WBISHOP, WROOK, WQUEEN, WKING,
    BPAWN, BKNIGHT, BBISHOP, BROOK, BQUEEN, BKING,
    EMPTY
};
bool isBlackPiece(Piece piece);
bool isWhitePiece(Piece piece); 
bool isEnemyPiece(Piece movingPiece, Piece targetPiece);