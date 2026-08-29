#include <iostream>
#include "board.h"
#include "BitboardUtils.h"
#include "PieceType.h"
#include <square.cpp>
#include <Pawn.cpp>
#include <Knight.cpp>
#include <Rook.cpp>
#include <Queen.cpp>
#include <King.cpp>
#include <iostream>
#include <bitset>
using namespace BitboardUtils;
Board::Board(){
    initialize();
    
}
void Board::initialize() {
    
    blackPawns=0;
    blackKnights=0;
    blackBishops=0;
    blackRooks=0;
    blackQueens=0;
    blackKing=0;
    whitePawns=0;
    whiteKnights=0;
    whiteBishops=0;
    whiteRooks=0;
    whiteQueens=0;
    whiteKing=0;
    whitePawns =((1ULL<<8)-1)<<8;
    blackPawns =((1ULL<<8)-1)<<48;
    whiteKing = setBit(whiteKing, 4);
    blackKing = setBit(blackKing, 60);

    blackQueens = setBit(blackQueens, 59);
    whiteQueens = setBit(whiteQueens, 3);
    whiteBishops=setBit(whiteBishops,5)|setBit(whiteBishops,2);
    blackBishops=setBit(blackBishops,58)|setBit(blackBishops,61);
    whiteRooks=setBit(whiteRooks,0)|setBit(whiteRooks,7);
    blackRooks=setBit(blackRooks,56)|setBit(blackRooks,63);
    whiteKnights=setBit(whiteKnights,1)|setBit(whiteKnights,6);
    blackKnights=setBit(blackKnights,57)|setBit(blackKnights,62);
    updateOccupancy();
}   

char pieceToChar(Piece piece)
{
    switch (piece)
    {
        case WPAWN: return 'P';
        case WKNIGHT: return 'N';
        case WBISHOP: return 'B';
        case WROOK: return 'R';
        case WQUEEN: return 'Q';
        case WKING: return 'K';
        case BPAWN: return 'p';
        case BKNIGHT: return 'n';
        case BBISHOP: return 'b';
        case BROOK: return 'r';
        case BQUEEN: return 'q';
        case BKING: return 'k';
        default: return '.';
    }
}
Piece charToPiece(char c){
 switch (c)
 {
        case 'P': return WPAWN;
        case  'N': return WKNIGHT;
        case  'B': return WBISHOP;
        case  'R': return WROOK;
        case  'Q': return  WQUEEN;
        case  'K': return WKING ;
        case  'p': return BPAWN;
        case  'n': return BKNIGHT;
        case  'b': return BBISHOP;
        case  'r': return BROOK;
        case  'q': return BQUEEN;
        case  'k': return BKING;
        default: return EMPTY;
 }
}



void  Board::printBoard() const{

for (int rank = 7; rank >= 0; rank--) {

    for (int file = 0; file < 8; file++) {

        int square = rank * 8 + file;
        std::cout << pieceToChar(getPiece(square)) << " ";
    }

    std::cout << '\n';
}
}

void Board::updateOccupancy(){
    whitePieces =
        whitePawns
        | whiteKnights
        | whiteBishops
        | whiteRooks
        | whiteQueens
        | whiteKing;
    blackPieces =
    blackPawns
    | blackKnights
    | blackBishops
    | blackRooks
    | blackQueens
    | blackKing;    
    occupied = whitePieces | blackPieces;
    empty    = ~occupied ;   
}

Piece Board::getPiece(int square) const {

    if (getBit(whitePawns, square))
        return WPAWN;

    else if (getBit(blackPawns, square))
        return BPAWN;

    else if (getBit(whiteKing, square))
        return WKING;

    else if (getBit(blackKing, square))
        return BKING;

    else if (getBit(whiteQueens, square))
        return WQUEEN;

    else if (getBit(blackQueens, square))
        return BQUEEN;

    else if (getBit(whiteRooks, square))
        return WROOK;

    else if (getBit(blackRooks, square))
        return BROOK;

    else if (getBit(whiteBishops, square))
        return WBISHOP;

    else if (getBit(blackBishops, square))
        return BBISHOP;

    else if (getBit(whiteKnights, square))
        return WKNIGHT;

    else if (getBit(blackKnights, square))
        return BKNIGHT;

    else
        return EMPTY;
}
Bitboard Board::getWhitePawns() const {
    return whitePawns;
}

Bitboard Board::getWhiteKnights() const {
    return whiteKnights;
}

Bitboard Board::getWhiteBishops() const {
    return whiteBishops;
}

Bitboard Board::getWhiteRooks() const {
    return whiteRooks;
}

Bitboard Board::getWhiteQueens() const {
    return whiteQueens;
}

Bitboard Board::getWhiteKing() const {
    return whiteKing;
}


Bitboard Board::getBlackPawns() const {
    return blackPawns;
}

Bitboard Board::getBlackKnights() const {
    return blackKnights;
}

Bitboard Board::getBlackBishops() const {
    return blackBishops;
}

Bitboard Board::getBlackRooks() const {
    return blackRooks;
}

Bitboard Board::getBlackQueens() const {
    return blackQueens;
}

Bitboard Board::getBlackKing() const {
    return blackKing;
}


Bitboard Board::getWhitePieces() const {
    return whitePieces;
}

Bitboard Board::getBlackPieces() const {
    return blackPieces;
}

Bitboard Board::getOccupied() const {
    return occupied;
}

Bitboard Board::getEmpty() const {
    return empty;
}

void Board::loadFEN(const std::string& fen)
{
    // =========================================================
    // RESET POSITION
    // =========================================================

    whitePawns   = 0ULL;
    whiteKnights = 0ULL;
    whiteBishops = 0ULL;
    whiteRooks   = 0ULL;
    whiteQueens  = 0ULL;
    whiteKing    = 0ULL;

    blackPawns   = 0ULL;
    blackKnights = 0ULL;
    blackBishops = 0ULL;
    blackRooks   = 0ULL;
    blackQueens  = 0ULL;
    blackKing    = 0ULL;

    whitePieces = 0ULL;
    blackPieces = 0ULL;
    occupied    = 0ULL;

    whiteToMove = false;

    enPassantSquare = -1;

    whiteKingsideCastle  = false;
    whiteQueensideCastle = false;
    blackKingsideCastle  = false;
    blackQueensideCastle = false;


    // =========================================================
    // BOARD
    // =========================================================

    int rank = 7;
    int file = 0;

    int fenIndex = 0;

    while (rank >= 0 && fenIndex < fen.length())
    {
        char c = fen[fenIndex];

        // End of board section
        if (c == ' ')
        {
            fenIndex++;
            break;
        }

        // New rank
        if (c == '/')
        {
            rank--;
            file = 0;
            fenIndex++;
            continue;
        }

        // Empty squares
        if (c >= '1' && c <= '8')
        {
            file += c - '0';
            fenIndex++;
            continue;
        }

        // Piece
        int square = rank * 8 + file;

        switch (c)
        {
            // White
            case 'P':
                whitePawns |= (1ULL << square);
                break;

            case 'N':
                whiteKnights |= (1ULL << square);
                break;

            case 'B':
                whiteBishops |= (1ULL << square);
                break;

            case 'R':
                whiteRooks |= (1ULL << square);
                break;

            case 'Q':
                whiteQueens |= (1ULL << square);
                break;

            case 'K':
                whiteKing |= (1ULL << square);
                break;


            // Black
            case 'p':
                blackPawns |= (1ULL << square);
                break;

            case 'n':
                blackKnights |= (1ULL << square);
                break;

            case 'b':
                blackBishops |= (1ULL << square);
                break;

            case 'r':
                blackRooks |= (1ULL << square);
                break;

            case 'q':
                blackQueens |= (1ULL << square);
                break;

            case 'k':
                blackKing |= (1ULL << square);
                break;
        }

        file++;
        fenIndex++;
    }


    // =========================================================
    // SIDE TO MOVE
    // =========================================================

    whiteToMove = fen[fenIndex] == 'w';

    fenIndex += 2;


    // =========================================================
    // CASTLING RIGHTS
    // =========================================================

    while (fenIndex < fen.length() && fen[fenIndex] != ' ')
    {
        switch (fen[fenIndex])
        {
            case 'K':
                whiteKingsideCastle = true;
                break;

            case 'Q':
                whiteQueensideCastle = true;
                break;

            case 'k':
                blackKingsideCastle = true;
                break;

            case 'q':
                blackQueensideCastle = true;
                break;
        }

        fenIndex++;
    }

    fenIndex++;


    // =========================================================
    // EN PASSANT
    // =========================================================

    if (fenIndex < fen.length() && fen[fenIndex] != '-')
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



    whitePieces =
        whitePawns   |
        whiteKnights |
        whiteBishops |
        whiteRooks   |
        whiteQueens  |
        whiteKing;

    blackPieces =
        blackPawns   |
        blackKnights |
        blackBishops |
        blackRooks   |
        blackQueens  |
        blackKing;

    occupied = whitePieces | blackPieces;
}
bool Board::isSquareAttacked(int square, int side){
    Bitboard pawns;
    Bitboard knights;
    Bitboard bishops;
    Bitboard rooks;
    Bitboard queens;
    Bitboard king; 
    if(side==WHITE){
        pawns   = whitePawns;
        knights = whiteKnights;
        bishops = whiteBishops;
        rooks   = whiteRooks;
        queens  = whiteQueens;
        king    = whiteKing;
    }
    else{
        pawns = blackPawns;
        knights = blackKnights;
        bishops = blackBishops;
        rooks   = blackRooks;
        queens  = blackQueens;
        king    = blackKing;
    }
    

if (KnightAttacks[square] &
    knights){
        return true;
    }

if (KingAttacks[square] &
    king){
        return true;
    }    
if (PawnAttacks[square] &
    queens){
        return true;
    }      
if (Bishop::attacks(square,occupied) &
        (bishops | queens))
        return true;

if (Rook::attacks(square, occupied) & (rooks | queens))
        return true;

return false;        
}

Bitboard  Board::generateAllMoves(int side){
    std::vector<Move> allMoves;

        while (whiteKnights)
        {
            int SourceSquare=std::getLsb(whiteKnights);
            Bitboard attacks= KnightAttacks[SourceSquare];
            attacks &= ~whitePieces;

            while (attacks){
                    int TargetSquare=std::getLsb(whiteKnights);
                    
                    allMoves.insert(allMoves.end(),SourceSquare.begin(),TargetSquare.end());
                    attacks|=attacks-1;

            }

            
        }
 
}



/*
void Board::makeMove(const Move& move) {

}

Piece Board::getPiece(int square) const{
Piece piece=EMPTY; 

}



std::vector<Move> generateLegalMoves(){

}

bool Board::isSquareAttacked(int square, bool byWhite) const{

}

bool Board::isInCheck(bool white) const{

}

long long Board::perft(int depth){

}

void Board::setPosition(
        const Piece position[64],
        bool whiteToMove,
        int epSquare,
        bool bqs,
        bool bks,
        bool wqs,
        bool wks
    ){

    }

bool Board::canWhiteCastleKingside() const{

}
bool Board::canWhiteCastleQueenside() const{

}
bool Board::canBlackCastleKingside() const{

}
bool Board::canBlackCastleQueenside() const{

}

int Board::getEnPassantSquare() const{

}
*/