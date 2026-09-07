#include <iostream>
#include <string>
#include "square.h"

int notationToIndex(const std::string& notation)
{
    if (notation.length() != 2)
        return -1;

    if (notation[0] < 'a' || notation[0] > 'h' || notation[1] < '1' || notation[1] > '8')
        return -1;

    const int file = notation[0] - 'a';
    const int rank = notation[1] - '1';
    return rank * 8 + file;
}

std::string indexToNotation(int index)
{
    if (index < 0 || index > 63)
        return "";

    std::string notation = "  ";
    notation[0] = 'a' + (index % 8);
    notation[1] = '1' + (index / 8);
    return notation;
}

int getRow(int index)    { return index / 8; }
int getColumn(int index) { return index % 8; }