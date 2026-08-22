#include <iostream>
#include "board.h"
#include "BitboardUtils.h"
#include "PieceType.h"
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
/*
void Board::makeMove(const Move& move) {

}

Piece Board::getPiece(int square) const{
Piece piece=EMPTY; 

}

std::vector<Move> generateAllMoves(){

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

void Board::loadFEN(const std::string& fen){

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