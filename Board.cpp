#include <iostream>
#include "board.h"
#include "Pawn.h"
#include "Knight.h"
#include "King.h"
#include "Queen.h"
#include "Rook.h"
#include "Bishop.h"
#include "square.h"
#include "Piece.h"
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
WhiteToMove = true;
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
    
    Piece piece=getPiece(move.from);
    if (piece == WKING) {
    whiteKingsideCastle = false;
    whiteQueensideCastle = false;
    }

    if (piece == BKING) {
        blackKingsideCastle = false;
        blackQueensideCastle = false;
    }
    if(piece==BROOK && move.from== 7){
        blackKingsideCastle=false;
    }
    if(piece==BROOK && move.from== 0 ){
        blackQueensideCastle=false;
    }
    if(piece==WROOK && move.from== 63){
        whiteKingsideCastle=false;
    }
    if(piece == WROOK && move.from == 56){
    whiteQueensideCastle = false;
    }
    if(move.to==0 && getPiece(move.to)==BROOK){
        blackQueensideCastle=false;
    }
    if(move.to==56 && getPiece(move.to)==WROOK){
        whiteQueensideCastle=false;
    }
     if(move.to==63 && getPiece(move.to)==WROOK){
        whiteKingsideCastle=false;
    }
    if(move.to==7 && getPiece(move.to)==BROOK){
        blackKingsideCastle=false;
    }
    squares[move.to] = squares[move.from];
    squares[move.from] = EMPTY;
    WhiteToMove = !WhiteToMove;



}
Piece Board::getPiece(int index) const{
    if(index < 0 || index >= 64) {
        return EMPTY; 
    }
    return squares[index];
}
bool Board::canWhiteCastleKingside() const{
    return whiteKingsideCastle;
}
bool Board::canWhiteCastleQueenside() const{
        return whiteQueensideCastle;

    }
bool Board::canBlackCastleKingside() const{
         return blackKingsideCastle;
    }
bool  Board::canBlackCastleQueenside() const{
          return blackQueensideCastle;
    }
int Board ::getEnPassantSquare() const{
   return enPassantSquare;
}    
std::vector<Move> Board::generateAllMoves(){
    std::vector<Move> allMoves;
    for(int i=0;i<64;i++){
        Piece piece = getPiece(i);
        if(piece==EMPTY){
            continue;
        }
        if(WhiteToMove && isBlackPiece(piece)){
            continue;
        }
        if(!WhiteToMove && isWhitePiece(piece)) {
            continue;
        }
        std::vector<Move> pieceMoves;
        switch(piece){
            case WPAWN:
            case BPAWN:
                pieceMoves = generatePawnMoves(*this,i);
                break;
            case BBISHOP:
            case WBISHOP:
                 pieceMoves = generateBishopMoves(*this,i);
                 break;
            case WROOK:
            case BROOK: 
                 pieceMoves = generateRookMoves(*this,i);
                 break;
            case WQUEEN:
            case BQUEEN: 
                 pieceMoves = generateQueenMoves(*this,i); 
                 break;
            case WKING:
            case BKING: 
                 pieceMoves = generateKingMoves(*this,i);   
                 break;
            case WKNIGHT:
            case BKNIGHT: 
                 pieceMoves = generateKnightMoves(*this,i);              
                 break;
  
                          

        }
        allMoves.insert(allMoves.end(),pieceMoves.begin(),pieceMoves.end());
    }
    return allMoves;
}