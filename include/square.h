#pragma once
#include <string>

int notationToIndex(const std::string& notation);
std::string indexToNotation(int index);
int getRow(int index);
int getColumn(int index);