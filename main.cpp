#include <iostream>
#include "board.h"
#include "square.h"
int main() {
    Board board;
    board.printBoard();
    std::string fromNotation, toNotation;
    std::cout << "Enter move (e.g., e2 e4): ";
    std::cin >> fromNotation >> toNotation;
    int fromIndex = notationToIndex(fromNotation);
    int toIndex = notationToIndex(toNotation);
    Move move(fromIndex, toIndex);

    board.makeMove(move);
    board.printBoard();
}