/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Plain data struct representing a single navigable menu item.
 * No logic — consumed by CarouselMenuScreen and future menu screens.
 */

#ifndef MENU_ITEM_H
#define MENU_ITEM_H

#include <Arduino.h>

class ScreenManager; // forward declaration — avoids circular include

/**
 * @brief A single entry in a menu screen.
 */
struct MenuItem {
    const uint8_t* bitmap;           ///< PROGMEM 1-bit bitmap; nullptr = use placeholderChar
    uint8_t        bitmapW;          ///< Bitmap width in pixels
    uint8_t        bitmapH;          ///< Bitmap height in pixels
    char           placeholderChar;  ///< Displayed centred in tile when bitmap is nullptr
    const char*    label;            ///< Text label shown in the label bar when selected
    void (*action)(ScreenManager&);  ///< Called when this item is confirmed with the button
};

#endif // MENU_ITEM_H
