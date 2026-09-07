
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <string>
#include <cstdlib>
#include "Board.h"

struct DepthCase {
    int depth;
    long long expectedNodes;
};

struct PerftPosition {
    std::string name;
    std::string fen; 
    std::vector<DepthCase> cases;
};


static const std::vector<PerftPosition> kPositions = {
    {
        "Start position",
        "",
        {
            {1, 20},
            {2, 400},
            {3, 8902},
            {4, 197281},
            {5, 4865609},
        }
    },
    {
        "Kiwipete",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        {
            {1, 48},
            {2, 2039},
            {3, 97862},
            {4, 4085603},
        }
    },
    {
        "Endgame position (position 3)",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        {
            {1, 14},
            {2, 191},
            {3, 2812},
            {4, 43238},
            {5, 674624},
        }
    },
    {
        "Castling/promotion stress (position 4)",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        {
            {1, 6},
            {2, 264},
            {3, 9467},
            {4, 422333},
        }
    },
    {
        "Position 5",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        {
            {1, 44},
            {2, 1486},
            {3, 62379},
        }
    },
};

int main(int argc, char** argv) {
    int maxDepthOverride = -1;
    if (argc > 1) {
        maxDepthOverride = std::atoi(argv[1]);
    }

    int totalCases = 0;
    int passedCases = 0;
    long long totalNodes = 0;
    double totalSeconds = 0.0;

    std::cout << std::left;

    for (const PerftPosition& position : kPositions) {
        std::cout << "== " << position.name << " ==\n";

        for (const DepthCase& dc : position.cases) {
            int depth = maxDepthOverride > 0 ? maxDepthOverride : dc.depth;
            if (maxDepthOverride > 0 && depth != dc.depth) {

            }

            Board board;
            if (!position.fen.empty()) {
                board.loadFEN(position.fen);
            }

            auto start = std::chrono::high_resolution_clock::now();
            long long nodes = board.perft(depth);
            auto end = std::chrono::high_resolution_clock::now();
            double seconds = std::chrono::duration<double>(end - start).count();

            totalCases++;
            totalNodes += nodes;
            totalSeconds += seconds;

            bool hasReference = (maxDepthOverride <= 0);
            bool pass = !hasReference || (nodes == dc.expectedNodes);
            if (pass) passedCases++;

            double nps = seconds > 0.0 ? nodes / seconds : 0.0;

            std::cout << "  depth " << std::setw(2) << depth
                       << "  nodes " << std::setw(12) << nodes;

            if (hasReference) {
                std::cout << "  expected " << std::setw(12) << dc.expectedNodes
                           << "  " << (pass ? "PASS" : "FAIL <<<<<<");
            }

            std::cout << "  time " << std::fixed << std::setprecision(3) << seconds << "s"
                       << "  nps " << std::fixed << std::setprecision(0) << nps
                       << "\n";


            if (maxDepthOverride > 0) {
                break; 
            }
        }

        std::cout << "\n";
    }

    std::cout << "----------------------------------------\n";
    std::cout << "Total nodes: " << totalNodes << "\n";
    std::cout << "Total time:  " << std::fixed << std::setprecision(3) << totalSeconds << "s\n";
    if (totalSeconds > 0.0) {
        std::cout << "Average nps: " << std::fixed << std::setprecision(0)
                   << (totalNodes / totalSeconds) << "\n";
    }
    std::cout << passedCases << "/" << totalCases << " cases passed\n";

    return (passedCases == totalCases) ? 0 : 1;
}
