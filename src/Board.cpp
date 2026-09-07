#include <iostream>
#include <vector>
#include <bitset>
#include <algorithm>
#include "Board.h"
#include "BitboardUtils.h"
#include "PieceType.h"
#include "square.h"
#include "Pawn.h"
#include "Knight.h"
#include "Rook.h"
#include "Queen.h"
#include "Bishop.h"
#include "King.h"
#include "NNUE.h"

using namespace BitboardUtils;

namespace
{

    constexpr int MATE_VALUE = 1000000;

    inline Move noMove() { return Move{ -1, -1, EMPTY, EMPTY, EMPTY, false, false }; }
    inline bool sameMove(const Move& a, const Move& b)
    {
        return a.from == b.from && a.to == b.to && a.piece == b.piece && a.promotion == b.promotion;
    }

    inline int lmrReduction(int depth, int moveIndex)
    {
        int r = 1;
        if (depth >= 6 && moveIndex >= 8)  r = 2;
        if (depth >= 10 && moveIndex >= 16) r = 3;
        return r;
    }

    void castlingRookSquares(const Move& move, int& rookFrom, int& rookTo, Piece& rookPiece)
    {
        if (move.piece == WKING)
        {
            if (move.to == 6)  { rookFrom = 7;  rookTo = 5;  rookPiece = WROOK; }
            else                { rookFrom = 0;  rookTo = 3;  rookPiece = WROOK; }
        }
        else // BKING
        {
            if (move.to == 62) { rookFrom = 63; rookTo = 61; rookPiece = BROOK; }
            else                { rookFrom = 56; rookTo = 59; rookPiece = BROOK; }
        }
    }
    int enPassantCapturedSquare(const Move& move)
    {
        return (move.piece == WPAWN) ? (move.to - 8) : (move.to + 8);
    }
}

Board::Board()
{
    // attack tables (knight/king/bishop/rook/pawn) and zobrist keys are static/global lookup data shared by all board objects, so they only need to be built once no matter how many boards  created.
    static bool attacksInitialized = false;
    if (!attacksInitialized)
    {
        Knight::initKnightAttacks();
        King::initKingAttacks();
        Bishop::initBishopAttacks();
        Rook::initRookAttacks();
        Pawn::initPawnAttacks();
        Zobrist::init();
        attacksInitialized = true;
    }

    tt.resize(TT_NUM_ENTRIES);
    clearSearchHeuristics();
    history.reserve(512);

    initialize();
}

void Board::initialize()
{
    // Reset every bitboard to empty before placing the standard starting position.
    blackPawns   = 0ULL;
    blackKnights = 0ULL;
    blackBishops = 0ULL;
    blackRooks   = 0ULL;
    blackQueens  = 0ULL;
    blackKing    = 0ULL;
    whitePawns   = 0ULL;
    whiteKnights = 0ULL;
    whiteBishops = 0ULL;
    whiteRooks   = 0ULL;
    whiteQueens  = 0ULL;
    whiteKing    = 0ULL;

    whitePawns = ((1ULL << 8) - 1) << 8;
    blackPawns = ((1ULL << 8) - 1) << 48;

    whiteKing = setBit(whiteKing, 4);
    blackKing = setBit(blackKing, 60);

    blackQueens = setBit(blackQueens, 59);
    whiteQueens = setBit(whiteQueens, 3);

    whiteBishops = setBit(whiteBishops, 5) | setBit(whiteBishops, 2);
    blackBishops = setBit(blackBishops, 58) | setBit(blackBishops, 61);

    whiteRooks = setBit(whiteRooks, 0) | setBit(whiteRooks, 7);
    blackRooks = setBit(blackRooks, 56) | setBit(blackRooks, 63);

    whiteKnights = setBit(whiteKnights, 1) | setBit(whiteKnights, 6);
    blackKnights = setBit(blackKnights, 57) | setBit(blackKnights, 62);

    sideToMove      = WHITE;
    whiteToMove     = true;
    enPassantSquare = -1;

    whiteKingsideCastle  = true;
    whiteQueensideCastle = true;
    blackKingsideCastle  = true;
    blackQueensideCastle = true;

    nodeCount = 0;

    updateOccupancy();
    refreshAccumulators();       // build NNUE accumulators from scratch for this position
    zobristKey = computeZobristKey();
}

char pieceToChar(Piece piece)
{
    switch (piece)
    {
        case WPAWN:   return 'P';
        case WKNIGHT: return 'N';
        case WBISHOP: return 'B';
        case WROOK:   return 'R';
        case WQUEEN:  return 'Q';
        case WKING:   return 'K';
        case BPAWN:   return 'p';
        case BKNIGHT: return 'n';
        case BBISHOP: return 'b';
        case BROOK:   return 'r';
        case BQUEEN:  return 'q';
        case BKING:   return 'k';
        default:      return '.';
    }
}

Piece charToPiece(char c)
{
    switch (c)
    {
        case 'P': return WPAWN;
        case 'N': return WKNIGHT;
        case 'B': return WBISHOP;
        case 'R': return WROOK;
        case 'Q': return WQUEEN;
        case 'K': return WKING;
        case 'p': return BPAWN;
        case 'n': return BKNIGHT;
        case 'b': return BBISHOP;
        case 'r': return BROOK;
        case 'q': return BQUEEN;
        case 'k': return BKING;
        default:  return EMPTY;
    }
}

void Board::printBoard() const
{
    for (int rank = 7; rank >= 0; rank--)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            std::cout << pieceToChar(getPiece(square)) << " ";
        }
        std::cout << '\n';
    }
}

void Board::updateOccupancy()
{
    whitePieces =
        whitePawns | whiteKnights | whiteBishops |
        whiteRooks | whiteQueens  | whiteKing;

    blackPieces =
        blackPawns | blackKnights | blackBishops |
        blackRooks | blackQueens  | blackKing;

    occupied = whitePieces | blackPieces;
    empty    = ~occupied;
}

Piece Board::getPiece(int square) const
{
    if (!getBit(occupied, square))
        return EMPTY;

    if (getBit(whitePawns, square))   return WPAWN;
    if (getBit(blackPawns, square))   return BPAWN;
    if (getBit(whiteKing, square))    return WKING;
    if (getBit(blackKing, square))    return BKING;
    if (getBit(whiteQueens, square))  return WQUEEN;
    if (getBit(blackQueens, square))  return BQUEEN;
    if (getBit(whiteRooks, square))   return WROOK;
    if (getBit(blackRooks, square))   return BROOK;
    if (getBit(whiteBishops, square)) return WBISHOP;
    if (getBit(blackBishops, square)) return BBISHOP;
    if (getBit(whiteKnights, square)) return WKNIGHT;
    if (getBit(blackKnights, square)) return BKNIGHT;

    return EMPTY;
}

Bitboard& Board::bitboardFor(Piece piece)
{
    // returns a reference so callers can mutate the underlying bitboard directly 
    switch (piece)
    {
        case WPAWN:   return whitePawns;
        case WKNIGHT: return whiteKnights;
        case WBISHOP: return whiteBishops;
        case WROOK:   return whiteRooks;
        case WQUEEN:  return whiteQueens;
        case WKING:   return whiteKing;
        case BPAWN:   return blackPawns;
        case BKNIGHT: return blackKnights;
        case BBISHOP: return blackBishops;
        case BROOK:   return blackRooks;
        case BQUEEN:  return blackQueens;
        case BKING:   return blackKing;
        default:
            static Bitboard dummy;
            dummy = 0ULL;
            return dummy;
    }
}

Bitboard Board::getWhitePawns()   const { return whitePawns; }
Bitboard Board::getWhiteKnights() const { return whiteKnights; }
Bitboard Board::getWhiteBishops() const { return whiteBishops; }
Bitboard Board::getWhiteRooks()   const { return whiteRooks; }
Bitboard Board::getWhiteQueens()  const { return whiteQueens; }
Bitboard Board::getWhiteKing()    const { return whiteKing; }

Bitboard Board::getBlackPawns()   const { return blackPawns; }
Bitboard Board::getBlackKnights() const { return blackKnights; }
Bitboard Board::getBlackBishops() const { return blackBishops; }
Bitboard Board::getBlackRooks()   const { return blackRooks; }
Bitboard Board::getBlackQueens()  const { return blackQueens; }
Bitboard Board::getBlackKing()    const { return blackKing; }

Bitboard Board::getWhitePieces() const { return whitePieces; }
Bitboard Board::getBlackPieces() const { return blackPieces; }
Bitboard Board::getOccupied()    const { return occupied; }
Bitboard Board::getEmpty()       const { return empty; }

bool Board::isWhiteToMove() const { return whiteToMove; }
long long Board::getNodeCount() const { return nodeCount; }

uint64_t Board::computeZobristKey() const
{
    uint64_t key = 0;

    for (int square = 0; square < 64; square++)
    {
        const Piece piece = getPiece(square);
        if (piece != EMPTY)
            key ^= Zobrist::pieceKeys[piece][square];
    }

    if (!whiteToMove)
        key ^= Zobrist::sideKey;

    key ^= Zobrist::castlingKeys[Zobrist::castlingIndex(
        whiteKingsideCastle, whiteQueensideCastle,
        blackKingsideCastle, blackQueensideCastle)];

    if (enPassantSquare != -1)
        key ^= Zobrist::enPassantFileKeys[enPassantSquare % 8];

    return key;
}

uint64_t Board::getZobristKey() const { return zobristKey; }

void Board::clearTranspositionTable()
{
    std::fill(tt.begin(), tt.end(), TTEntry{});
}

TTEntry* Board::probeTT(uint64_t key)
{
    // Fixed-size hash table indexed by the low bits of the Zobrist key.
    // Collisions are possible (different position, same slot) - the stored
    // key is checked to make sure we're not reading a stale/foreign entry.
    TTEntry& entry = tt[key & TT_INDEX_MASK];
    return (entry.flag != TTFlag::EMPTY && entry.key == key) ? &entry : nullptr;
}

void Board::storeTT(uint64_t key, int depth, int score, TTFlag flag, const Move& bestMove)
{
    TTEntry& entry = tt[key & TT_INDEX_MASK];
    if (entry.flag == TTFlag::EMPTY || entry.key != key || depth >= entry.depth)
    {
        entry.key   = key;
        entry.depth = depth;
        entry.score = score;
        entry.flag  = flag;
        entry.bestMove = bestMove;
    }
}
//to parse and load the standard fen to a board
void Board::loadFEN(const std::string& fen)
{
    whitePawns = whiteKnights = whiteBishops = 0ULL;
    whiteRooks = whiteQueens  = whiteKing    = 0ULL;
    blackPawns = blackKnights = blackBishops = 0ULL;
    blackRooks = blackQueens  = blackKing    = 0ULL;
    whitePieces = blackPieces = occupied = 0ULL;

    whiteToMove = false;
    enPassantSquare = -1;

    whiteKingsideCastle  = false;
    whiteQueensideCastle = false;
    blackKingsideCastle  = false;
    blackQueensideCastle = false;

    int rank = 7;
    int file = 0;
    size_t fenIndex = 0;

    // part 1: piece placement from rank 8 down to rank 1
    while (rank >= 0 && fenIndex < fen.length())
    {
        char c = fen[fenIndex];

        if (c == ' ') { fenIndex++; break; }
        if (c == '/') { rank--; file = 0; fenIndex++; continue; }
        if (c >= '1' && c <= '8') { file += c - '0'; fenIndex++; continue; }

        int square = rank * 8 + file;

        switch (c)
        {
            case 'P': whitePawns   |= (1ULL << square); break;
            case 'N': whiteKnights |= (1ULL << square); break;
            case 'B': whiteBishops |= (1ULL << square); break;
            case 'R': whiteRooks   |= (1ULL << square); break;
            case 'Q': whiteQueens  |= (1ULL << square); break;
            case 'K': whiteKing    |= (1ULL << square); break;
            case 'p': blackPawns   |= (1ULL << square); break;
            case 'n': blackKnights |= (1ULL << square); break;
            case 'b': blackBishops |= (1ULL << square); break;
            case 'r': blackRooks   |= (1ULL << square); break;
            case 'q': blackQueens  |= (1ULL << square); break;
            case 'k': blackKing    |= (1ULL << square); break;
        }

        file++;
        fenIndex++;
    }

    // part 2 side to move
    if (fenIndex < fen.length())
        whiteToMove = fen[fenIndex] == 'w';

    sideToMove = whiteToMove ? WHITE : BLACK;

    fenIndex += 2; // skip past side-to-move char and the following space

    // part 3: castling rights 
    while (fenIndex < fen.length() && fen[fenIndex] != ' ')
    {
        switch (fen[fenIndex])
        {
            case 'K': whiteKingsideCastle  = true; break;
            case 'Q': whiteQueensideCastle = true; break;
            case 'k': blackKingsideCastle  = true; break;
            case 'q': blackQueensideCastle = true; break;
        }
        fenIndex++;
    }
    fenIndex++;

    // part 4 en passant target square 
    if (fenIndex + 1 < fen.length() && fen[fenIndex] != '-')
    {
        std::string square;
        square += fen[fenIndex];
        square += fen[fenIndex + 1];
        enPassantSquare = notationToIndex(square);
    }
    else
    {
        enPassantSquare = -1;
    }

    updateOccupancy();
    refreshAccumulators();   // nnue accumulators must be rebuilt for each of the new position
    zobristKey = computeZobristKey();
}

bool Board::isSquareAttacked(int square, int side) const
{
    return isSquareAttackedWithOccupancy(square, side, occupied);
}

bool Board::isSquareAttackedWithOccupancy(int square, int side, Bitboard customOccupied) const
{
    Bitboard pawns, knights, bishops, rooks, queens, king;

    if (side == WHITE)
    {
        pawns   = whitePawns;
        knights = whiteKnights;
        bishops = whiteBishops;
        rooks   = whiteRooks;
        queens  = whiteQueens;
        king    = whiteKing;
    }
    else
    {
        pawns   = blackPawns;
        knights = blackKnights;
        bishops = blackBishops;
        rooks   = blackRooks;
        queens  = blackQueens;
        king    = blackKing;
    }

    if (Knight::attacks(square) & knights)
        return true;

    if (King::attacks(square) & king)
        return true;

    const Bitboard pawnAttackers =
        (side == WHITE) ? Pawn::attacks(square, BLACK) : Pawn::attacks(square, WHITE);

    if (pawnAttackers & pawns)
        return true;

    if (Bishop::attacks(square, customOccupied) & (bishops | queens))
        return true;

    if (Rook::attacks(square, customOccupied) & (rooks | queens))
        return true;

    return false;
}

bool Board::isInCheck(bool white) const
{
    const Bitboard kingBB = white ? whiteKing : blackKing;
    const int kingSquare = getLSB(kingBB);
    return isSquareAttacked(kingSquare, white ? BLACK : WHITE);
}
// to generate pseudo-legal ignoring whether they leave the mover's own king in check
std::vector<Move> Board::generateAllMoves(int side) const
{
    std::vector<Move> allMoves;
    allMoves.reserve(64);

    Bitboard ownPieces, enemyPieces;
    Bitboard pawns, knights, bishops, rooks, queens, king;

    if (side == WHITE)
    {
        ownPieces = whitePieces; enemyPieces = blackPieces;
        pawns = whitePawns; knights = whiteKnights; bishops = whiteBishops;
        rooks = whiteRooks; queens = whiteQueens;   king    = whiteKing;
    }
    else
    {
        ownPieces = blackPieces; enemyPieces = whitePieces;
        pawns = blackPawns; knights = blackKnights; bishops = blackBishops;
        rooks = blackRooks; queens = blackQueens;   king    = blackKing;
    }

    const Piece knightPiece = (side == WHITE) ? WKNIGHT : BKNIGHT;
    const Piece bishopPiece = (side == WHITE) ? WBISHOP : BBISHOP;
    const Piece rookPiece   = (side == WHITE) ? WROOK   : BROOK;
    const Piece queenPiece  = (side == WHITE) ? WQUEEN  : BQUEEN;
    const Piece kingPiece   = (side == WHITE) ? WKING   : BKING;
    const Piece pawnPiece   = (side == WHITE) ? WPAWN   : BPAWN;

    Bitboard bb = knights;
    while (bb)
    {
        int from = getLSB(bb);
        Bitboard attacks = Knight::attacks(from) & ~ownPieces;
        while (attacks)
        {
            int to = getLSB(attacks);
            allMoves.push_back(Move{ from, to, knightPiece, getPiece(to), EMPTY, false, false });
            attacks &= attacks - 1; // clear lowest set bit, move to next attack square
        }
        bb &= bb - 1;
    }

    bb = bishops;
    while (bb)
    {
        int from = getLSB(bb);
        Bitboard attacks = Bishop::attacks(from, occupied) & ~ownPieces;
        while (attacks)
        {
            int to = getLSB(attacks);
            allMoves.push_back(Move{ from, to, bishopPiece, getPiece(to), EMPTY, false, false });
            attacks &= attacks - 1;
        }
        bb &= bb - 1;
    }

    bb = rooks;
    while (bb)
    {
        int from = getLSB(bb);
        Bitboard attacks = Rook::attacks(from, occupied) & ~ownPieces;
        while (attacks)
        {
            int to = getLSB(attacks);
            allMoves.push_back(Move{ from, to, rookPiece, getPiece(to), EMPTY, false, false });
            attacks &= attacks - 1;
        }
        bb &= bb - 1;
    }

    bb = queens;
    while (bb)
    {
        int from = getLSB(bb);
        Bitboard attacks = Queen::attacks(from, occupied) & ~ownPieces;
        while (attacks)
        {
            int to = getLSB(attacks);
            allMoves.push_back(Move{ from, to, queenPiece, getPiece(to), EMPTY, false, false });
            attacks &= attacks - 1;
        }
        bb &= bb - 1;
    }

    bb = king;
    while (bb)
    {
        int from = getLSB(bb);
        Bitboard attacks = King::attacks(from) & ~ownPieces;
        while (attacks)
        {
            int to = getLSB(attacks);
            allMoves.push_back(Move{ from, to, kingPiece, getPiece(to), EMPTY, false, false });
            attacks &= attacks - 1;
        }
        bb &= bb - 1;
    }

    //making castling so squares between king/rook must be empty, and king's start,
    // pass-through, and landing squares must not be attacked
    if (side == WHITE)
    {
        if (whiteKingsideCastle &&
            !getBit(occupied, 5) && !getBit(occupied, 6) &&
            !isSquareAttacked(4, BLACK) && !isSquareAttacked(5, BLACK) && !isSquareAttacked(6, BLACK))
        {
            allMoves.push_back(Move{ 4, 6, WKING, EMPTY, EMPTY, false, true });
        }
        if (whiteQueensideCastle &&
            !getBit(occupied, 1) && !getBit(occupied, 2) && !getBit(occupied, 3) &&
            !isSquareAttacked(4, BLACK) && !isSquareAttacked(3, BLACK) && !isSquareAttacked(2, BLACK))
        {
            allMoves.push_back(Move{ 4, 2, WKING, EMPTY, EMPTY, false, true });
        }
    }
    else
    {
        if (blackKingsideCastle &&
            !getBit(occupied, 61) && !getBit(occupied, 62) &&
            !isSquareAttacked(60, WHITE) && !isSquareAttacked(61, WHITE) && !isSquareAttacked(62, WHITE))
        {
            allMoves.push_back(Move{ 60, 62, BKING, EMPTY, EMPTY, false, true });
        }
        if (blackQueensideCastle &&
            !getBit(occupied, 57) && !getBit(occupied, 58) && !getBit(occupied, 59) &&
            !isSquareAttacked(60, WHITE) && !isSquareAttacked(59, WHITE) && !isSquareAttacked(58, WHITE))
        {
            allMoves.push_back(Move{ 60, 58, BKING, EMPTY, EMPTY, false, true });
        }
    }

    bb = pawns;
    while (bb)
    {
        int from = getLSB(bb);
        int rank = from / 8;

        const int singlePush   = (side == WHITE) ? from + 8  : from - 8;
        const bool onStartRank = (side == WHITE) ? (rank == 1) : (rank == 6);
        const bool willPromote = (side == WHITE) ? (rank == 6) : (rank == 1);

        const Piece promoQueen  = (side == WHITE) ? WQUEEN  : BQUEEN;
        const Piece promoRook   = (side == WHITE) ? WROOK   : BROOK;
        const Piece promoBishop = (side == WHITE) ? WBISHOP : BBISHOP;
        const Piece promoKnight = (side == WHITE) ? WKNIGHT : BKNIGHT;

        // generate  4 promotion moves (Q/R/B/N) automatically when the pawn lands on the back rank
        auto addPawnMove = [&](int to, Piece captured = EMPTY, bool isEP = false)
        {
            if (willPromote)
            {
                allMoves.push_back(Move{from, to, pawnPiece, captured, promoQueen,  false, false});
                allMoves.push_back(Move{from, to, pawnPiece, captured, promoRook,   false, false});
                allMoves.push_back(Move{from, to, pawnPiece, captured, promoBishop, false, false});
                allMoves.push_back(Move{from, to, pawnPiece, captured, promoKnight, false, false});
            }
            else
            {
                allMoves.push_back(Move{from, to, pawnPiece, captured, EMPTY, isEP, false});
            }
        };

        if (getBit(empty, singlePush))
        {
            addPawnMove(singlePush);

            if (onStartRank)
            {
                int doublePush = (side == WHITE) ? from + 16 : from - 16;
                if (getBit(empty, doublePush))
                    addPawnMove(doublePush);
            }
        }

        const Bitboard attackTargets = Pawn::attacks(from, (side == WHITE) ? WHITE : BLACK);

        Bitboard captures = attackTargets & enemyPieces;
        while (captures)
        {
            int to = getLSB(captures);
            addPawnMove(to, getPiece(to));
            captures &= captures - 1;
        }

        if (enPassantSquare != -1 && (attackTargets & (1ULL << enPassantSquare)))
        {
            const Piece capturedPawn = (side == WHITE) ? BPAWN : WPAWN;
            allMoves.push_back(Move{from, enPassantSquare, pawnPiece, capturedPawn, EMPTY, true, false});
        }

        bb &= bb - 1;
    }

    return allMoves;
}

MoveList Board::generateLegalMoves()
{
    // fully legal move generator  this computes check/pin information up front and constrains destinations
    // per-piece, which is significantly faster for the search 
    MoveList result;

    const int side = whiteToMove ? WHITE : BLACK;
    const bool isWhite = (side == WHITE);
    const int enemySide = isWhite ? BLACK : WHITE;

#ifdef ENGINE_PROFILE
    auto t0 = std::chrono::high_resolution_clock::now();
#endif

    const Bitboard ownPieces   = isWhite ? whitePieces : blackPieces;
    const Bitboard enemyPieces = isWhite ? blackPieces : whitePieces;

    const Bitboard ownPawns    = isWhite ? whitePawns   : blackPawns;
    const Bitboard ownKnights  = isWhite ? whiteKnights : blackKnights;
    const Bitboard ownBishops  = isWhite ? whiteBishops : blackBishops;
    const Bitboard ownRooks    = isWhite ? whiteRooks   : blackRooks;
    const Bitboard ownQueens   = isWhite ? whiteQueens  : blackQueens;
    const Bitboard ownKingBB   = isWhite ? whiteKing    : blackKing;

    const Bitboard enemyKnights = isWhite ? blackKnights : whiteKnights;
    const Bitboard enemyBishops = isWhite ? blackBishops : whiteBishops;
    const Bitboard enemyRooks   = isWhite ? blackRooks   : whiteRooks;
    const Bitboard enemyQueens  = isWhite ? blackQueens  : whiteQueens;
    const Bitboard enemyPawns   = isWhite ? blackPawns   : whitePawns;

    const int kingSquare = getLSB(ownKingBB);

    const Piece ownKnightPiece = isWhite ? WKNIGHT : BKNIGHT;
    const Piece ownBishopPiece = isWhite ? WBISHOP : BBISHOP;
    const Piece ownRookPiece   = isWhite ? WROOK   : BROOK;
    const Piece ownQueenPiece  = isWhite ? WQUEEN  : BQUEEN;
    const Piece ownKingPiece   = isWhite ? WKING   : BKING;
    const Piece ownPawnPiece   = isWhite ? WPAWN   : BPAWN;

    // find every enemy piece currently giving check to our king.
    Bitboard checkers = 0ULL;
    checkers |= Knight::attacks(kingSquare) & enemyKnights;
    checkers |= (isWhite ? Pawn::attacks(kingSquare, WHITE) : Pawn::attacks(kingSquare, BLACK)) & enemyPawns;
    checkers |= Bishop::attacks(kingSquare, occupied) & (enemyBishops | enemyQueens);
    checkers |= Rook::attacks(kingSquare, occupied) & (enemyRooks | enemyQueens);

    const int numCheckers = popcount(checkers);

    // Pin detection: for each enemy sliding piece that *would* attack the
    // king if one specific one of our own pieces weren't in the way, that
    // piece is pinned and may only move along the pin ray (still allowing
    // captures of the pinner, or blocking along the same line).
    Bitboard pinned = 0ULL;
    Bitboard pinRay[64];

    {
        // "X-ray" occupancy: remove our own pieces so sliding attacks pass
        // through them, revealing which of our pieces sit on a line between
        // the king and a potential pinning slider.
        const Bitboard xrayOcc = occupied & ~ownPieces;
        const Bitboard potentialPinners =
            (Bishop::attacks(kingSquare, xrayOcc) & (enemyBishops | enemyQueens)) |
            (Rook::attacks(kingSquare, xrayOcc)   & (enemyRooks   | enemyQueens));

        Bitboard pp = potentialPinners;
        while (pp)
        {
            const int pinnerSquare = getLSB(pp);
            pp &= pp - 1;

            const Bitboard between = squaresBetween(kingSquare, pinnerSquare);
            const Bitboard blockers = between & ownPieces;

            if (popcount(blockers) == 1)
            {
                const int pinnedSquare = getLSB(blockers);
                pinned |= (1ULL << pinnedSquare);
                pinRay[pinnedSquare] = between | (1ULL << pinnerSquare);
            }
        }
    }

    // When in single check, every move (except king moves) must either
    // capture the checker or block the ray between it and the king.
    // blockMask expresses "squares that resolve the check" as a bitboard.
    Bitboard blockMask = ~0ULL;
    if (numCheckers == 1)
    {
        const int checkerSquare = getLSB(checkers);
        blockMask = squaresBetween(kingSquare, checkerSquare) | checkers;
    }

    auto addMoves = [&](int from, Bitboard destinations, Piece piece)
    {
        while (destinations)
        {
            const int to = getLSB(destinations);
            destinations &= destinations - 1;
            result.push(from, to, piece, getPiece(to), EMPTY, false, false);
        }
    };

    // In double check, only king moves are legal - everything below this
    // guard is skipped entirely in that case.
    if (numCheckers < 2)
    {
        Bitboard bb = ownKnights;
        while (bb)
        {
            const int from = getLSB(bb);
            bb &= bb - 1;
            // A pinned knight can never move without exposing the king
            // (it can't stay on the pin ray), so it's simply immobile.
            if (pinned & (1ULL << from))
                continue;
            addMoves(from, Knight::attacks(from) & ~ownPieces & blockMask, ownKnightPiece);
        }

        bb = ownBishops;
        while (bb)
        {
            const int from = getLSB(bb);
            bb &= bb - 1;
            Bitboard dest = Bishop::attacks(from, occupied) & ~ownPieces & blockMask;
            if (pinned & (1ULL << from)) dest &= pinRay[from]; // restrict to the pin line
            addMoves(from, dest, ownBishopPiece);
        }

        bb = ownRooks;
        while (bb)
        {
            const int from = getLSB(bb);
            bb &= bb - 1;
            Bitboard dest = Rook::attacks(from, occupied) & ~ownPieces & blockMask;
            if (pinned & (1ULL << from)) dest &= pinRay[from];
            addMoves(from, dest, ownRookPiece);
        }

        bb = ownQueens;
        while (bb)
        {
            const int from = getLSB(bb);
            bb &= bb - 1;
            Bitboard dest = Queen::attacks(from, occupied) & ~ownPieces & blockMask;
            if (pinned & (1ULL << from)) dest &= pinRay[from];
            addMoves(from, dest, ownQueenPiece);
        }

        const Piece promoQueen  = isWhite ? WQUEEN  : BQUEEN;
        const Piece promoRook   = isWhite ? WROOK   : BROOK;
        const Piece promoBishop = isWhite ? WBISHOP : BBISHOP;
        const Piece promoKnight = isWhite ? WKNIGHT : BKNIGHT;

        bb = ownPawns;
        while (bb)
        {
            const int from = getLSB(bb);
            bb &= bb - 1;

            const bool isPinned = pinned & (1ULL << from);
            const Bitboard allowedMask = isPinned ? pinRay[from] : ~0ULL;

            const int rank = from / 8;
            const int singlePush = isWhite ? from + 8 : from - 8;
            const bool onStartRank = isWhite ? (rank == 1) : (rank == 6);
            const bool willPromote = isWhite ? (rank == 6) : (rank == 1);

            auto addPawnMove = [&](int to, Piece captured, bool isEP)
            {
                if (willPromote)
                {
                    result.push(from, to, ownPawnPiece, captured, promoQueen,  false, false);
                    result.push(from, to, ownPawnPiece, captured, promoRook,   false, false);
                    result.push(from, to, ownPawnPiece, captured, promoBishop, false, false);
                    result.push(from, to, ownPawnPiece, captured, promoKnight, false, false);
                }
                else
                {
                    result.push(from, to, ownPawnPiece, captured, EMPTY, isEP, false);
                }
            };

            if (getBit(empty, singlePush))
            {
                if ((allowedMask & (1ULL << singlePush)) && (blockMask & (1ULL << singlePush)))
                    addPawnMove(singlePush, EMPTY, false);

                if (onStartRank)
                {
                    const int doublePush = isWhite ? from + 16 : from - 16;
                    if (getBit(empty, doublePush) &&
                        (allowedMask & (1ULL << doublePush)) && (blockMask & (1ULL << doublePush)))
                        addPawnMove(doublePush, EMPTY, false);
                }
            }

            const Bitboard attackTargets = Pawn::attacks(from, isWhite ? WHITE : BLACK);

            Bitboard captures = attackTargets & enemyPieces & allowedMask & blockMask;
            while (captures)
            {
                const int to = getLSB(captures);
                captures &= captures - 1;
                addPawnMove(to, getPiece(to), false);
            }

            // En passant is special-cased fully (rather than folded into the
            // pin/block masks above) because the "captured" pawn sits on a
            // different square than "to", so it needs its own legality check
            // via wouldLeaveKingInCheck below.
            if (enPassantSquare != -1 &&
                (attackTargets & (1ULL << enPassantSquare)) &&
                (allowedMask & (1ULL << enPassantSquare)))
            {
                const int epCapturedSquare = isWhite ? (enPassantSquare - 8) : (enPassantSquare + 8);

                const bool resolvesCheck =
                    (numCheckers == 0) ||
                    ((blockMask & (1ULL << enPassantSquare)) != 0) ||
                    (getLSB(checkers) == epCapturedSquare);

                if (resolvesCheck)
                {
                    const Piece capturedPawn = isWhite ? BPAWN : WPAWN;
                    const Move epMove{ from, enPassantSquare, ownPawnPiece, capturedPawn, EMPTY, true, false };

                    // Handles the rare "discovered check via en passant"
                    // case (removing both pawns from the rank exposes the
                    // king to a rook/queen along that rank) that the pin
                    // detection above doesn't cover.
                    if (!wouldLeaveKingInCheck(epMove, side))
                        result.push(epMove);
                }
            }
        }
    }

    // King moves: always generated (even in double check), and always
    // legality-checked directly against attacks with the king itself
    // removed from occupancy (so it can't "hide behind itself" from a
    // slider along its escape direction).
    {
        const Bitboard kingOffOcc = occupied & ~ownKingBB;
        Bitboard dest = King::attacks(kingSquare) & ~ownPieces;
        while (dest)
        {
            const int to = getLSB(dest);
            dest &= dest - 1;
            if (!isSquareAttackedWithOccupancy(to, enemySide, kingOffOcc))
                result.push(kingSquare, to, ownKingPiece, getPiece(to), EMPTY, false, false);
        }
    }

    // Castling is only legal when not currently in check.
    if (numCheckers == 0)
    {
        if (isWhite)
        {
            if (whiteKingsideCastle &&
                !getBit(occupied, 5) && !getBit(occupied, 6) &&
                !isSquareAttacked(4, BLACK) && !isSquareAttacked(5, BLACK) && !isSquareAttacked(6, BLACK))
            {
                result.push(4, 6, WKING, EMPTY, EMPTY, false, true);
            }
            if (whiteQueensideCastle &&
                !getBit(occupied, 1) && !getBit(occupied, 2) && !getBit(occupied, 3) &&
                !isSquareAttacked(4, BLACK) && !isSquareAttacked(3, BLACK) && !isSquareAttacked(2, BLACK))
            {
                result.push(4, 2, WKING, EMPTY, EMPTY, false, true);
            }
        }
        else
        {
            if (blackKingsideCastle &&
                !getBit(occupied, 61) && !getBit(occupied, 62) &&
                !isSquareAttacked(60, WHITE) && !isSquareAttacked(61, WHITE) && !isSquareAttacked(62, WHITE))
            {
                result.push(60, 62, BKING, EMPTY, EMPTY, false, true);
            }
            if (blackQueensideCastle &&
                !getBit(occupied, 57) && !getBit(occupied, 58) && !getBit(occupied, 59) &&
                !isSquareAttacked(60, WHITE) && !isSquareAttacked(59, WHITE) && !isSquareAttacked(58, WHITE))
            {
                result.push(60, 58, BKING, EMPTY, EMPTY, false, true);
            }
        }
    }

#ifdef ENGINE_PROFILE
    auto t1 = std::chrono::high_resolution_clock::now();
    timeGenMoves += std::chrono::duration<double>(t1 - t0).count();
    timeLegalityFilter += 0.0; 
#endif

    return result;
}

long long Board::perft(int depth)
{
    // standard perft (performance test / move-count verification): counts
    // leaf nodes at the given depth. Used to validate move generation
    // correctness against known perft results, not used during real search.
    if (depth == 0)
        return 1;

    const MoveList moves = generateLegalMoves();

    if (depth == 1)
        return static_cast<long long>(moves.size());

    long long nodes = 0;

    for (const Move& move : moves)
    {
        makeMove(move);
        nodes += perft(depth - 1);
        unmakeMove(move);
    }

    return nodes;
}

int Board::pieceValue(Piece piece)
{
    // Standard material values in centipawns, used for move-ordering (MVV-LVA
    // style scoring) and as the fallback evaluation when NNUE isn't loaded.
    if (piece == EMPTY)
        return 0;

    switch (static_cast<int>(piece) % 6)
    {
        case 0: return 100;   // pawn
        case 1: return 320;   // knight
        case 2: return 330;   // bishop
        case 3: return 500;   // rook
        case 4: return 900;   // queen
        case 5: return 20000; // king
        default: return 0;
    }
}

int Board::moveScore(const Move& move, int ply, const Move& ttMove) const
{
    // Move ordering priority (highest score searched first), roughly:
    // 1. TT best move from a previous search of this position
    // 2. Captures, scored MVV-LVA (favor capturing valuable pieces with
    //    cheap ones)
    // 3. Promotions
    // 4. Killer moves (quiet moves that caused a beta cutoff at this ply
    //    in a sibling branch)
    // 5. History heuristic (quiet moves that have historically caused
    //    cutoffs elsewhere in the tree)
    if (sameMove(move, ttMove))
        return 2'000'000;

    if (move.captured != EMPTY)
        return 1'000'000 + 10 * pieceValue(move.captured) - pieceValue(move.piece);

    if (move.promotion != EMPTY)
        return 900'000 + pieceValue(static_cast<Piece>(move.promotion)) * 10;

    if (ply >= 0 && ply < MAX_PLY)
    {
        if (sameMove(move, killerMoves[ply][0]))
            return 800'000;
        if (sameMove(move, killerMoves[ply][1]))
            return 799'000;
    }

    return historyHeuristic[move.from][move.to];
}

void Board::orderMoves(MoveList& moves, int ply, const Move& ttMove) const
{
    // Good move ordering is what makes alpha-beta pruning effective - trying
    // the best moves first lets later branches get pruned much sooner.
    std::sort(moves.begin(), moves.end(), [this, ply, &ttMove](const Move& a, const Move& b)
    {
        return moveScore(a, ply, ttMove) > moveScore(b, ply, ttMove);
    });
}

void Board::recordCutoff(const Move& move, int depth, int ply)
{
    // Called when a move causes a beta cutoff (fail-high). Only quiet moves
    // are tracked as killers/history - captures/promotions are already
    // prioritized by moveScore() above, so tracking them here would be
    // redundant.
    if (move.captured != EMPTY || move.promotion != EMPTY)
        return;

    if (ply >= 0 && ply < MAX_PLY && !sameMove(move, killerMoves[ply][0]))
    {
        killerMoves[ply][1] = killerMoves[ply][0];
        killerMoves[ply][0] = move;
    }

    // Weighted by depth^2 so cutoffs found deeper in the tree (more
    // "expensive" to discover) count for more than shallow ones.
    historyHeuristic[move.from][move.to] += depth * depth;
}

void Board::clearSearchHeuristics()
{
    // Killers/history are per-search-call state, not part of the permanent
    // position; reset at the start of each new search so stale data from a
    // previous, unrelated position doesn't bias move ordering.
    for (int ply = 0; ply < MAX_PLY; ply++)
    {
        killerMoves[ply][0] = noMove();
        killerMoves[ply][1] = noMove();
    }
    std::fill(&historyHeuristic[0][0], &historyHeuristic[0][0] + 64 * 64, 0);
}

bool Board::hasNonPawnMaterial(bool white) const
{
    // Used to guard null-move pruning: in pure king+pawn endgames, "passing"
    // your turn (null move) can give wildly wrong results (zugzwang), so
    // null-move pruning is disabled when the side to move has no pieces
    // other than pawns/king.
    return white
        ? (whiteKnights | whiteBishops | whiteRooks | whiteQueens) != 0ULL
        : (blackKnights | blackBishops | blackRooks | blackQueens) != 0ULL;
}

int Board::scoreToTT(int score, int ply)
{
    // Mate scores are stored "distance from root" in the TT (independent of
    // ply), then converted back to "distance from the current node" when
    // read - otherwise a mate score cached at one ply would report the
    // wrong mate distance when reused at a different ply.
    constexpr int MATE_THRESHOLD = MATE_VALUE - MAX_PLY;
    if (score >= MATE_THRESHOLD)  return score + ply;
    if (score <= -MATE_THRESHOLD) return score - ply;
    return score;
}

int Board::scoreFromTT(int score, int ply)
{
    constexpr int MATE_THRESHOLD = MATE_VALUE - MAX_PLY;
    if (score >= MATE_THRESHOLD)  return score - ply;
    if (score <= -MATE_THRESHOLD) return score + ply;
    return score;
}

void Board::makeNullMove()
{
    // "Passes" the turn without moving a piece - used only for null-move
    // pruning. Clears en passant (a null move can't preserve an EP right)
    // and flips side to move; no piece bitboards change.
    history.push_back({
        enPassantSquare,
        whiteKingsideCastle, whiteQueensideCastle,
        blackKingsideCastle, blackQueensideCastle,
        zobristKey
    });

    if (enPassantSquare != -1)
        zobristKey ^= Zobrist::enPassantFileKeys[enPassantSquare % 8];
    enPassantSquare = -1;

    zobristKey ^= Zobrist::sideKey;
    whiteToMove = !whiteToMove;
    sideToMove = whiteToMove ? WHITE : BLACK;
}

void Board::unmakeNullMove()
{
    const UndoState prev = history.back();
    history.pop_back();

    enPassantSquare      = prev.enPassantSquare;
    whiteKingsideCastle  = prev.whiteKingsideCastle;
    whiteQueensideCastle = prev.whiteQueensideCastle;
    blackKingsideCastle  = prev.blackKingsideCastle;
    blackQueensideCastle = prev.blackQueensideCastle;
    zobristKey           = prev.zobristKey;

    whiteToMove = !whiteToMove;
    sideToMove = whiteToMove ? WHITE : BLACK;
}

void Board::refreshAccumulators()
{
    // Full (non-incremental) rebuild of both perspectives' NNUE accumulators
    // from the current board state. Expensive relative to the incremental
    // update path below, so only called when incremental updates aren't
    // valid (position load, or after a king move - see applyAccumulatorUpdate).
    if (!NNUE::isLoaded())
    {
        std::fill(accumWhite, accumWhite + NNUE::FT_DIM, 0);
        std::fill(accumBlack, accumBlack + NNUE::FT_DIM, 0);
        return;
    }

    NNUE::refreshAccumulatorInt(*this, true, accumWhite);
    NNUE::refreshAccumulatorInt(*this, false, accumBlack);
}

void Board::applyAccumulatorUpdate(const Move& move)
{
    // Incremental NNUE update: rather than recomputing the full accumulator
    // after every move (expensive), only add/remove the features that
    // actually changed. HalfKP features are relative to each side's king
    // square, so if a king moves, its own-perspective accumulator can't be
    // patched incrementally and gets a full refresh instead (see makeMove).
    const bool movingIsKing = (move.piece == WKING || move.piece == BKING);
    const bool movingIsWhite = isWhitePiece(move.piece);

    for (int side = 0; side < 2; side++)
    {
        const bool perspectiveIsWhite = (side == 0);
        int32_t* acc = perspectiveIsWhite ? accumWhite : accumBlack;

        // Only skip patching this perspective's accumulator if it's *this
        // perspective's own* king that's moving (that case is handled by a
        // full refresh elsewhere) - the opposite king's HalfKP features are
        // still updated normally here.
        const bool ownKingMoving = movingIsKing && (movingIsWhite == perspectiveIsWhite);

        const Bitboard kingBB = perspectiveIsWhite ? whiteKing : blackKing;
        const int kingSquare = getLSB(kingBB);
        const int kingOriented = NNUE::orientSquare(perspectiveIsWhite, kingSquare);

        if (move.isCastling && !ownKingMoving)
        {
            int rookFrom, rookTo; Piece rookPiece;
            castlingRookSquares(move, rookFrom, rookTo, rookPiece);

            const int idxRookOld = NNUE::computeFeatureIndex(perspectiveIsWhite, rookFrom, rookPiece, kingOriented);
            NNUE::removeFeatureInt(idxRookOld, acc);

            const int idxRookNew = NNUE::computeFeatureIndex(perspectiveIsWhite, rookTo, rookPiece, kingOriented);
            NNUE::addFeatureInt(idxRookNew, acc);
        }
        if (move.captured != EMPTY && !ownKingMoving)
        {
            const int captureSquare = move.isEnPassant ? enPassantCapturedSquare(move) : move.to;
            const int idxCap = NNUE::computeFeatureIndex(perspectiveIsWhite, captureSquare, move.captured, kingOriented);
            NNUE::removeFeatureInt(idxCap, acc);
        }

        if (!movingIsKing)
        {
            const int idxFrom = NNUE::computeFeatureIndex(perspectiveIsWhite, move.from, move.piece, kingOriented);
            NNUE::removeFeatureInt(idxFrom, acc);

            const Piece finalPiece = (move.promotion != EMPTY) ? static_cast<Piece>(move.promotion) : move.piece;
            const int idxTo = NNUE::computeFeatureIndex(perspectiveIsWhite, move.to, finalPiece, kingOriented);
            NNUE::addFeatureInt(idxTo, acc);
        }
    }
}

void Board::undoAccumulatorUpdate(const Move& move)
{
    // Mirror image of applyAccumulatorUpdate, run in reverse order so the
    // accumulator is restored to exactly its pre-move state.
    const bool movingIsKing = (move.piece == WKING || move.piece == BKING);
    const bool movingIsWhite = isWhitePiece(move.piece);

    for (int side = 0; side < 2; side++)
    {
        const bool perspectiveIsWhite = (side == 0);
        int32_t* acc = perspectiveIsWhite ? accumWhite : accumBlack;

        const bool ownKingMoving = movingIsKing && (movingIsWhite == perspectiveIsWhite);

        const Bitboard kingBB = perspectiveIsWhite ? whiteKing : blackKing;
        const int kingSquare = getLSB(kingBB);
        const int kingOriented = NNUE::orientSquare(perspectiveIsWhite, kingSquare);

        if (move.isCastling && !ownKingMoving)
        {
            int rookFrom, rookTo; Piece rookPiece;
            castlingRookSquares(move, rookFrom, rookTo, rookPiece);

            const int idxRookNew = NNUE::computeFeatureIndex(perspectiveIsWhite, rookTo, rookPiece, kingOriented);
            NNUE::removeFeatureInt(idxRookNew, acc);

            const int idxRookOld = NNUE::computeFeatureIndex(perspectiveIsWhite, rookFrom, rookPiece, kingOriented);
            NNUE::addFeatureInt(idxRookOld, acc);
        }

        if (!movingIsKing)
        {
            const Piece finalPiece = (move.promotion != EMPTY) ? static_cast<Piece>(move.promotion) : move.piece;
            const int idxTo = NNUE::computeFeatureIndex(perspectiveIsWhite, move.to, finalPiece, kingOriented);
            NNUE::removeFeatureInt(idxTo, acc);
        }

        if (move.captured != EMPTY && !ownKingMoving)
        {
            const int captureSquare = move.isEnPassant ? enPassantCapturedSquare(move) : move.to;
            const int idxCap = NNUE::computeFeatureIndex(perspectiveIsWhite, captureSquare, move.captured, kingOriented);
            NNUE::addFeatureInt(idxCap, acc);
        }

        if (!movingIsKing)
        {
            const int idxFrom = NNUE::computeFeatureIndex(perspectiveIsWhite, move.from, move.piece, kingOriented);
            NNUE::addFeatureInt(idxFrom, acc);
        }
    }
}

int Board::evaluateNNUE() const
{
    if (!NNUE::isLoaded())
    {
        // Material-only fallback so the engine stays functional (never
        // crashes / never returns nothing) if the NNUE network file is
        // missing or fails to load. Much weaker than the real network,
        // but keeps search well-defined end to end.
        int score = 0;
        score += popcount(whitePawns)   * pieceValue(WPAWN);
        score += popcount(whiteKnights) * pieceValue(WKNIGHT);
        score += popcount(whiteBishops) * pieceValue(WBISHOP);
        score += popcount(whiteRooks)   * pieceValue(WROOK);
        score += popcount(whiteQueens)  * pieceValue(WQUEEN);

        score -= popcount(blackPawns)   * pieceValue(BPAWN);
        score -= popcount(blackKnights) * pieceValue(BKNIGHT);
        score -= popcount(blackBishops) * pieceValue(BBISHOP);
        score -= popcount(blackRooks)   * pieceValue(BROOK);
        score -= popcount(blackQueens)  * pieceValue(BQUEEN);

        return whiteToMove ? score : -score;
    }

    // Evaluation is always from the perspective of the side to move
    // ("us" positive is good for us), which is why the accumulator pointers
    // are swapped based on whiteToMove rather than always us=white.
    const int32_t* us   = whiteToMove ? accumWhite : accumBlack;
    const int32_t* them = whiteToMove ? accumBlack : accumWhite;
    return NNUE::evaluateFromAccumulatorsInt(us, them);
}

void Board::makeMove(const Move& move, bool updateNNUE)
{
    const int from = move.from;
    const int to   = move.to;

    const Piece movingPiece = move.piece;
    const Piece capturedPiece = getPiece(to);

    const int oldEP = enPassantSquare;
    const int oldCastlingIndex = Zobrist::castlingIndex(
        whiteKingsideCastle, whiteQueensideCastle,
        blackKingsideCastle, blackQueensideCastle);
    history.push_back({
        enPassantSquare,
        whiteKingsideCastle, whiteQueensideCastle,
        blackKingsideCastle, blackQueensideCastle,
        zobristKey
    });

    enPassantSquare = -1;

    // NNUE accumulator is updated based on the *move itself* (incrementally),
    // using board state as it still is *before* any bitboards below change.
    if (NNUE::isLoaded() && updateNNUE)
        applyAccumulatorUpdate(move);

    const bool movingIsWhite = isWhitePiece(movingPiece);
    const Bitboard fromBit = 1ULL << from;
    const Bitboard toBit   = 1ULL << to;

    zobristKey ^= Zobrist::pieceKeys[movingPiece][from];

    if (capturedPiece != EMPTY)
    {
        bitboardFor(capturedPiece) &= ~toBit;
        if (isWhitePiece(capturedPiece))
            whitePieces &= ~toBit;
        else
            blackPieces &= ~toBit;

        zobristKey ^= Zobrist::pieceKeys[capturedPiece][to];
    }

    // En passant capture: the captured pawn isn't on the destination square,
    // it's the square the mover jumped over/past.
    if ((movingPiece == WPAWN || movingPiece == BPAWN) &&
        to == oldEP &&
        capturedPiece == EMPTY)
    {
        if (movingPiece == WPAWN)
        {
            const Bitboard epBit = 1ULL << (to - 8);
            blackPawns  &= ~epBit;
            blackPieces &= ~epBit;
            zobristKey ^= Zobrist::pieceKeys[BPAWN][to - 8];
        }
        else
        {
            const Bitboard epBit = 1ULL << (to + 8);
            whitePawns  &= ~epBit;
            whitePieces &= ~epBit;
            zobristKey ^= Zobrist::pieceKeys[WPAWN][to + 8];
        }
    }

    bitboardFor(movingPiece) ^= fromBit | toBit;
    if (movingIsWhite)
        whitePieces ^= fromBit | toBit;
    else
        blackPieces ^= fromBit | toBit;

    zobristKey ^= Zobrist::pieceKeys[movingPiece][to];

    if (move.promotion != EMPTY)
    {
        // The pawn was already placed on `to` above as a pawn; swap it out
        // for the promoted piece.
        bitboardFor(movingPiece) &= ~toBit;
        bitboardFor(static_cast<Piece>(move.promotion)) |= toBit;

        zobristKey ^= Zobrist::pieceKeys[movingPiece][to];
        zobristKey ^= Zobrist::pieceKeys[move.promotion][to];
    }

    // Move the rook as part of castling (the king's own move was already
    // handled by the generic piece-move logic above).
    if (movingPiece == WKING)
    {
        if (from == 4 && to == 6)
        {
            whiteRooks  ^= (1ULL << 7) | (1ULL << 5);
            whitePieces ^= (1ULL << 7) | (1ULL << 5);
            zobristKey ^= Zobrist::pieceKeys[WROOK][7] ^ Zobrist::pieceKeys[WROOK][5];
        }
        else if (from == 4 && to == 2)
        {
            whiteRooks  ^= (1ULL << 0) | (1ULL << 3);
            whitePieces ^= (1ULL << 0) | (1ULL << 3);
            zobristKey ^= Zobrist::pieceKeys[WROOK][0] ^ Zobrist::pieceKeys[WROOK][3];
        }
    }
    else if (movingPiece == BKING)
    {
        if (from == 60 && to == 62)
        {
            blackRooks  ^= (1ULL << 63) | (1ULL << 61);
            blackPieces ^= (1ULL << 63) | (1ULL << 61);
            zobristKey ^= Zobrist::pieceKeys[BROOK][63] ^ Zobrist::pieceKeys[BROOK][61];
        }
        else if (from == 60 && to == 58)
        {
            blackRooks  ^= (1ULL << 56) | (1ULL << 59);
            blackPieces ^= (1ULL << 56) | (1ULL << 59);
            zobristKey ^= Zobrist::pieceKeys[BROOK][56] ^ Zobrist::pieceKeys[BROOK][59];
        }
    }

    // Update castling rights: moving the king or a rook off its home
    // square permanently forfeits the relevant right(s).
    if (movingPiece == WKING)
    {
        whiteKingsideCastle  = false;
        whiteQueensideCastle = false;
    }
    else if (movingPiece == BKING)
    {
        blackKingsideCastle  = false;
        blackQueensideCastle = false;
    }
    else if (movingPiece == WROOK)
    {
        if (from == 7) whiteKingsideCastle  = false;
        if (from == 0) whiteQueensideCastle = false;
    }
    else if (movingPiece == BROOK)
    {
        if (from == 63) blackKingsideCastle  = false;
        if (from == 56) blackQueensideCastle = false;
    }

    // Capturing an enemy rook on its home square also forfeits that side's
    // right to castle on that wing, even though it's not the rook's own move.
    if (capturedPiece == WROOK)
    {
        if (to == 7) whiteKingsideCastle  = false;
        if (to == 0) whiteQueensideCastle = false;
    }
    else if (capturedPiece == BROOK)
    {
        if (to == 63) blackKingsideCastle  = false;
        if (to == 56) blackQueensideCastle = false;
    }

    // A pawn double-push creates a fresh en passant target square for the
    // opponent's immediately-following move only.
    if (movingPiece == WPAWN && to == from + 16)
        enPassantSquare = from + 8;
    else if (movingPiece == BPAWN && to == from - 16)
        enPassantSquare = from - 8;

    occupied = whitePieces | blackPieces;
    empty    = ~occupied;

    // HalfKP features are relative to king square, so if this move moved a
    // king, that side's own accumulator can't be patched incrementally -
    // it needs a full recompute against the king's new square.
    if (NNUE::isLoaded() && updateNNUE && (movingPiece == WKING || movingPiece == BKING))
        NNUE::refreshAccumulatorInt(*this, movingPiece == WKING, movingPiece == WKING ? accumWhite : accumBlack);

    const int newCastlingIndex = Zobrist::castlingIndex(
        whiteKingsideCastle, whiteQueensideCastle,
        blackKingsideCastle, blackQueensideCastle);
    if (oldCastlingIndex != newCastlingIndex)
        zobristKey ^= Zobrist::castlingKeys[oldCastlingIndex] ^ Zobrist::castlingKeys[newCastlingIndex];

    if (oldEP != -1)
        zobristKey ^= Zobrist::enPassantFileKeys[oldEP % 8];
    if (enPassantSquare != -1)
        zobristKey ^= Zobrist::enPassantFileKeys[enPassantSquare % 8];

    zobristKey ^= Zobrist::sideKey;

    whiteToMove = !whiteToMove;
    sideToMove = whiteToMove ? WHITE : BLACK;
}

void Board::unmakeMove(const Move& move, bool updateNNUE)
{
    // Reverses makeMove exactly, restoring bitboards, castling rights, en
    // passant square, and the Zobrist key from the saved history entry
    // (rather than recomputing the key from scratch).
    const int from = move.from;
    const int to   = move.to;
    const Piece movingPiece = move.piece;
    const bool movingIsWhite = isWhitePiece(movingPiece);

    const Bitboard fromBit = 1ULL << from;
    const Bitboard toBit   = 1ULL << to;

    // NNUE accumulator must be unwound first, while the board still
    // reflects the *post-move* state that applyAccumulatorUpdate expects.
    if (NNUE::isLoaded() && updateNNUE)
        undoAccumulatorUpdate(move);

    const UndoState prev = history.back();
    history.pop_back();

    enPassantSquare      = prev.enPassantSquare;
    whiteKingsideCastle  = prev.whiteKingsideCastle;
    whiteQueensideCastle = prev.whiteQueensideCastle;
    blackKingsideCastle  = prev.blackKingsideCastle;
    blackQueensideCastle = prev.blackQueensideCastle;
    zobristKey            = prev.zobristKey;

    if (movingPiece == WKING)
    {
        if (from == 4 && to == 6)
        {
            whiteRooks  ^= (1ULL << 7) | (1ULL << 5);
            whitePieces ^= (1ULL << 7) | (1ULL << 5);
        }
        else if (from == 4 && to == 2)
        {
            whiteRooks  ^= (1ULL << 0) | (1ULL << 3);
            whitePieces ^= (1ULL << 0) | (1ULL << 3);
        }
    }
    else if (movingPiece == BKING)
    {
        if (from == 60 && to == 62)
        {
            blackRooks  ^= (1ULL << 63) | (1ULL << 61);
            blackPieces ^= (1ULL << 63) | (1ULL << 61);
        }
        else if (from == 60 && to == 58)
        {
            blackRooks  ^= (1ULL << 56) | (1ULL << 59);
            blackPieces ^= (1ULL << 56) | (1ULL << 59);
        }
    }

    if (move.promotion != EMPTY)
    {
        bitboardFor(static_cast<Piece>(move.promotion)) &= ~toBit;
        bitboardFor(movingPiece) |= fromBit;
    }
    else
    {
        bitboardFor(movingPiece) ^= fromBit | toBit;
    }

    if (movingIsWhite)
        whitePieces ^= fromBit | toBit;
    else
        blackPieces ^= fromBit | toBit;

    if (move.captured != EMPTY)
    {
        if (move.isEnPassant)
        {
            if (movingPiece == WPAWN)
            {
                const Bitboard epBit = 1ULL << (to - 8);
                blackPawns  |= epBit;
                blackPieces |= epBit;
            }
            else
            {
                const Bitboard epBit = 1ULL << (to + 8);
                whitePawns  |= epBit;
                whitePieces |= epBit;
            }
        }
        else
        {
            bitboardFor(move.captured) |= toBit;
            if (isWhitePiece(move.captured))
                whitePieces |= toBit;
            else
                blackPieces |= toBit;
        }
    }

    occupied = whitePieces | blackPieces;
    empty    = ~occupied;

    // Same reasoning as in makeMove: a king move needs a full accumulator
    // refresh rather than incremental patching.
    if (NNUE::isLoaded() && updateNNUE && (movingPiece == WKING || movingPiece == BKING))
        NNUE::refreshAccumulatorInt(*this, movingPiece == WKING, movingPiece == WKING ? accumWhite : accumBlack);

    whiteToMove = !whiteToMove;
    sideToMove = whiteToMove ? WHITE : BLACK;
}

int Board::quiescence(int alpha, int beta, int ply)
{
    // Extends search past the nominal depth limit along "noisy" lines
    // (captures/promotions, or while in check) so the static evaluation is
    // never taken in the middle of an unresolved tactical exchange - this
    // avoids the classic "horizon effect" where the engine stops searching
    // right before losing material back.
    nodeCount++;

    const bool inCheck = isInCheck(whiteToMove);
    int standPat = 0;

    if (!inCheck)
    {
        // "Stand pat": the option to not make any further move here and
        // just accept the static evaluation, since a player is never forced
        // to make a losing capture.
        standPat = evaluateNNUE();

        if (standPat >= beta)
            return beta;
        if (standPat > alpha)
            alpha = standPat;
    }

    MoveList moves = generateLegalMoves();

    if (moves.empty())
        return inCheck ? -(MATE_VALUE - ply) : 0;

    const int killerPly = ply < MAX_PLY ? ply : MAX_PLY - 1;
    orderMoves(moves, killerPly, noMove());

    for (const Move& move : moves)
    {
        const bool isTactical = (move.captured != EMPTY) || (move.promotion != EMPTY);

        // While in check, all legal moves must be tried (there's no "stand
        // pat" option - you can't just not respond to check). Otherwise,
        // only tactical moves are explored.
        if (!inCheck && !isTactical)
            continue;

        makeMove(move);
        const int score = -quiescence(-beta, -alpha, ply + 1);
        unmakeMove(move);

        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

int Board::negamax(int depth, int alpha, int beta, int ply)
{
    // Core alpha-beta search (negamax form: each side maximizes its own
    // score, and a child's score is negated when returned to the parent,
    // since "good for opponent" = "bad for me").
    nodeCount++;

    if (depth <= 0)
    {
#ifdef ENGINE_PROFILE
        auto t0 = std::chrono::high_resolution_clock::now();
        int score = quiescence(alpha, beta, ply);
        auto t1 = std::chrono::high_resolution_clock::now();
        timeEval += std::chrono::duration<double>(t1 - t0).count();
        return score;
#else
        return quiescence(alpha, beta, ply);
#endif
    }

    const uint64_t key = zobristKey;
    const int killerPly = ply < MAX_PLY ? ply : MAX_PLY - 1;

    Move ttMove = noMove();

    if (TTEntry* entry = probeTT(key))
    {
        // Even if the stored entry isn't deep enough to trust its score
        // outright, its best move is still useful for move ordering.
        ttMove = entry->bestMove;

        if (entry->depth >= depth)
        {
            const int storedScore = scoreFromTT(entry->score, ply);

            if (entry->flag == TTFlag::EXACT)
                return storedScore;
            if (entry->flag == TTFlag::LOWERBOUND)
                alpha = std::max(alpha, storedScore);
            else if (entry->flag == TTFlag::UPPERBOUND)
                beta = std::min(beta, storedScore);

            if (alpha >= beta)
                return storedScore;
        }
    }
    const int alphaOriginal = alpha;

    constexpr int NULL_MOVE_MIN_DEPTH = 3;
    constexpr int NULL_MOVE_REDUCTION = 2;
    constexpr int MATE_THRESHOLD = MATE_VALUE - Board::MAX_PLY;

    // Null-move pruning: "what if I just passed my turn here - would the
    // opponent still not be able to beat beta?" If even doing nothing keeps
    // the score above beta, the real best move certainly will too, so this
    // branch can be cut short. Disabled near mate scores (unreliable),
    // while in check (passing isn't legal), and in king+pawn endgames
    // (zugzwang risk - see hasNonPawnMaterial).
    if (ply > 0 &&
        depth >= NULL_MOVE_MIN_DEPTH &&
        beta < MATE_THRESHOLD && beta > -MATE_THRESHOLD &&
        !isInCheck(whiteToMove) &&
        hasNonPawnMaterial(whiteToMove))
    {
        makeNullMove();
        const int nullScore = -negamax(depth - 1 - NULL_MOVE_REDUCTION, -beta, -beta + 1, ply + 1);
        unmakeNullMove();

        if (nullScore >= beta)
            return beta;
    }

    MoveList moves = generateLegalMoves();
    orderMoves(moves, killerPly, ttMove);

    const bool inCheckAtNode = isInCheck(whiteToMove);

    if (moves.empty())
        return inCheckAtNode ? -(MATE_VALUE - ply) : 0; // no legal moves: checkmate or stalemate

    constexpr int LMR_MIN_DEPTH = 3;
    constexpr int LMR_FULL_SEARCH_MOVES = 4;

    int bestScore = -MATE_VALUE - 1;
    Move bestMove = moves[0];
    int moveIndex = 0;

    for (const Move& move : moves)
    {
        const bool isQuiet = (move.captured == EMPTY && move.promotion == EMPTY);
        const bool isKiller = killerPly < MAX_PLY &&
            (sameMove(move, killerMoves[killerPly][0]) || sameMove(move, killerMoves[killerPly][1]));

#ifdef ENGINE_PROFILE
        auto t0 = std::chrono::high_resolution_clock::now();
        makeMove(move);
        auto t1 = std::chrono::high_resolution_clock::now();
        timeMakeUnmake += std::chrono::duration<double>(t1 - t0).count();
#else
        makeMove(move);
#endif

        const bool givesCheck = isInCheck(whiteToMove);

        // Late Move Reduction: search "unpromising" quiet moves (late in
        // move order, not a killer, not a check, at sufficient depth) with
        // a reduced depth first. If it unexpectedly beats alpha, it's
        // re-searched at full depth to confirm - this saves a large amount
        // of work on moves that are usually not the best move anyway.
        const bool canReduce =
            !inCheckAtNode && isQuiet && !isKiller && !givesCheck &&
            depth >= LMR_MIN_DEPTH && moveIndex >= LMR_FULL_SEARCH_MOVES;

        int score;

        if (canReduce)
        {
            const int reduction = lmrReduction(depth, moveIndex);
            const int reducedDepth = std::max(depth - 1 - reduction, 0);

            // Narrow "null window" search: only asks "is this move at least
            // as good as alpha?" (cheaper than a full [-beta, -alpha]
            // search), used to cheaply confirm/reject the reduced-depth result.
            score = -negamax(reducedDepth, -alpha - 1, -alpha, ply + 1);

            if (score > alpha)
                score = -negamax(depth - 1, -beta, -alpha, ply + 1);
        }
        else
        {
            score = -negamax(depth - 1, -beta, -alpha, ply + 1);
        }

#ifdef ENGINE_PROFILE
        auto t2 = std::chrono::high_resolution_clock::now();
        unmakeMove(move);
        auto t3 = std::chrono::high_resolution_clock::now();
        timeMakeUnmake += std::chrono::duration<double>(t3 - t2).count();
#else
        unmakeMove(move);
#endif

        moveIndex++;

        if (score > bestScore)
        {
            bestScore = score;
            bestMove = move;
        }

        if (bestScore > alpha)
            alpha = bestScore;

        if (alpha >= beta)
        {
            // Beta cutoff (fail-high): this move is already too good, the
            // opponent won't allow this branch, so remaining sibling moves
            // don't need to be searched.
            recordCutoff(move, depth, killerPly);
            break;
        }
    }

    // Classify the result for TT storage: EXACT only if the true best score
    // fell strictly within (alpha, beta); otherwise it's a bound rather
    // than a precise value, since search was cut short.
    TTFlag flag;
    if (bestScore <= alphaOriginal)
        flag = TTFlag::UPPERBOUND;
    else if (bestScore >= beta)
        flag = TTFlag::LOWERBOUND;
    else
        flag = TTFlag::EXACT;

    storeTT(key, depth, scoreToTT(bestScore, ply), flag, bestMove);

    return bestScore;
}

int Board::evaluatePosition(int depth)
{
    // Convenience entry point that returns just the score of the current
    // position at a fixed depth (used for testing/debugging, not for
    // picking a move to play - see findBestMove for that).
    nodeCount = 0;
    clearSearchHeuristics();
    return negamax(depth, -MATE_VALUE - 1, MATE_VALUE + 1, 0);
}

Move Board::findBestMove(int depth)
{
    // Root search: like negamax, but tracks *which* move produced the best
    // score (negamax itself only returns a score, not a move) and stores it
    // in the TT so future/iterative-deepening searches can reuse it.
    nodeCount = 0;
    clearSearchHeuristics();

    MoveList moves = generateLegalMoves();

    if (moves.empty())
        return noMove();

    Move ttMove = noMove();
    if (TTEntry* entry = probeTT(zobristKey))
        ttMove = entry->bestMove;

    orderMoves(moves, 0, ttMove);

    Move bestMove = moves[0];
    int bestScore = -MATE_VALUE - 1;
    int alpha = -MATE_VALUE - 1;
    const int beta = MATE_VALUE + 1;

    for (const Move& move : moves)
    {
        nodeCount++;

        makeMove(move);
        int score = -negamax(depth - 1, -beta, -alpha, 1);
        unmakeMove(move);

        if (score > bestScore)
        {
            bestScore = score;
            bestMove = move;
        }

        if (bestScore > alpha)
            alpha = bestScore;
    }

    storeTT(zobristKey, depth, scoreToTT(bestScore, 0), TTFlag::EXACT, bestMove);

    return bestMove;
}

bool Board::wouldLeaveKingInCheck(const Move& move, int side)
{
    // "Try the move, check for check, then undo it" - a simpler (if slower)
    // legality check than the pin/block-mask logic in generateLegalMoves,
    // used only for the rare en passant discovered-check edge case where
    // that logic doesn't apply cleanly. Deliberately does NOT touch the
    // Zobrist key, castling rights, or NNUE accumulators, since it's meant
    // to be a throwaway simulation, not a real move.
    const int from = move.from;
    const int to   = move.to;
    const Piece movingPiece = move.piece;
    const Piece capturedPiece = move.captured;

    const Bitboard fromBit = 1ULL << from;
    const Bitboard toBit   = 1ULL << to;

    int epCapturedSquare = -1;
    if (capturedPiece != EMPTY)
    {
        if (move.isEnPassant)
        {
            epCapturedSquare = (movingPiece == WPAWN) ? (to - 8) : (to + 8);
            bitboardFor(capturedPiece) &= ~(1ULL << epCapturedSquare);
        }
        else
        {
            bitboardFor(capturedPiece) &= ~toBit;
        }
    }

    bitboardFor(movingPiece) &= ~fromBit;

    Piece placedPiece = movingPiece;
    if (move.promotion != EMPTY)
    {
        placedPiece = static_cast<Piece>(move.promotion);
        bitboardFor(placedPiece) |= toBit;
    }
    else
    {
        bitboardFor(movingPiece) |= toBit;
    }

    updateOccupancy();

    const bool inCheck = isInCheck(side == WHITE);

    // Undo the simulated move.
    if (move.promotion != EMPTY)
        bitboardFor(placedPiece) &= ~toBit;
    else
        bitboardFor(movingPiece) &= ~toBit;

    bitboardFor(movingPiece) |= fromBit;

    if (capturedPiece != EMPTY)
    {
        if (move.isEnPassant)
            bitboardFor(capturedPiece) |= (1ULL << epCapturedSquare);
        else
            bitboardFor(capturedPiece) |= toBit;
    }

    updateOccupancy();

    return inCheck;
}