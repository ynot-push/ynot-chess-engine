#pragma once
#include "Board.h"
#include "Move.h"
#include <vector>
enum Piece {
    WPAWN, WKNIGHT, WBISHOP, WROOK, WQUEEN, WKING,
    BPAWN, BKNIGHT, BBISHOP, BROOK, BQUEEN, BKING,
    EMPTY
};
bool isBlackPiece(Piece piece);
bool isWhitePiece(Piece piece); 
bool isEnemyPiece(Piece movingPiece, Piece targetPiece);
std::vector<Move> generateSlidingMoves(const Board &board, int squareIndex, const int directions[], int count);
std::vector<Move> generateJumpingMoves(const Board &board, int squareIndex,const int directions[], int count);