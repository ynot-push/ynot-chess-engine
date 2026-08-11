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
Piece squares[64];

for(int i=0;i<64;i++){
    if(getBit(whitePawns,i)){
        squares[i]=WPAWN;
    }
    else if (getBit(blackPawns,i))
    {
        squares[i]=WPAWN;
    }
    else if (getBit(whiteKing,i))
    {
        squares[i]=WKING;
    }
    else if (getBit(blackKing,i))
    {
        squares[i]=BKING;
    }
    else if (getBit(whiteQueens,i))
    {
         squares[i]=WQUEEN;
    }
    else if (getBit(blackQueens,i))
    {
         squares[i]=BQUEEN;
    }
    else if (getBit(blackRooks,i))
    {
         squares[i]=BROOK;
    }
    else if (getBit(whiteRooks,i))
    {
        squares[i]=WROOK;
    }
    else if (getBit(whiteBishops,i))
    {
         squares[i]=WBISHOP;
    }
    else if (getBit(blackBishops,i))
    {
         squares[i]=BBISHOP;
    }
    else if (getBit(whiteKnights,i))
    {
         squares[i]=WKNIGHT;
    }
    else if (getBit(blackKnights,i))
    {
         squares[i]=BKNIGHT;
    }
    else{
         squares[i]=EMPTY;
    }

}
for(int i=0; i<64; i++) {
        std::cout<<pieceToChar(squares[i])<<" ";
        if((i+1)%8==0)
        {
            std::cout<<std::endl;
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
