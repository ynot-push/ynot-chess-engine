#pragma once

#include "PieceType.h"
#include "Move.h"
#include <vector>
class Board;
bool isBlackPiece(Piece piece);
bool isWhitePiece(Piece piece); 
bool isEnemyPiece(Piece movingPiece, Piece targetPiece);
std::vector<Move> generateSlidingMoves(const Board &board, int squareIndex, const int directions[], int count);
std::vector<Move> generateJumpingMoves(const Board &board, int squareIndex,const int directions[], int count);