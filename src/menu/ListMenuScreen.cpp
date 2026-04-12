/*
 * Created by Ed Fillingham on 12/04/2026.
 *
 * Scrolling list menu screen implementation.
 */

#include "menu/ListMenuScreen.h"
#include <math.h>
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

void ListMenuScreen::update() {
    unsigned long now = millis();
    float dt = (now - _lastMs) / 1000.0f;
    _lastMs = now;

    _borderPhase += BORDER_SPEED * dt;
    if (_borderPhase >= 2.0f * (float)M_PI) _borderPhase -= 2.0f * (float)M_PI;

    _borderEnvelope = min(_borderEnvelope + BORDER_INTRO_SPEED * dt, 1.0f);

    _dirty = true;
}

void ListMenuScreen::render() {
    if (!_dirty) return;
    _dirty = false;
    updateNumberBoxBorder();
}

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

    // Number box: NUM_BOX_SIZE×NUM_BOX_SIZE, centred in the NUM_COL_W left column
    int numBoxX = (NUM_COL_W - NUM_BOX_SIZE) / 2; // = 5
    int numBoxY = y + (ROW_H - NUM_BOX_SIZE) / 2;  // = y + 6

    // Row number text (1-based absolute position).
    // Uses NUM_TEXT_SIZE=3: each char is NUM_TEXT_SIZE*6=18px wide, NUM_TEXT_SIZE*8=24px tall.
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
    int labelX = NUM_COL_W + 8;         // = 58
    int labelY = y + (ROW_H - 16) / 2; // = y + 18
    _tft.setTextColor(textColor, COL_BG);
    _tft.setTextSize(2);
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

uint16_t ListMenuScreen::blendColor(uint16_t bg, uint16_t fg, float alpha) {
    uint8_t r = ((bg >> 11) & 0x1F) + (int)(((int)((fg >> 11) & 0x1F) - (int)((bg >> 11) & 0x1F)) * alpha);
    uint8_t g = ((bg >>  5) & 0x3F) + (int)(((int)((fg >>  5) & 0x3F) - (int)((bg >>  5) & 0x3F)) * alpha);
    uint8_t b = ((bg      ) & 0x1F) + (int)(((int)( fg        & 0x1F) - (int)( bg        & 0x1F)) * alpha);
    return (uint16_t)((r << 11) | (g << 5) | b);
}

void ListMenuScreen::drawNumberBoxBorder(int boxX, int boxY, float thickness, uint16_t colour) {
    int t = (int)roundf(thickness);
    for (int i = 0; i < t; i++) {
        _tft.drawRect(boxX - 1 - i, boxY - 1 - i,
                      NUM_BOX_SIZE + 2 + 2 * i,
                      NUM_BOX_SIZE + 2 + 2 * i,
                      colour);
    }
}

void ListMenuScreen::updateNumberBoxBorder() {
    int boxX = (NUM_COL_W - NUM_BOX_SIZE) / 2;
    int boxY = TITLE_H + (_selectedIndex - _viewStart) * ROW_H + (ROW_H - NUM_BOX_SIZE) / 2;

    float exactThickness = BORDER_T_MIN + (BORDER_T_MAX - BORDER_T_MIN) * (0.5f + 0.5f * sinf(_borderPhase));
    uint16_t borderColour = (_borderEnvelope >= 0.99f)
        ? COL_BORDER
        : blendColor(COL_BG, COL_BORDER, _borderEnvelope);

    // Early-exit: nothing changed since last frame
    if (fabsf(exactThickness - _lastBorderThickness) < 0.01f && borderColour == _lastBorderColor) return;

    // Too faded: clear old border (if any) and bail
    if (_borderEnvelope < 0.02f) {
        if (_lastBorderThickness >= 0.0f) {
            drawNumberBoxBorder(boxX, boxY, _lastBorderThickness, COL_BG);
        }
        _lastBorderThickness = -1.0f;
        _lastBorderColor     = 0;
        return;
    }

    // First visible frame: draw without erase pass
    if (_lastBorderThickness < 0.0f) {
        drawNumberBoxBorder(boxX, boxY, exactThickness, borderColour);
    } else {
        // Erase old ring, draw new ring — prevents ghost pixels when thickness changes
        drawNumberBoxBorder(boxX, boxY, _lastBorderThickness, COL_BG);
        drawNumberBoxBorder(boxX, boxY, exactThickness, borderColour);
    }

    _lastBorderThickness = exactThickness;
    _lastBorderColor     = borderColour;
}
