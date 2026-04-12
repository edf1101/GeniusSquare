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

// ---- Input (stubs — implemented in Task 5) ----

void ListMenuScreen::onEncoderChange(int delta) {
    (void)delta;
}

void ListMenuScreen::onButtonPress() {}

// ---- Drawing (stubs — implemented in Task 4) ----

void ListMenuScreen::drawTitleBar() {}

void ListMenuScreen::drawRowArea() {}

void ListMenuScreen::drawRow(uint8_t screenRow, uint8_t absIndex, bool selected) {
    (void)screenRow; (void)absIndex; (void)selected;
}
