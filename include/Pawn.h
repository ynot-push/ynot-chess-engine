#pragma once
#include "Board.h"
#include "Move.h"
#include <vector>

std::vector<Move> generatePawnMoves(const Board &board, int squareIndex);
