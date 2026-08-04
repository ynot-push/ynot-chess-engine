#pragma once

struct Move {
    int from;
    int to;
    Move(int fromSquare, int toSquare)
        : from(fromSquare), to(toSquare)
    {
    }
};