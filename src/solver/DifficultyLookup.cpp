/*
* Created by Ed Fillingham on 09/05/2026.
*
* Implementation of difficulty lookup via packed data table.
*/

#include "DifficultyLookup.h"

// Packed difficulty data for 100 seeds
// 2 bits per seed: 0=invalid, 1=hard, 2=medium, 3=easy
static const uint8_t DIFFICULTY_DATA[25] = {
    0xbd,  // seeds 0-3
    0xef,  // seeds 4-7
    0xfe,  // seeds 8-11
    0x9f,  // seeds 12-15
    0x99,  // seeds 16-19
    0xfa,  // seeds 20-23
    0xa6,  // seeds 24-27
    0x56,  // seeds 28-31
    0x6a,  // seeds 32-35
    0xba,  // seeds 36-39
    0xfa,  // seeds 40-43
    0xff,  // seeds 44-47
    0xaf,  // seeds 48-51
    0xea,  // seeds 52-55
    0xe9,  // seeds 56-59
    0xa9,  // seeds 60-63
    0xea,  // seeds 64-67
    0xad,  // seeds 68-71
    0xf9,  // seeds 72-75
    0xa9,  // seeds 76-79
    0xee,  // seeds 80-83
    0x9a,  // seeds 84-87
    0xde,  // seeds 88-91
    0xba,  // seeds 92-95
    0xe7  // seeds 96-99
};

PuzzleDifficulty getDifficulty(int seed) {
    if (seed < 0 || seed >= 100) {
        return PuzzleDifficulty::NOT_FOUND;
    }

    int byteIndex = seed / 4;
    int bitOffset = (seed % 4) * 2;

    uint8_t byte = DIFFICULTY_DATA[byteIndex];
    uint8_t bits = (byte >> bitOffset) & 0x03;

    return static_cast<PuzzleDifficulty>(bits);
}
