#include "include/Rook.h"
#include <iostream>
#include <iomanip>

void testRookMagic()
{
std::cout << "Initializing rook magic tables...\n";

Rook::initRookAttacks();

std::cout << "Initialization complete!\n";

for (int square = 0; square < 64; square++)
{
    Bitboard mask = Rook::RookMask(square);

    int relevantBits =
        __builtin_popcountll(mask);

    int possibleOccupancies =
        1 << relevantBits;

    for (int index = 0;
         index < possibleOccupancies;
         index++)
    {
        Bitboard occupancy =
            Rook::setOccupancy(
                index,
                relevantBits,
                mask);

        Bitboard slowAttack =
            Rook::attacksSlow(
                square,
                occupancy);

        Bitboard magicAttack =
            Rook::attacks(
                square,
                occupancy);

        if (slowAttack != magicAttack)
        {
            std::cout << "\nMAGIC TEST FAILED!\n";

            std::cout << "Square: "
                      << square << '\n';

            std::cout << "Occupancy index: "
                      << index << '\n';

            std::cout << "Slow attack: 0x"
                      << std::hex
                      << slowAttack << '\n';

            std::cout << "Magic attack: 0x"
                      << std::hex
                      << magicAttack << '\n';

            return;
        }
    }

    std::cout << "Square "
              << std::dec
              << square
              << " passed ("
              << possibleOccupancies
              << " occupancies)\n";
}

std::cout << "\nALL ROOK MAGIC TESTS PASSED!\n";

}

int main()
{
    long long example=1ULL<<0;
    std::cout<<example<<"ggynot hhh";
testRookMagic();

return 0;

}
