#pragma once
#include "Piece.h"
#include "Move.h"
class Board {

private:
    Piece squares[64];
    bool WhiteToMove;
    int enPassantSquare =-1;    
    bool whiteKingsideCastle = true;
    bool whiteQueensideCastle = true;
    bool blackKingsideCastle = true;
    bool blackQueensideCastle = true;

public:
    Board();
    void initialize();
    void printBoard() const;
    void makeMove(const Move& move);
    Piece getPiece(int index) const;
    std::vector<Move> generateAllMoves(); 
    bool canWhiteCastleKingside() const;
    bool canWhiteCastleQueenside() const;
    bool canBlackCastleKingside() const;
    bool canBlackCastleQueenside() const;
    int getEnPassantSquare()const;
};