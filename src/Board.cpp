#include <iostream>
#include "Board.h"
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
    int oldEnPassantSquare =enPassantSquare;
    enPassantSquare = -1;
    if ((piece == WPAWN || piece == BPAWN) &&
    move.to == oldEnPassantSquare &&
    getPiece(move.to) == EMPTY) {

    int direction = (piece == WPAWN) ? -1 : 1;
    int capturedPawnSquare = move.to - direction * 8;
    squares[capturedPawnSquare] = EMPTY;
}
    if(piece==WPAWN && move.to==move.from-16){
          enPassantSquare=move.from-8;
    }
      if(piece==BPAWN && move.to==move.from+16){
          enPassantSquare=move.from+8;
    }
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
    if (move.promotionPiece != EMPTY) {
        squares[move.to] = move.promotionPiece;
    }
    if(piece==WKING && move.to==move.from+2){
        squares[61] = squares[63];
        squares[63] = EMPTY;       
    }
    if (piece == WKING && move.to == move.from - 2) {
        squares[59] = squares[56];
        squares[56] = EMPTY;
    }
    if (piece == BKING && move.to == move.from + 2) {
        squares[5] = squares[7];
        squares[7] = EMPTY;
    }
    if (piece == BKING && move.to == move.from - 2) {
        squares[3] = squares[0];
        squares[0] = EMPTY;
    }
   
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
bool Board::whiteToMove() const{
    return WhiteToMove;
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
bool Board::isSquareAttacked(int square, bool byWhite) const {

    int row = getRow(square);
    int col = getColumn(square);

    // --------------------------------------------------
    // PAWNS
    // --------------------------------------------------

    if (byWhite) {
        // White pawns attack one row upward (-1)
        if (row < 7) {
            if (col > 0 && getPiece(square + 7) == WPAWN)
                return true;

            if (col < 7 && getPiece(square + 9) == WPAWN)
                return true;
        }
    }
    else {
        // Black pawns attack one row downward (+1)
        if (row > 0) {
            if (col > 0 && getPiece(square - 9) == BPAWN)
                return true;

            if (col < 7 && getPiece(square - 7) == BPAWN)
                return true;
        }
    }

    // --------------------------------------------------
    // KNIGHTS
    // --------------------------------------------------

    const int knightDirections[8] =
        {17, 15, 10, 6, -17, -15, -10, -6};

    Piece knight = byWhite ? WKNIGHT : BKNIGHT;

    for (int i = 0; i < 8; i++) {

        int target = square + knightDirections[i];

        if (!isValid(target))
            continue;

        int rowDiff = abs(getRow(square) - getRow(target));
        int colDiff = abs(getColumn(square) - getColumn(target));

        if ((rowDiff == 2 && colDiff == 1) ||
            (rowDiff == 1 && colDiff == 2)) {

            if (getPiece(target) == knight)
                return true;
        }
    }

    // --------------------------------------------------
    // BISHOPS / QUEENS
    // --------------------------------------------------

    const int diagonalDirections[4] =
        {9, 7, -9, -7};

    Piece bishop = byWhite ? WBISHOP : BBISHOP;
    Piece queen  = byWhite ? WQUEEN  : BQUEEN;

    for (int i = 0; i < 4; i++) {

        int current = square;

        while (true) {

            int previous = current;
            current += diagonalDirections[i];

            if (!isValid(current))
                break;

            // Prevent wrapping around the board
            if (abs(getColumn(previous) - getColumn(current)) != 1)
                break;

            Piece piece = getPiece(current);

            if (piece == EMPTY)
                continue;

            if (piece == bishop || piece == queen)
                return true;

            break;
        }
    }

    // --------------------------------------------------
    // ROOKS / QUEENS
    // --------------------------------------------------

    const int straightDirections[4] =
        {1, -1, 8, -8};

    Piece rook = byWhite ? WROOK : BROOK;

    for (int i = 0; i < 4; i++) {

        int current = square;

        while (true) {

            int previous = current;
            current += straightDirections[i];

            if (!isValid(current))
                break;

            // Prevent horizontal wrapping
            if ((straightDirections[i] == 1 ||
                 straightDirections[i] == -1) &&
                abs(getColumn(previous) - getColumn(current)) != 1) {
                break;
            }

            Piece piece = getPiece(current);

            if (piece == EMPTY)
                continue;

            if (piece == rook || piece == queen)
                return true;

            break;
        }
    }

    // --------------------------------------------------
    // KING
    // --------------------------------------------------

    Piece king = byWhite ? WKING : BKING;

    const int kingDirections[8] =
        {1, -1, 8, -8, 9, -9, 7, -7};

    for (int i = 0; i < 8; i++) {

        int target = square + kingDirections[i];

        if (!isValid(target))
            continue;

        int rowDiff = abs(getRow(square) - getRow(target));
        int colDiff = abs(getColumn(square) - getColumn(target));

        if (rowDiff <= 1 && colDiff <= 1) {

            if (getPiece(target) == king)
                return true;
        }
    }

    return false;
}
bool Board::isInCheck(bool white) const {

    Piece king = white ? WKING : BKING;

    for (int i = 0; i < 64; i++) {

        if (getPiece(i) == king) {
            return isSquareAttacked(i, !white);
        }
    }

    return false;
}
std::vector<Move> Board::generateLegalMoves() {
    std::vector<Move> legalMoves;

    std::vector<Move> pseudoLegalMoves = generateAllMoves();

    bool movingSideIsWhite = WhiteToMove;

    for (const Move& move : pseudoLegalMoves) {

        Board testBoard = *this;

        testBoard.makeMove(move);

        // If our king is still in check, the move is illegal
        if (!testBoard.isInCheck(movingSideIsWhite)) {
            legalMoves.push_back(move);
        }
    }

    return legalMoves;
}
long long Board::perft(int depth) {

    if (depth == 0) {
        return 1;
    }

    std::vector<Move> moves = generateLegalMoves();

    long long nodes = 0;

    for (const Move& move : moves) {

        Board child = *this;

        child.makeMove(move);

        nodes += child.perft(depth - 1);
    }

    return nodes;
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
     for (int i = 0; i < 64; i++) {
        squares[i] = position[i];
    }

    WhiteToMove = whiteToMove;

enPassantSquare = epSquare;
whiteKingsideCastle = wks;
whiteQueensideCastle = wqs;
blackKingsideCastle = bks;
blackQueensideCastle = bqs;
}
void Board::loadFEN(const std::string& fen) {

    Piece position[64];

    int boardIndex = 0;
    int fenIndex = 0;

    bool whiteToMove = false;

    int enPassantSquare = -1;

    bool blackQueensideCastle = false;
    bool blackKingsideCastle = false;
    bool whiteQueensideCastle = false;
    bool whiteKingsideCastle = false;

    // -------------------------
    // BOARD
    // -------------------------

    for (int i = 0; i < fen.length(); i++) {

        if (fen[i] == ' ') {
            fenIndex = i + 1;
            break;
        }

        if (fen[i] == '/') {
            continue;
        }

        if (fen[i] >= '1' && fen[i] <= '8') {

            int emptySquares = fen[i] - '0';

            for (int j = 0; j < emptySquares; j++) {
                position[boardIndex] = EMPTY;
                boardIndex++;
            }

        } else {

            position[boardIndex] = charToPiece(fen[i]);
            boardIndex++;
        }
    }

    // -------------------------
    // SIDE TO MOVE
    // -------------------------

    whiteToMove = fen[fenIndex] == 'w';

    fenIndex += 2;

    // -------------------------
    // CASTLING RIGHTS
    // -------------------------

    while (fen[fenIndex] != ' ') {

        if (fen[fenIndex] == 'K')
            whiteKingsideCastle = true;

        else if (fen[fenIndex] == 'Q')
            whiteQueensideCastle = true;

        else if (fen[fenIndex] == 'k')
            blackKingsideCastle = true;

        else if (fen[fenIndex] == 'q')
            blackQueensideCastle = true;

        fenIndex++;
    }

    fenIndex++;

    // -------------------------
    // EN PASSANT
    // -------------------------

    if (fen[fenIndex] == '-') {

        enPassantSquare = -1;

    } else {

        std::string square;

        square += fen[fenIndex];
        square += fen[fenIndex + 1];

        enPassantSquare = notationToIndex(square);
    }

    // -------------------------
    // APPLY POSITION
    // -------------------------

    setPosition(
        position,
        whiteToMove,
        enPassantSquare,
        blackQueensideCastle,
        blackKingsideCastle,
        whiteQueensideCastle,
        whiteKingsideCastle
    );
}
void Board::divide(int depth) {

    std::vector<Move> moves = generateLegalMoves();

    long long total = 0;

    for (const Move& move : moves) {

        Board child = *this;
        child.makeMove(move);

        long long nodes = child.perft(depth - 1);

        std::cout << indexToNotation(move.from)
                  << indexToNotation(move.to)
                  << " "
                  << nodes
                  << std::endl;

        total += nodes;
    }

    std::cout << "Total: " << total << std::endl;
}