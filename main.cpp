#include <iostream>
#include <cstdint>
#include <bit>
#include "board.h"
#include "BitboardUtils.h"
using namespace std;

int main() {
    Board Board;
    cout << "White: " << popcount(Board.getWhitePieces()) << '\n';
    cout << "Black: " << popcount(Board.getBlackPieces()) << '\n';
    cout << "Occupied: " << popcount(Board.getOccupied()) << '\n';
    cout << "Empty: " << popcount(Board.getEmpty()) << '\n';
    Board.printBoard();
}