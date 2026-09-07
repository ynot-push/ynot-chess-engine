#include <iostream>
#include <chrono>
#include "Board.h"
#include "NNUE.h"
#include "square.h"

int main()
{
    if (!NNUE::load("tools/nnue.bin"))
    {
        std::cerr << "Failed to load nnue.bin\n";
        return 1;
    }

    Board board;
    board.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

for (int depth = 1; depth <= 6; depth++)
{
    auto start = std::chrono::steady_clock::now();
    Move best = board.findBestMove(depth);
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration<double>(end - start).count();
    long long nodes = board.getNodeCount();
    double nps = seconds > 0 ? nodes / seconds : 0;

    std::cout << "depth " << depth << ": "
               << indexToNotation(best.from) << indexToNotation(best.to)
               << "  nodes=" << nodes
               << "  time=" << seconds << "s"
               << "  nps=" << static_cast<long long>(nps) << "\n";
}
    std::cout << "genMoves:   " << Board::timeGenMoves << "s\n";
    std::cout << "legality:   " << Board::timeLegalityFilter << "s\n";
    std::cout << "eval:       " << Board::timeEval << "s\n";
    std::cout << "makeUnmake: " << Board::timeMakeUnmake << "s\n";

    return 0;
}