#include <iostream>
#include <cstdint>
#include <bit>

using namespace std;

int main() {
uint64_t board = 0;

board |= (1ULL << 2);
board |= (1ULL << 5);
board |= (1ULL << 8);
board |= (1ULL << 13);

uint64_t lsb = board & -board;

cout << board << endl;
cout << lsb << endl;
}