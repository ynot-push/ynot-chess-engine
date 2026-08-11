#include <iostream>
#include "include/board.h"
#include "include/BitboardUtils.h"
#include "include/PieceType.h"
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
    blackPawns =((1ULL<<16)-1)<<8;
    setBit(whiteKing,4);   
    setBit(blackKing,60);
    setBit(blackQueens,59);
    setBit(whiteQueens,3);
    whiteBishops=setBit(whiteBishops,5)|setBit(whiteBishops,2);
    blackBishops=setBit(blackBishops,58)|setBit(blackBishops,61);
    whiteRooks=setBit(whiteRooks,0)|setBit(whiteRooks,7);
    blackRooks=setBit(blackRooks,56)|setBit(blackRooks,63);
    whiteKnights=setBit(whiteKnights,1)|setBit(whiteKnights,6);
    blackKnights=setBit(blackKnights,57)|setBit(blackKnights,62);
}   

void  Board::printBoard() const{
Piece piece=EMPTY;

for(int i=0;i<64;i++){
    if(getBit(whitePawns,i)){

    }
    else if (getBit(whitePawns,i))
    {
        /* code */
    }
    else if (getBit(blackPawns,i))
    {
        /* code */
    }
    else if (getBit(whiteKing,i))
    {
        /* code */
    }
    else if (getBit(blackKing,i))
    {
        /* code */
    }
    else if (getBit(whiteQueens,i))
    {
        /* code */
    }
    else if (getBit(blackQueens,i))
    {
        /* code */
    }
    else if (getBit(blackRooks,i))
    {
        /* code */
    }
    else if (getBit(whiteRooks,i))
    {
        /* code */
    }
    else if (getBit(whiteBishops,i))
    {
        /* code */
    }
    else if (getBit(blackBishops,i))
    {
        /* code */
    }
    else if (getBit(whiteKnights,i))
    {
        /* code */
    }
    else if (getBit(blackKnights,i))
    {
        /* code */
    }
    else{
        
    }

}
}

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
