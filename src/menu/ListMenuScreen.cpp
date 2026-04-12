/*
 * Created by Ed Fillingham on 12/04/2026.
 *
 * Scrolling list menu screen implementation.
 */

#include "menu/ListMenuScreen.h"
#include <stdio.h>
#include <string.h>

// ---- Constructor ----

ListMenuScreen::ListMenuScreen(TFT_eSPI& tft, ScreenManager& manager,
                               const ListItem* items, uint8_t count,
                               const char* title)
    : _tft(tft), _manager(manager), _items(items), _count(count), _title(title),
      _selectedIndex(0), _viewStart(0)
{
}

// ---- Helpers ----

uint16_t ListMenuScreen::totalItems() const {
    return _count + 1; // +1 for auto-injected Back
}

const char* ListMenuScreen::labelFor(uint8_t absIndex) const {
    if (absIndex == 0) return "Back";
    return _items[absIndex - 1].label;
}

// ---- Lifecycle ----

void ListMenuScreen::onEnter() {
    _selectedIndex = 0;
    _viewStart     = 0;
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawRowArea();
}

void ListMenuScreen::onExit() {}

void ListMenuScreen::onResume() {
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawRowArea();
}

void ListMenuScreen::update() {}

void ListMenuScreen::render() {}

// ---- Input ----

void ListMenuScreen::onEncoderChange(int delta) {
    const uint8_t prevIndex = _selectedIndex;

    int newIndex = (int)_selectedIndex + delta;
    if (newIndex < 0)                    newIndex = 0;
    if (newIndex >= (int)totalItems())   newIndex = (int)totalItems() - 1;
    _selectedIndex = (uint8_t)newIndex;

    if (_selectedIndex == prevIndex) return; // already at boundary, nothing to redraw

    // Scroll viewport to keep selected item visible
    if (_selectedIndex < _viewStart) {
        _viewStart = _selectedIndex;
    } else if (_selectedIndex >= _viewStart + VISIBLE_ROWS) {
        _viewStart = _selectedIndex - VISIBLE_ROWS + 1;
    }

    drawRowArea();
}

void ListMenuScreen::onButtonPress() {
    if (_selectedIndex == 0) {
        _manager.pop();
    } else {
        _items[_selectedIndex - 1].action(_manager);
    }
}

// ---- Drawing ----

void ListMenuScreen::drawTitleBar() {
    _tft.setTextColor(COL_TITLE, COL_BG);
    _tft.setTextSize(2);
    int titleX = (SCR_W - (int)strlen(_title) * 12) / 2;
    _tft.setCursor(titleX, (TITLE_H - 16) / 2);
    _tft.print(_title);
}

void ListMenuScreen::drawRow(uint8_t screenRow, uint8_t absIndex, bool selected) {
    int y = TITLE_H + screenRow * ROW_H;
    uint16_t textColor = selected ? COL_TEXT_SEL : COL_TEXT_UNSEL;

    // Clear the row (ROW_H-1 preserves the divider pixel at the bottom)
    _tft.fillRect(0, y, SCR_W, ROW_H - 1, COL_BG);

    // Number box: 24×24px, centred in the 30px left column
    int numBoxX = (NUM_COL_W - NUM_BOX_SIZE) / 2; // = 3
    int numBoxY = y + (ROW_H - NUM_BOX_SIZE) / 2;  // = y + 14

    if (selected) {
        _tft.drawRect(numBoxX, numBoxY, NUM_BOX_SIZE, NUM_BOX_SIZE, COL_NUM_BOX);
    }

    // Row number text (1-based), size 2 = 12×16px per char
    char numStr[4];
    snprintf(numStr, sizeof(numStr), "%d", absIndex + 1);
    int numTextX = numBoxX + (NUM_BOX_SIZE - (int)strlen(numStr) * 12) / 2;
    int numTextY = numBoxY + (NUM_BOX_SIZE - 16) / 2; // = numBoxY + 4
    _tft.setTextColor(textColor, COL_BG);
    _tft.setTextSize(2);
    _tft.setCursor(numTextX, numTextY);
    _tft.print(numStr);

    // Label text, size 2, vertically centred in row
    int labelX = NUM_COL_W + 4;         // = 34
    int labelY = y + (ROW_H - 16) / 2; // = y + 18
    _tft.setTextColor(textColor, COL_BG);
    _tft.setCursor(labelX, labelY);
    _tft.print(labelFor(absIndex));
}

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
