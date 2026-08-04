#pragma once
#include "Piece.h"
#include "Move.h"
class Board {

private:
    Piece squares[64];

public:
    Board();
    void initialize();
    void printBoard() const;
    void makeMove(const Move& move);
    Piece getPiece(int index) const;
};