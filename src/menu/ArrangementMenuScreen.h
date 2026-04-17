/*
 * Created by Ed Fillingham on 17/04/2026.
 *
 * A Screen that displays exactly 2 arrangement options, each showing a
 * difficulty label, previous best score, and a 6×6 blocker grid preview.
 * A "Back" item is auto-injected at index 0, giving 3 total rows with
 * 2 visible at a time (scrollable with the encoder).
 *
 * Layout (landscape 280×240):
 * - Title bar: 30px top
 * - 2 rows × 105px: left side = 2 text lines; right side = 84×84 grid
 *
 * Call setItems() to swap item data at runtime (re-renders on next loop).
 * count must be exactly 2; an assertion fires otherwise.
 */

#ifndef ARRANGEMENT_MENU_SCREEN_H
#define ARRANGEMENT_MENU_SCREEN_H

#include <TFT_eSPI.h>
#include <vector>
#include "menu/Screen.h"
#include "menu/ScreenManager.h"
#include "menu/BorderAnimator.h"
#include "menu/ArrangementItem.h"
#include "utils/math/maths.h"

class ArrangementMenuScreen : public Screen {
public:
    /**
     * @brief Construct an ArrangementMenuScreen.
     * @param tft     Reference to the shared TFT display.
     * @param manager Reference to the ScreenManager.
     * @param items   Pointer to exactly 2 ArrangementItems. Must outlive this object.
     * @param count   Must be 2 (asserted).
     * @param title   Text displayed in the title bar.
     */
    ArrangementMenuScreen(TFT_eSPI& tft, ScreenManager& manager,
                          ArrangementItem* items, uint8_t count,
                          const char* title);

    /**
     * @brief Swap the item array at runtime. Marks the screen dirty for re-render.
     * @param items Pointer to exactly 2 ArrangementItems. Must outlive this object.
     * @param count Must be 2 (asserted).
     */
    void setItems(ArrangementItem* items, uint8_t count);

    void onEnter()           override;
    void onExit()            override;
    void onResume()          override;
    void update()            override;
    void render()            override;
    void onEncoderChange(int delta) override;
    void onButtonPress()     override;

    static constexpr uint8_t VISIBLE_ROWS = 2;

private:
    TFT_eSPI&        _tft;
    ScreenManager&   _manager;
    ArrangementItem* _items;
    uint8_t          _count;          ///< Always 2
    const char*      _title;

    uint8_t          _selectedIndex;  ///< 0=Back, 1-2=arrangement items
    uint8_t          _viewStart;      ///< Absolute index of first visible row
    bool             _dirty;
    unsigned long    _lastMs;
    BorderAnimator   _border;

    // ---- Layout constants (landscape 280×240) ----
    static constexpr int SCR_W   = 280;
    static constexpr int SCR_H   = 240;
    static constexpr int TITLE_H = 30;
    static constexpr int ROW_H   = (SCR_H - TITLE_H) / VISIBLE_ROWS; // 105

    // ---- Grid constants (right-aligned in each row) ----
    static constexpr int GRID_CELLS = 6;
    static constexpr int CELL_SZ    = 14;                          // px per cell
    static constexpr int GRID_SZ    = GRID_CELLS * CELL_SZ;       // 84
    static constexpr int GRID_X     = SCR_W - GRID_SZ - 16;        // 188 (left edge of grid)
    static constexpr int GRID_Y_OFF = (ROW_H - GRID_SZ) / 2;      // 10 (top padding within row)

    // ---- Colour palette (RGB565) ----
    static constexpr uint16_t COL_BG         = 0x0000; ///< Black background
    static constexpr uint16_t COL_TEXT_SEL   = 0xFFFF; ///< Selected row text — white
    static constexpr uint16_t COL_TEXT_UNSEL = 0xAD75; ///< Unselected row text — grey
    static constexpr uint16_t COL_DIVIDER    = 0x2965; ///< Row divider — dark grey
    static constexpr uint16_t COL_BORDER     = 0xFFFF; ///< Animated glow border — white
    static constexpr uint16_t COL_TITLE      = 0xAD75; ///< Title bar text — grey
    static constexpr uint16_t COL_GRID_LINE  = 0xFFFF; ///< Grid cell lines — white
    static constexpr uint16_t COL_BLOCKER    = 0xDDB4; ///< Blocker cell fill — light grey

    /** @brief Total items including auto-injected Back (always 3). */
    uint8_t totalItems() const { return _count + 1; }

    void drawTitleBar();
    void drawRowArea();

    /**
     * @brief Draw a single row (clears row first, then renders content).
     * @param absIndex Absolute item index (0=Back, 1-2=real items).
     * @param rowSlot  On-screen slot (0 or 1).
     */
    void drawRow(uint8_t absIndex, uint8_t rowSlot);

    /**
     * @brief Draw the 6×6 blocker grid inside a row.
     * @param rowTop  Pixel y of the row's top edge.
     * @param coords  7 blocker coordinates to fill.
     */
    void drawArrangementGrid(int rowTop, const std::vector<Coord>& coords);

    /**
     * @brief Update the animated glow border around the selected row.
     *        Delta-renders: only issues TFT commands when thickness/colour changed.
     */
    void updateSelectedBorder();
};

#endif // ARRANGEMENT_MENU_SCREEN_H
