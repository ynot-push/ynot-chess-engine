#include <iostream>
#include <chrono>
#include "Board.h"
#include "Search.h"
#include "square.h"

std::string moveToString(const Move& move) {
    if (move.isNull()) {
        return "(none)";
    }

    std::string text = indexToNotation(move.from) + indexToNotation(move.to);

    switch (move.promotionPiece) {
        case WQUEEN:  case BQUEEN:  text += 'q'; break;
        case WROOK:   case BROOK:   text += 'r'; break;
        case WBISHOP: case BBISHOP: text += 'b'; break;
        case WKNIGHT: case BKNIGHT: text += 'n'; break;
        default: break;
    }

    return text;
}

int main() {
    Board board; // defaults to the standard starting position

    std::cout << "YNOT Chess Engine - search demo" << std::endl;
    board.printBoard();
    std::cout << std::endl;

    for (int depth = 1; depth <= 6; depth++) {

        auto start = std::chrono::high_resolution_clock::now();

        Search::SearchResult result = Search::findBestMove(board, depth);

        auto end = std::chrono::high_resolution_clock::now();
        double seconds = std::chrono::duration<double>(end - start).count();

        std::cout << "depth " << result.depth
                  << "  bestmove " << moveToString(result.bestMove)
                  << "  score " << result.score
                  << "  nodes " << result.nodes
                  << "  time " << seconds << "s";

        if (seconds > 0.0) {
            std::cout << "  nps " << static_cast<long long>(result.nodes / seconds);
        }

        std::cout << std::endl;

        if (result.bestMove.isNull()) {
            std::cout << (board.isInCheck(board.whiteToMove()) ? "checkmate" : "stalemate")
                       << " at the root, stopping." << std::endl;
            break;
        }
    }

    return 0;
}