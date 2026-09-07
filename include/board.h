#pragma once
#include "PieceType.h"
#include "Move.h"
#include <vector>
#include <string>
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
    bool isSquareAttacked(int square, bool byWhite) const;
    bool isInCheck(bool white) const;
    bool whiteToMove() const;
    std::vector<Move> generateLegalMoves();
    long long perft(int depth);
  void setPosition(
    const Piece position[64],
    bool whiteToMove,
    int epSquare,
    bool bqs,
    bool bks,
    bool wqs,
    bool wks
);
    void loadFEN(const std::string& fen);
    void divide(int depth);
};
