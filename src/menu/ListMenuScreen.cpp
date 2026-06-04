/*
 * Created by Ed Fillingham on 12/04/2026.
 *
 * ListMenuScreen — scrollable vertical list with numbered rows.
 *
 * A Screen that displays a vertical list of options with on-screen scrolling.
 * VISIBLE_ROWS (default 2) rows are shown at once; encoder scrolls through the list.
 *
 * UI layout (landscape 280×240):
 * - Title bar: 30px top (grey text)
 * - Row area: 210px (2 rows × 105px each, minus 1px gap)
 *   - Each row displays: [number box (40×40px)] [label text]
 *   - Number box has animated glow border when selected
 *
 * The first entry is always a "Back" item (auto-injected) that calls pop().
 * Real items (from the items array) start at index 1.
 *
 * Animation: Border fades in at 8 rad/sec with a sine-wave thickness (1–3px).
 * Delta rendering avoids flicker — only TFT updates occur when border thickness/colour
 * changes.
 */

#include "menu/ListMenuScreen.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "utils/Buzzer.h"

extern Buzzer buzzer;

// ---- Constructor ----

/**
 * @brief Construct a ListMenuScreen.
 *
 * @param tft     Reference to the shared TFT_eSPI display.
 * @param manager Reference to the ScreenManager (used to pop this screen on Back).
 * @param items   Pointer to array of real ListItem entries (Back is auto-injected).
 * @param count   Number of real items (not including Back).
 * @param title   Text displayed in the title bar.
 */
ListMenuScreen::ListMenuScreen(TFT_eSPI& tft, ScreenManager& manager,
                               const ListItem* items, uint8_t count,
                               const char* title)
    : _tft(tft), _manager(manager), _items(items), _count(count), _title(title),
      _selectedIndex(0), _viewStart(0),
      _lastMs(millis()), _dirty(false)
{
}

// ---- Helpers ----

/**
 * @brief Return total navigable items (real items + auto-injected Back).
 */
uint16_t ListMenuScreen::totalItems() const {
    return _count + 1; // +1 for auto-injected Back
}

/**
 * @brief Return the display label for an absolute item index.
 *
 * Index 0 returns "Back"; indices 1..count return _items[index-1].label.
 *
 * @param absIndex Absolute index in the list (0 = Back, 1..count = real items).
 * @return Pointer to label string (never nullptr).
 */
const char* ListMenuScreen::labelFor(uint8_t absIndex) const {
    if (absIndex == 0) return "Back";
    return _items[absIndex - 1].label;
}

// ---- Lifecycle ----

/**
 * @brief Called when this screen is pushed onto the ScreenManager stack.
 *
 * Initialises all animation state, resets selection to Back (index 0),
 * fills screen with black background, and draws title bar + row area.
 * The border is not drawn until render() updates it.
 */
void ListMenuScreen::onEnter() {
    _selectedIndex       = 0;        // Start at Back item
    _viewStart           = 0;        // Show top rows
    _border.reset();
    _lastMs              = millis();
    _dirty               = false;
    _tft.fillScreen(COL_BG);         // Clear display
    drawTitleBar();                  // Draw static title
    drawRowArea();                   // Draw all visible rows
}

/**
 * @brief Called when this screen is popped from the stack.
 *
 * No cleanup needed — TFT and ScreenManager are shared and owned by main.cpp.
 */
void ListMenuScreen::onExit() {}

/**
 * @brief Called when a child screen has been popped and this screen resumes.
 *
 * Resets animation state (border phase, envelope) and redraws static regions
 * without full reinitialisation. Selection state is preserved.
 */
void ListMenuScreen::onResume() {
    _border.reset();
    _lastMs              = millis();
    _dirty               = false;
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawRowArea();
}

/**
 * @brief Update animation state (called every loop iteration).
 *
 * 1. Compute delta-time since last update
 * 2. Advance border phase (sine wave for thickness oscillation)
 * 3. Fade in the border envelope (starts at 0, fades to 1 at 8 rad/sec)
 * 4. Mark screen as dirty so render() updates the border on next frame
 */
void ListMenuScreen::update() {
    unsigned long now = millis();
    float dt = (now - _lastMs) / 1000.0f;  // Delta time in seconds
    _lastMs = now;

    _border.advance(dt);

    _dirty = true;  // Trigger render() to update border this frame
}

/**
 * @brief Render the current frame to the TFT.
 *
 * If dirty flag is clear (no animation or input changes), skip rendering.
 * Otherwise, update the animated border on the selected row's number box.
 * All other UI (rows, title) are static and drawn once in onEnter()/onResume().
 */
void ListMenuScreen::render() {
    if (!_dirty) return;
    _dirty = false;
    updateNumberBoxBorder();  // Delta rendering: only update border if thickness/colour changed
}

// ---- Input ----

/**
 * @brief Handle rotary encoder changes.
 *
 * Updates _selectedIndex (clamped to [0, totalItems-1]). If the selection changes,
 * updates _viewStart to keep the selected row visible (within VISIBLE_ROWS range),
 * erases the old border (if visible), and redraws the affected rows.
 *
 * @param delta Signed step delta (positive = down, negative = up).
 */
void ListMenuScreen::onEncoderChange(int delta) {
    const uint8_t prevIndex = _selectedIndex;

    // Compute new index, clamped to [0, totalItems-1] (no wrapping).
    int newIndex = (int)_selectedIndex + delta;
    if (newIndex < 0)                    newIndex = 0;
    if (newIndex >= (int)totalItems())   newIndex = (int)totalItems() - 1;
    _selectedIndex = (uint8_t)newIndex;

    if (_selectedIndex == prevIndex) return; // already at boundary, nothing to redraw

    buzzer.play(SoundEffect::MENU_TICK);

    // Erase the live border on the departing row before the viewport shifts.
    if (_border.lastThickness >= 0.0f) {
        int oldScreenRow = (int)prevIndex - (int)_viewStart;
        if (oldScreenRow >= 0 && oldScreenRow < (int)VISIBLE_ROWS) {
            int oldBoxX = (NUM_COL_W - NUM_BOX_SIZE) / 2;
            int oldBoxY = TITLE_H + oldScreenRow * ROW_H + (ROW_H - NUM_BOX_SIZE) / 2;
            BorderAnimator::drawBorderWithRoundedCorners(_tft, oldBoxX, oldBoxY, NUM_BOX_SIZE, NUM_BOX_SIZE, _border.lastThickness, COL_BG, COL_BG);
        }
    }

    const uint8_t prevViewStart = _viewStart;

    // Scroll viewport to keep selected item visible
    if (_selectedIndex < _viewStart) {
        _viewStart = _selectedIndex;
    } else if (_selectedIndex >= _viewStart + VISIBLE_ROWS) {
        _viewStart = _selectedIndex - VISIBLE_ROWS + 1;
    }

    // Reset border so it fades in fresh on the new row
    _border.reset();

    if (_viewStart != prevViewStart) {
        drawRowArea(); // Viewport shifted — all rows may show different items
    } else {
        // Only the two rows that changed selection state need redrawing
        int prevScreenRow = (int)prevIndex    - (int)_viewStart;
        int newScreenRow  = (int)_selectedIndex - (int)_viewStart;
        if (prevScreenRow >= 0 && prevScreenRow < (int)VISIBLE_ROWS)
            drawRow((uint8_t)prevScreenRow, prevIndex, false);
        if (newScreenRow >= 0 && newScreenRow < (int)VISIBLE_ROWS)
            drawRow((uint8_t)newScreenRow, _selectedIndex, true);
    }
}

/**
 * @brief Handle button press (confirm/select action).
 *
 * - If Back item (index 0) is selected, pop this screen.
 * - Otherwise, call the action callback for the selected item.
 */
void ListMenuScreen::onButtonPress() {
    buzzer.play(SoundEffect::MENU_SELECT);
    if (_selectedIndex == 0) {
        _manager.pop();  // Back selected — return to previous screen
    } else {
        _items[_selectedIndex - 1].action(_manager);  // Call item's action callback
    }
}

// ---- Drawing ----

/**
 * @brief Draw the title bar (30px top).
 *
 * Displays the screen title in grey, horizontally centred, vertically centred.
 * Uses TFT text size 2 (12×16px per character).
 */
void ListMenuScreen::drawTitleBar() {
    _tft.setTextColor(COL_TITLE, COL_BG);
    _tft.setTextSize(2);
    int titleX = (SCR_W - (int)strlen(_title) * 12) / 2;  // Horizontal centre
    _tft.setCursor(titleX, (TITLE_H - 16) / 2);           // Vertical centre
    _tft.print(_title);
}

/**
 * @brief Draw a single list row.
 *
 * Layout per row (ROW_H = 105px):
 * - Left column (NUM_COL_W = 50px): Number box (40×40px, centred)
 *   - Display: 1-based index, size 3 text (18×24px per char), centred in box
 * - Right area: Label text (size 2, 12×16px), left-aligned at +8px, vertically centred
 * - Divider: 1px grey line at bottom (preserved by ROW_H-1 fill)
 *
 * Colours: white if selected, grey if unselected.
 *
 * @param screenRow  On-screen row index (0..VISIBLE_ROWS-1).
 * @param absIndex   Absolute item index (0 = Back, 1..count = real items).
 * @param selected   Whether this row is the current selection.
 */
void ListMenuScreen::drawRow(uint8_t screenRow, uint8_t absIndex, bool selected) {
    int y = TITLE_H + screenRow * ROW_H;
    uint16_t textColor = selected ? COL_TEXT_SEL : COL_TEXT_UNSEL;

    // Clear the row (ROW_H-1 preserves the divider pixel at the bottom)
    _tft.fillRect(0, y, SCR_W, ROW_H - 1, COL_BG);

    // Number box: NUM_BOX_SIZE×NUM_BOX_SIZE (40×40px), centred in the NUM_COL_W (50px) left column
    int numBoxX = (NUM_COL_W - NUM_BOX_SIZE) / 2; // = 5
    int numBoxY = y + (ROW_H - NUM_BOX_SIZE) / 2;  // = y + 6

    // Row number text (1-based absolute position).
    // Uses NUM_TEXT_SIZE=3: each char is 18px wide, 24px tall.
    // Note: 3-digit numbers (100+) visually overflow the 40px box.
    char numStr[4];
    snprintf(numStr, sizeof(numStr), "%d", absIndex + 1);
    int numTextX = numBoxX + (NUM_BOX_SIZE - (int)strlen(numStr) * NUM_TEXT_SIZE * 6) / 2;
    int numTextY = numBoxY + (NUM_BOX_SIZE - NUM_TEXT_SIZE * 8) / 2;
    _tft.setTextColor(textColor, COL_BG);
    _tft.setTextSize(NUM_TEXT_SIZE);
    _tft.setCursor(numTextX, numTextY);
    _tft.print(numStr);

    // Label text, size 2 (12×16px), vertically centred in row
    int labelX = NUM_COL_W + 8;          // = 58 (right of number column + padding)
    int labelY = y + (ROW_H - 16) / 2;   // Vertically centred
    _tft.setTextColor(textColor, COL_BG);
    _tft.setTextSize(2);
    _tft.setCursor(labelX, labelY);
    _tft.print(labelFor(absIndex));
}

/**
 * @brief Redraw all visible rows (and dividers).
 *
 * Draws VISIBLE_ROWS rows starting from _viewStart index. Dividers are drawn
 * first (grey 1px lines between rows) so row fills never erase them.
 * Each row is drawn with appropriate highlight based on selection state.
 * Empty slots (beyond totalItems()) are filled with background colour.
 */
void ListMenuScreen::drawRowArea() {
    // Dividers drawn first so row fills (height ROW_H-1) never erase them
    for (uint8_t i = 0; i < VISIBLE_ROWS - 1; i++) {
        int divY = TITLE_H + (i + 1) * ROW_H - 1;
        _tft.drawFastHLine(0, divY, SCR_W, COL_DIVIDER);
    }

    for (uint8_t i = 0; i < VISIBLE_ROWS; i++) {
        uint8_t absIndex = _viewStart + i;
        if (absIndex < totalItems()) {
            drawRow(i, absIndex, absIndex == _selectedIndex);
        } else {
            // No item here — clear to background (ROW_H-1 to preserve divider pixel)
            _tft.fillRect(0, TITLE_H + i * ROW_H, SCR_W, ROW_H - 1, COL_BG);
        }
    }
}

/**
 * @brief Update the animated glow border on the selected row's number box.
 *
 * This is a delta-rendering optimization: only redraws the border when thickness
 * or colour changes by more than BorderAnimator::BORDER_CHANGE_TOL. Avoids unnecessary SPI traffic.
 *
 * Animation:
 * 1. Thickness oscillates between BORDER_T_MIN (1px) and BORDER_T_MAX (3px) via sine wave
 *    based on _border.phase (4 rad/sec).
 * 2. Colour fades in via _border.envelope (fades in at 8 rad/sec after selection; reset to 0
 *    when selection changes).
 *
 * Early-exit conditions:
 * - If thickness and colour haven't changed > 0.01f, skip render
 * - If envelope < 0.02f (nearly invisible), clear border and skip render
 *
 * Note: The AA fringe in drawBorderWithRoundedCorners automatically handles
 * ghost-pixel cleanup when the border shrinks, so we don't need an erase pass.
 */
void ListMenuScreen::updateNumberBoxBorder() {
    // Compute number box position on screen (within visible row area)
    int boxX = (NUM_COL_W - NUM_BOX_SIZE) / 2;
    int boxY = TITLE_H + (_selectedIndex - _viewStart) * ROW_H + (ROW_H - NUM_BOX_SIZE) / 2;

    // Compute animated border thickness (sine wave between 1–3px)
    float exactThickness = BorderAnimator::BORDER_T_MIN + (BorderAnimator::BORDER_T_MAX - BorderAnimator::BORDER_T_MIN) * (0.5f + 0.5f * sinf(_border.phase));

    // Compute border colour with fade blending
    uint16_t borderColour = (_border.envelope >= 0.99f)
        ? COL_BORDER                                                              // Full opacity
        : BorderAnimator::blendColor(COL_BG, COL_BORDER, _border.envelope);      // Blend with background

    // Early-exit: thickness and colour unchanged since last frame (within tolerance)
    if (fabsf(exactThickness - _border.lastThickness) < BorderAnimator::BORDER_CHANGE_TOL && borderColour == _border.lastColor) return;

    // Too faded to see; clear old border (if any) and skip rendering
    if (_border.envelope < 0.02f) {
        if (_border.lastThickness >= 0.0f) {
            // Erase by drawing the same border shape in background colour
            BorderAnimator::drawBorderWithRoundedCorners(_tft, boxX, boxY, NUM_BOX_SIZE, NUM_BOX_SIZE, _border.lastThickness, COL_BG, COL_BG);
        }
        _border.lastThickness = -1.0f;  // Flag: no border drawn
        _border.lastColor     = 0;
        return;
    }

    // Draw border — no separate erase pass needed. The AA fringe in drawBorderWithRoundedCorners
    // overwrites ghost pixels when the border shrinks, preventing black-flash artefacts.
    BorderAnimator::drawBorderWithRoundedCorners(_tft, boxX, boxY, NUM_BOX_SIZE, NUM_BOX_SIZE, exactThickness, borderColour, COL_BG);

    // Update cache for next frame's change detection
    _border.lastThickness = exactThickness;
    _border.lastColor     = borderColour;
}
