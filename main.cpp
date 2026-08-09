#include <iostream>
#include "board.h"
#include "square.h"

#include <iostream>
#include "Board.h"

int main() {

    Board board;

    board.loadFEN(
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"
    );
    board.printBoard();
      std::cout << "Kiwipete Perft depth 1: "
              << board.perft(1) << '\n';

    std::cout << "Kiwipete Perft depth 2: "
              << board.perft(2) << '\n';

    std::cout << "Kiwipete Perft depth 3: "
              << board.perft(3) << '\n';
    
              
               board.loadFEN(
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
    );
    board.printBoard();
      std::cout << "Kiwipete Perft depth 1: "
              << board.perft(1) << '\n';

    std::cout << "Kiwipete Perft depth 2: "
              << board.perft(2) << '\n';

    std::cout << "Kiwipete Perft depth 3: "
              << board.perft(3) << '\n';
              
 
              board.loadFEN(
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"
    );
    board.printBoard();
      std::cout << "Kiwipete Perft depth 1: "
              << board.perft(1) << '\n';

    std::cout << "Kiwipete Perft depth 2: "
              << board.perft(2) << '\n';

    std::cout << "Kiwipete Perft depth 3: "
              << board.perft(3) << '\n';
              


               
              board.loadFEN(
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
    );
    board.printBoard();
      std::cout << "Kiwipete Perft depth 1: "
              << board.perft(1) << '\n';

    std::cout << "Kiwipete Perft depth 2: "
              << board.perft(2) << '\n';

    std::cout << "Kiwipete Perft depth 3: "
              << board.perft(3) << '\n';
              
              
}
