#pragma once
#include "Board.h"
#include "Move.h"
#include <vector>

std::vector<Move> generateRookMoves(const Board &board, int squareIndex);