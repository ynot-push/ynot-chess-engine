#pragma once
#include "PieceType.h"
struct Move {
    int from;
    int to;
    Piece piece;
    Piece captured;
    int promotion;
    bool isEnPassant;
    bool isCastling;
    

};