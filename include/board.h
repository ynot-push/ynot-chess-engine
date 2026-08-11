#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Piece.h"
#include "Move.h"

using Bitboard = uint64_t;

class Board {

private:

    // White pieces
    Bitboard whitePawns;
    Bitboard whiteKnights;
    Bitboard whiteBishops;
    Bitboard whiteRooks;
    Bitboard whiteQueens;
    Bitboard whiteKing;

    // Black pieces
    Bitboard blackPawns;
    Bitboard blackKnights;
    Bitboard blackBishops;
    Bitboard blackRooks;
    Bitboard blackQueens;
    Bitboard blackKing;

    // Occupancy
    Bitboard whitePieces;
    Bitboard blackPieces;
    Bitboard occupied;
    Bitboard empty;

    // Game state
    bool whiteToMove;

    bool whiteKingsideCastle;
    bool whiteQueensideCastle;
    bool blackKingsideCastle;
    bool blackQueensideCastle;

    int enPassantSquare;

public:

    Board();

    void initialize();

    void printBoard() const;

    void makeMove(const Move& move);

    Piece getPiece(int square) const;

    std::vector<Move> generateAllMoves();

    std::vector<Move> generateLegalMoves();

    bool isSquareAttacked(int square, bool byWhite) const;

    bool isInCheck(bool white) const;

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

    bool canWhiteCastleKingside() const;
    bool canWhiteCastleQueenside() const;
    bool canBlackCastleKingside() const;
    bool canBlackCastleQueenside() const;

    int getEnPassantSquare() const;
};