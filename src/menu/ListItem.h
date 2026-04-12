/*
 * Created by Ed Fillingham on 12/04/2026.
 *
 * Plain data struct representing a single navigable list entry.
 * Used by ListMenuScreen. No bitmaps — label and callback only.
 */

#ifndef LIST_ITEM_H
#define LIST_ITEM_H

class ScreenManager; // forward declaration — avoids circular include

/**
 * @brief A single entry in a scrolling list screen.
 */
struct ListItem {
    const char*            label;   ///< Text displayed in the list row
    void (*action)(ScreenManager&); ///< Called when this item is confirmed with the button
};

#endif // LIST_ITEM_H
