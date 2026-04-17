/*
 * Created by Ed Fillingham on 17/04/2026.
 *
 * ArrangementMenuScreen — 2-row menu with blocker grid previews.
 *
 * Layout (landscape 280×240):
 * - Title bar: 30px top
 * - Row area: 210px (2 rows × 105px each)
 *   - Each arrangement row: [text left, scale-2] | [84×84 grid right]
 *   - Back row: "Back" centred
 *
 * Border animation (via BorderAnimator) outlines the selected row.
 */

#include "menu/ArrangementMenuScreen.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// ---- Constructor ----

/**
 * @brief Construct an ArrangementMenuScreen.
 *
 * @param tft     Shared TFT display reference.
 * @param manager Shared ScreenManager reference.
 * @param items   Pointer to exactly 2 ArrangementItems (must outlive this object).
 * @param count   Number of items — must be 2.
 * @param title   Title bar text.
 */
ArrangementMenuScreen::ArrangementMenuScreen(TFT_eSPI& tft, ScreenManager& manager,
                                              ArrangementItem* items, uint8_t count,
                                              const char* title)
    : _tft(tft), _manager(manager), _items(items), _count(count), _title(title),
      _selectedIndex(0), _viewStart(0), _dirty(false), _lastMs(0)
{
    assert(count == 2);
}

// ---- Runtime update ----

/**
 * @brief Swap the item array. Marks the screen dirty for re-render on the next loop.
 * @param items Pointer to exactly 2 ArrangementItems.
 * @param count Must be 2.
 */
void ArrangementMenuScreen::setItems(ArrangementItem* items, uint8_t count) {
    assert(count == 2);
    _items = items;
    _count = count;
    _dirty = true;
}

// ---- Lifecycle ----

/**
 * @brief Called when this screen is pushed onto the ScreenManager stack.
 *
 * Resets all state, clears the display, and draws the initial UI.
 */
void ArrangementMenuScreen::onEnter() {
    _selectedIndex = 0;
    _viewStart     = 0;
    _border.reset();
    _lastMs  = millis();
    _dirty   = false;
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawRowArea();
}

/**
 * @brief Called when this screen is popped from the stack. No cleanup needed.
 */
void ArrangementMenuScreen::onExit() {}

/**
 * @brief Called when a child screen returns. Resets animation and redraws static regions.
 */
void ArrangementMenuScreen::onResume() {
    _border.reset();
    _lastMs  = millis();
    _dirty   = false;
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawRowArea();
}

/**
 * @brief Advance border animation state. Called every loop iteration.
 */
void ArrangementMenuScreen::update() {
    unsigned long now = millis();
    float dt = (now - _lastMs) / 1000.0f;
    _lastMs = now;
    _border.advance(dt);
    _dirty = true;
}

/**
 * @brief Render the current frame. Skips when no changes to minimise SPI traffic.
 */
void ArrangementMenuScreen::render() {
    if (!_dirty) return;
    _dirty = false;
    updateSelectedBorder();
}

// ---- Input ----

/**
 * @brief Handle rotary encoder changes.
 *
 * Clamps selection to [0, totalItems-1], scrolls viewport if needed,
 * erases the old border, and redraws affected rows.
 *
 * @param delta Signed step (positive = down, negative = up).
 */
void ArrangementMenuScreen::onEncoderChange(int delta) {
    uint8_t prevIndex = _selectedIndex;

    int newIndex = (int)_selectedIndex + delta;
    if (newIndex < 0)                  newIndex = 0;
    if (newIndex >= (int)totalItems()) newIndex = (int)totalItems() - 1;
    _selectedIndex = (uint8_t)newIndex;

    if (_selectedIndex == prevIndex) return;

    // Erase live border on departing row before viewport potentially shifts
    if (_border.lastThickness >= 0.0f) {
        int oldSlot = (int)prevIndex - (int)_viewStart;
        if (oldSlot >= 0 && oldSlot < (int)VISIBLE_ROWS) {
            int rowTop = TITLE_H + oldSlot * ROW_H;
            BorderAnimator::drawBorderWithRoundedCorners(
                _tft, 2, rowTop + 2, SCR_W - 4, ROW_H - 4,
                _border.lastThickness, COL_BG, COL_BG);
        }
    }

    uint8_t prevViewStart = _viewStart;

    if (_selectedIndex < _viewStart) {
        _viewStart = _selectedIndex;
    } else if (_selectedIndex >= _viewStart + VISIBLE_ROWS) {
        _viewStart = _selectedIndex - VISIBLE_ROWS + 1;
    }

    // Reset border to fade in fresh on new row
    _border.reset();

    if (_viewStart != prevViewStart) {
        drawRowArea();
    } else {
        int prevSlot = (int)prevIndex      - (int)_viewStart;
        int newSlot  = (int)_selectedIndex - (int)_viewStart;
        if (prevSlot >= 0 && prevSlot < (int)VISIBLE_ROWS)
            drawRow(prevIndex,      (uint8_t)prevSlot);
        if (newSlot >= 0 && newSlot < (int)VISIBLE_ROWS)
            drawRow(_selectedIndex, (uint8_t)newSlot);
    }
}

/**
 * @brief Handle button press.
 *
 * Index 0 = Back (pops screen). Index 1-2 = call item's action callback.
 */
void ArrangementMenuScreen::onButtonPress() {
    if (_selectedIndex == 0) {
        _manager.pop();
    } else {
        _items[_selectedIndex - 1].action(_manager);
    }
}

// ---- Drawing ----

/**
 * @brief Draw the 30px title bar.
 */
void ArrangementMenuScreen::drawTitleBar() {
    _tft.setTextColor(COL_TITLE, COL_BG);
    _tft.setTextSize(2);
    int titleX = (SCR_W - (int)strlen(_title) * 12) / 2;
    _tft.setCursor(titleX, (TITLE_H - 16) / 2);
    _tft.print(_title);
}

/**
 * @brief Redraw all visible rows and their divider.
 *
 * Draws the divider first so row fills (ROW_H-1 height) never overwrite it.
 */
void ArrangementMenuScreen::drawRowArea() {
    // Dividers drawn first so row fills (ROW_H-1 height) never overwrite them
    for (uint8_t i = 0; i < VISIBLE_ROWS - 1; i++) {
        _tft.drawFastHLine(0, TITLE_H + (i + 1) * ROW_H - 1, SCR_W, COL_DIVIDER);
    }

    for (uint8_t i = 0; i < VISIBLE_ROWS; i++) {
        uint8_t absIndex = _viewStart + i;
        if (absIndex < totalItems()) {
            drawRow(absIndex, i);
        } else {
            _tft.fillRect(0, TITLE_H + i * ROW_H, SCR_W, ROW_H - 1, COL_BG);
        }
    }
}

/**
 * @brief Draw one row.
 *
 * Clears the row area, then renders:
 * - absIndex 0: "Back" text centred in the row
 * - absIndex 1-2: difficulty + score text (left) and blocker grid (right)
 *
 * @param absIndex Absolute item index (0=Back, 1-2=arrangement items).
 * @param rowSlot  On-screen slot (0 or 1).
 */
void ArrangementMenuScreen::drawRow(uint8_t absIndex, uint8_t rowSlot) {
    int rowTop = TITLE_H + rowSlot * ROW_H;
    bool selected = (absIndex == _selectedIndex);
    uint16_t textColor = selected ? COL_TEXT_SEL : COL_TEXT_UNSEL;

    _tft.fillRect(0, rowTop, SCR_W, ROW_H - 1, COL_BG);

    if (absIndex == 0) {
        // Back row: centred label only
        _tft.setTextColor(textColor, COL_BG);
        _tft.setTextSize(2);
        int bx = (SCR_W - 4 * 12) / 2;       // "Back" = 4 chars × 12px at size 2
        int by = rowTop + (ROW_H - 16) / 2;   // Vertically centred (16px = text height at size 2)
        _tft.setCursor(bx, by);
        _tft.print("Back");
        return;
    }

    const ArrangementItem& item = _items[absIndex - 1];

    // Build text strings
    static const char* DIFF_LABELS[] = { "Easy", "Med", "Hard" };
    static constexpr uint8_t DIFF_LABELS_COUNT = sizeof(DIFF_LABELS) / sizeof(DIFF_LABELS[0]);
    uint8_t diffIdx = (uint8_t)item.difficulty;
    if (diffIdx >= DIFF_LABELS_COUNT) diffIdx = 0;
    char line1[24];
    snprintf(line1, sizeof(line1), "Diff: %s", DIFF_LABELS[diffIdx]);

    char line2[24];
    if (item.seconds <= 0.0f) {
        snprintf(line2, sizeof(line2), "Score: --:--");
    } else {
        int total = (int)item.seconds;
        snprintf(line2, sizeof(line2), "Score: %02d:%02d", total / 60, total % 60);
    }

    // Two text lines, scale 2 (12×16px), vertically centred pair in row.
    // Line pair height: 16 + 8 + 16 = 40px; top pad = (105 - 40) / 2 = 32px.
    _tft.setTextColor(textColor, COL_BG);
    _tft.setTextSize(2);
    _tft.setCursor(8, rowTop + 32);
    _tft.print(line1);
    _tft.setCursor(8, rowTop + 56);
    _tft.print(line2);

    // Grid on right side
    drawArrangementGrid(rowTop, item.arrangement);
}

/**
 * @brief Draw the 6×6 blocker grid within a row.
 *
 * Draws white cell lines then fills each blocker coordinate with light grey.
 * Grid is GRID_SZ×GRID_SZ (84×84px), right-aligned with 8px margin,
 * vertically centred in the row (GRID_Y_OFF = 10px top pad).
 *
 * @param rowTop  Pixel y of the row's top edge.
 * @param coords  Blocker coordinates to fill (each x/y in [0,5]).
 */
void ArrangementMenuScreen::drawArrangementGrid(int rowTop, const std::vector<Coord>& coords) {
    int gx = GRID_X;
    int gy = rowTop + GRID_Y_OFF;

    // Draw grid lines (7 horizontal + 7 vertical = 6x6 cells)
    for (int i = 0; i <= GRID_CELLS; i++) {
        _tft.drawFastVLine(gx + i * CELL_SZ, gy, GRID_SZ, COL_GRID_LINE);
        _tft.drawFastHLine(gx, gy + i * CELL_SZ, GRID_SZ, COL_GRID_LINE);
    }

    // Draw blocker markers as filled circles centered in each cell's old inset rect
    for (const Coord& c : coords) {
        int centerX = gx + c.x * CELL_SZ + (CELL_SZ / 2);
        int centerY = gy + c.y * CELL_SZ + (CELL_SZ / 2);
        int radius = ((CELL_SZ - 1) / 2)-3;
        _tft.fillCircle(centerX, centerY, radius, COL_BLOCKER);
    }
}

/**
 * @brief Update the animated glow border around the selected row.
 *
 * Delta-renders: only issues TFT commands when the computed thickness or colour
 * differs from last frame by more than BorderAnimator::BORDER_CHANGE_TOL.
 * The border outlines the full row rectangle (2px inset from row edges).
 */
void ArrangementMenuScreen::updateSelectedBorder() {
    int rowSlot = (int)_selectedIndex - (int)_viewStart;
    if (rowSlot < 0 || rowSlot >= (int)VISIBLE_ROWS) return;
    int rowTop  = TITLE_H + rowSlot * ROW_H;

    // Border box: full row, 2px inset
    const int bx = 2;
    const int by = rowTop + 2;
    const int bw = SCR_W - 4;
    const int bh = ROW_H - 4;

    float exactThickness = BorderAnimator::BORDER_T_MIN
        + (BorderAnimator::BORDER_T_MAX - BorderAnimator::BORDER_T_MIN)
        * (0.5f + 0.5f * sinf(_border.phase));

    uint16_t borderColour = (_border.envelope >= 0.99f)
        ? COL_BORDER
        : BorderAnimator::blendColor(COL_BG, COL_BORDER, _border.envelope);

    if (fabsf(exactThickness - _border.lastThickness) < BorderAnimator::BORDER_CHANGE_TOL
        && borderColour == _border.lastColor) return;

    if (_border.envelope < 0.02f) {
        if (_border.lastThickness >= 0.0f) {
            BorderAnimator::drawBorderWithRoundedCorners(
                _tft, bx, by, bw, bh, _border.lastThickness, COL_BG, COL_BG);
        }
        _border.lastThickness = -1.0f;
        _border.lastColor     = 0;
        return;
    }

    BorderAnimator::drawBorderWithRoundedCorners(
        _tft, bx, by, bw, bh, exactThickness, borderColour, COL_BG);

    _border.lastThickness = exactThickness;
    _border.lastColor     = borderColour;
}
