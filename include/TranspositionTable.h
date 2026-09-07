#pragma once

#include <cstdint>
#include "Move.h"

enum class TTFlag : uint8_t { EMPTY, EXACT, LOWERBOUND, UPPERBOUND };

struct TTEntry
{ uint64_t key   = 0;
    int      depth = -1;
    int      score = 0;
    TTFlag   flag  = TTFlag::EMPTY;
    Move     bestMove{};
};

constexpr size_t TT_NUM_ENTRIES = 1ULL << 20;
constexpr size_t TT_INDEX_MASK  = TT_NUM_ENTRIES - 1;
