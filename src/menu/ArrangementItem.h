/*
 * Created by Ed Fillingham on 17/04/2026.
 *
 * Data types for ArrangementMenuScreen items.
 */

#ifndef ARRANGEMENT_ITEM_H
#define ARRANGEMENT_ITEM_H

#include <vector>
#include <functional>
#include "utils/math/maths.h"  // Coord

class ScreenManager; // forward declaration

/**
 * @brief Puzzle difficulty level.
 */
enum class Difficulty : uint8_t { EASY, MED, HARD };

/**
 * @brief A single entry in an ArrangementMenuScreen.
 */
struct ArrangementItem {
    Difficulty          difficulty;   ///< Difficulty label displayed on the row
    float               seconds;      ///< Previous best score (0.0f = no score → "--:--")
    std::vector<Coord>  arrangement;  ///< Exactly 7 blocker coordinates for the grid preview
    std::function<void(ScreenManager&)> action;  ///< Called when this item is confirmed
};

#endif // ARRANGEMENT_ITEM_H
