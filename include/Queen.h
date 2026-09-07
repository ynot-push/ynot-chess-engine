#pragma once
#include "Board.h"
#include "Move.h"
#include <vector>

std::vector<Move> generateQueenMoves(const Board &board, int squareIndex);