#include <iostream>
#include "include/board.h"
Board::Board(){
    initialize();
}
void Board::initialize() {


}   

void  Board::printBoard() const{

}

void Board::makeMove(const Move& move);

Piece Board::getPiece(int square) const;

std::vector<Move> generateAllMoves();

std::vector<Move> generateLegalMoves();

bool Board::isSquareAttacked(int square, bool byWhite) const;

bool Board::isInCheck(bool white) const;

long long Board::perft(int depth);

void Board::setPosition(
        const Piece position[64],
        bool whiteToMove,
        int epSquare,
        bool bqs,
        bool bks,
        bool wqs,
        bool wks
    );

void Board::loadFEN(const std::string& fen);

bool Board::canWhiteCastleKingside() const;
bool Board::canWhiteCastleQueenside() const;
bool Board::canBlackCastleKingside() const;
bool Board::canBlackCastleQueenside() const;

int Board::getEnPassantSquare() const;
