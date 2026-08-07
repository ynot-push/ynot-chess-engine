#pragma once
#include "Piece.h"
struct Move {
    int from;
    int to;
    
    Piece promotionPiece=EMPTY;
    Move(int fromSquare, int toSquare,Piece promotion=EMPTY)
        : from(fromSquare), to(toSquare), promotionPiece(promotion)
    {


    }

};