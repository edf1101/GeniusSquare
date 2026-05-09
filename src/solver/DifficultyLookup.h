/*
* Created by Ed Fillingham on 09/05/2026.
*
* Packed difficulty lookup for Genius Square puzzle seeds (0-99).
* Each seed difficulty is stored in 2 bits (25 bytes total for 100 seeds).
*/

#ifndef GENIUS_SQUARE_DIFFICULTY_LOOKUP_H
#define GENIUS_SQUARE_DIFFICULTY_LOOKUP_H

#include <cstdint>

/** @brief Packed difficulty classification for a puzzle arrangement. */
enum class PuzzleDifficulty {
    NOT_FOUND = 0, /**< Invalid or out-of-range seed. */
    HARD = 1,      /**< Hard difficulty (1-120 solutions). */
    MEDIUM = 2,    /**< Medium difficulty (121-1000 solutions). */
    EASY = 3       /**< Easy difficulty (>1000 solutions). */
};

/** @brief Retrieve the difficulty of a puzzle seed via packed lookup table.
 *  @param seed The seed number (0-99).
 *  @return PuzzleDifficulty classification for the seed.
 */
PuzzleDifficulty getDifficulty(int seed);

#endif // GENIUS_SQUARE_DIFFICULTY_LOOKUP_H
