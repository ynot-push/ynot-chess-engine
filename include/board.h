#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>

#include "Piece.h"
#include "Move.h"
#include "MoveList.h"
#include "NNUE.h"
#include "Zobrist.h"
#include "TranspositionTable.h"

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
    Side sideToMove;

    bool whiteKingsideCastle;
    bool whiteQueensideCastle;
    bool blackKingsideCastle;
    bool blackQueensideCastle;

    int enPassantSquare;

    struct UndoState
    {
        int enPassantSquare;
        bool whiteKingsideCastle;
        bool whiteQueensideCastle;
        bool blackKingsideCastle;
        bool blackQueensideCastle;
        uint64_t zobristKey;
    };
    std::vector<UndoState> history;

    uint64_t zobristKey;
    std::vector<TTEntry> tt;

    int32_t accumWhite[NNUE::FT_DIM];
    int32_t accumBlack[NNUE::FT_DIM];

    long long nodeCount;

    Bitboard& bitboardFor(Piece piece);

    int negamax(int depth, int alpha, int beta, int ply);
    int quiescence(int alpha, int beta, int ply);

    static int pieceValue(Piece piece);
    int moveScore(const Move& move, int ply, const Move& ttMove) const;
    void orderMoves(MoveList& moves, int ply, const Move& ttMove) const;
    void recordCutoff(const Move& move, int depth, int ply);
    void clearSearchHeuristics();

    bool hasNonPawnMaterial(bool white) const;
    void makeNullMove();
    void unmakeNullMove();

    static int scoreToTT(int score, int ply);
    static int scoreFromTT(int score, int ply);
    static constexpr int MAX_PLY = 128;
    Move killerMoves[MAX_PLY][2];
    int historyHeuristic[64][64];

    void refreshAccumulators();
    void applyAccumulatorUpdate(const Move& move);
    void undoAccumulatorUpdate(const Move& move);

    uint64_t computeZobristKey() const;
    TTEntry* probeTT(uint64_t key);
    void storeTT(uint64_t key, int depth, int score, TTFlag flag, const Move& bestMove);

public:

    Board();

    void initialize();

    void printBoard() const;
    void updateOccupancy();
    void makeMove(const Move& move, bool updateNNUE = true);
    void unmakeMove(const Move& move, bool updateNNUE = true);

    Piece getPiece(int square) const;

    std::vector<Move> generateAllMoves(int side) const;
    MoveList generateLegalMoves();

    bool isSquareAttacked(int square, int side) const;
    bool isSquareAttackedWithOccupancy(int square, int side, Bitboard customOccupied) const;
    bool isInCheck(bool white) const;

    long long perft(int depth);

    Move findBestMove(int depth);
    int evaluateNNUE() const;

    int evaluatePosition(int depth);

    void loadFEN(const std::string& fen);

    bool isWhiteToMove() const;
    long long getNodeCount() const;

    Bitboard getWhitePawns() const;
    Bitboard getWhiteKnights() const;
    Bitboard getWhiteBishops() const;
    Bitboard getWhiteRooks() const;
    Bitboard getWhiteQueens() const;
    Bitboard getWhiteKing() const;

    Bitboard getBlackPawns() const;
    Bitboard getBlackKnights() const;
    Bitboard getBlackBishops() const;
    Bitboard getBlackRooks() const;
    Bitboard getBlackQueens() const;
    Bitboard getBlackKing() const;

    Bitboard getWhitePieces() const;
    Bitboard getBlackPieces() const;
    Bitboard getOccupied() const;
    Bitboard getEmpty() const;
    bool wouldLeaveKingInCheck(const Move& move, int side);

    uint64_t getZobristKey() const;
    void clearTranspositionTable();

    // timing instrumentation
    static inline double timeGenMoves = 0.0;
    static inline double timeLegalityFilter = 0.0;
    static inline double timeEval = 0.0;
    static inline double timeMakeUnmake = 0.0;
};