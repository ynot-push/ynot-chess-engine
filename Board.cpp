#include <iostream>
#include "board.h"

Board::Board() {
    initialize();
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
void Board::initialize() {
int i;
for ( i=0;i<64;i++)
{
    squares[i] = EMPTY;
}
squares[0] = BROOK;
squares[1] = BKNIGHT;
squares[2] = BBISHOP;
squares[3] = BQUEEN;    
squares[4] = BKING;
squares[5] = BBISHOP;
squares[6] = BKNIGHT;
squares[7] = BROOK;
for( i=8;i<=15;i++)
{
    squares[i] = BPAWN;

}
for( i=48;i<=55;i++)
{
    squares[i] = WPAWN;
}
squares[56] = WROOK;
squares[57] = WKNIGHT;
squares[58] = WBISHOP;
squares[59] = WQUEEN;
squares[60] = WKING;
squares[61] = WBISHOP;
squares[62] = WKNIGHT;
squares[63] = WROOK;

}   
void Board::printBoard() const {
    for(int i=0; i<64; i++) {
        std::cout<<pieceToChar(squares[i])<<" ";
        if((i+1)%8==0)
        {
            std::cout<<std::endl;
        }
    }
}
void Board::makeMove(const Move& move){
    squares[move.to] = squares[move.from];
    squares[move.from] = EMPTY;
}
Piece Board::getPiece(int index) const{
    if(index < 0 || index >= 64) {
        return EMPTY; 
    }
    return squares[index];
}