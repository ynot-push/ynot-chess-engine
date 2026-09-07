#include <iostream>
#include <chrono>
#include "Board.h"

int runPerftTests()
{
    Board board;

    // Known-correct perft results for the standard starting position
    const long long expected[] = { 1, 20, 400, 8902, 197281, 4865609 };

    int failures = 0;

    for (int depth = 1; depth <= 5; depth++)
    {
        auto start = std::chrono::steady_clock::now();
        long long nodes = board.perft(depth);
        auto end = std::chrono::steady_clock::now();

        double seconds = std::chrono::duration<double>(end - start).count();
        bool ok = (nodes == expected[depth]);

        if (!ok) failures++;

        std::cout << "perft(" << depth << ") = " << nodes
                   << "  expected " << expected[depth]
                   << (ok ? "  OK" : "  MISMATCH")
                   << "  (" << seconds << "s)\n";
    }

    return failures;
}

int main()
{
    int failures = runPerftTests();

    if (failures == 0)
        std::cout << "\nAll perft tests passed.\n";
    else
        std::cout << "\n" << failures << " perft depth(s) failed.\n";

    return failures;
}