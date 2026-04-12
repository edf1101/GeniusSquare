# ListMenuScreen Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement a scrollable vertical list screen (`ListMenuScreen`) that handles large option sets, auto-injects a "Back" entry at index 0, snaps on scroll, and integrates cleanly into the existing `Screen`/`ScreenManager` stack.

**Architecture:** `ListItem` is a minimal label+callback struct (no bitmaps). `ListMenuScreen` implements the `Screen` interface identically to `CarouselMenuScreen` — no changes to any existing file except `main.cpp` for smoke-test wiring. The screen owns two state variables (`_selectedIndex`, `_viewStart`) and redraws synchronously on every input event.

**Tech Stack:** C++17, Arduino framework, TFT_eSPI, PlatformIO (`pio run` to build).

---

## File Map

| File | Action | Responsibility |
|---|---|---|
| `src/menu/ListItem.h` | Create | Plain data struct (label + callback) |
| `src/menu/ListMenuScreen.h` | Create | Class declaration, layout constants, `VISIBLE_ROWS` |
| `src/menu/ListMenuScreen.cpp` | Create | Full implementation |
| `src/main.cpp` | Modify | Include new headers, declare test list + wire Solver action |

---

## Task 1: Create `ListItem.h`

**Files:**
- Create: `src/menu/ListItem.h`

- [ ] **Step 1: Create the file**

```cpp
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
```

- [ ] **Step 2: Verify it builds**

```bash
pio run
```

Expected: `SUCCESS` (no new errors introduced by a header-only struct).

- [ ] **Step 3: Commit**

```bash
git add src/menu/ListItem.h
git commit -m "feat: add ListItem struct for scrolling list screen"
```

---

## Task 2: Create `ListMenuScreen.h`

**Files:**
- Create: `src/menu/ListMenuScreen.h`

- [ ] **Step 1: Create the header**

```cpp
/*
 * Created by Ed Fillingham on 12/04/2026.
 *
 * A Screen that displays a scrollable vertical list of options.
 * Suitable for large option sets (50+ items). The first entry is always
 * an auto-injected "Back" item that calls ScreenManager::pop().
 */

#ifndef LIST_MENU_SCREEN_H
#define LIST_MENU_SCREEN_H

#include <TFT_eSPI.h>
#include "menu/Screen.h"
#include "menu/ListItem.h"
#include "menu/ScreenManager.h"

class ListMenuScreen : public Screen {
public:
    /**
     * @brief Construct a ListMenuScreen.
     * @param tft     Reference to the shared TFT display.
     * @param manager Reference to the ScreenManager.
     * @param items   Pointer to real item array (Back not included). Must outlive this object.
     * @param count   Number of real items (not including Back).
     * @param title   Text displayed in the title bar.
     */
    ListMenuScreen(TFT_eSPI& tft, ScreenManager& manager,
                   const ListItem* items, uint8_t count,
                   const char* title);

    void onEnter() override;
    void onExit() override;
    void onResume() override;
    void update() override;
    void render() override;
    void onEncoderChange(int delta) override;
    void onButtonPress() override;

    /// Number of rows visible at once. Adjust here to change list density.
    static constexpr uint8_t VISIBLE_ROWS = 4;

private:
    TFT_eSPI&        _tft;
    ScreenManager&   _manager;
    const ListItem*  _items;   ///< Real items (Back not included)
    uint8_t          _count;   ///< Number of real items
    const char*      _title;

    uint8_t  _selectedIndex;   ///< Absolute index of highlighted item (0 = Back)
    uint8_t  _viewStart;       ///< Absolute index of first visible row

    // ---- Layout constants (landscape 280×240) ----
    static constexpr int SCR_W        = 280;
    static constexpr int SCR_H        = 240;
    static constexpr int TITLE_H      = 30;
    static constexpr int ROW_H        = (SCR_H - TITLE_H) / VISIBLE_ROWS; ///< 52px
    static constexpr int NUM_COL_W    = 30;  ///< Width of the left number column
    static constexpr int NUM_BOX_SIZE = 24;  ///< Side length of the number box square

    // ---- Colour palette (RGB565) ----
    static constexpr uint16_t COL_BG         = 0x0000; ///< Background — black
    static constexpr uint16_t COL_TEXT_SEL   = 0xFFFF; ///< Selected row text — white
    static constexpr uint16_t COL_TEXT_UNSEL = 0xAD75; ///< Unselected row text — grey
    static constexpr uint16_t COL_DIVIDER    = 0x2965; ///< Row divider line — dark grey
    static constexpr uint16_t COL_NUM_BOX    = 0xFFFF; ///< Number box outline — white
    static constexpr uint16_t COL_TITLE      = 0xAD75; ///< Title bar text — light grey

    /**
     * @brief Total navigable items including the auto-injected Back entry.
     */
    uint8_t totalItems() const;

    /**
     * @brief Return the display label for an absolute index.
     *        Index 0 returns "Back"; indices 1..count return _items[index-1].label.
     */
    const char* labelFor(uint8_t absIndex) const;

    /** @brief Draw the title bar. Called once on onEnter() and onResume(). */
    void drawTitleBar();

    /** @brief Redraw all visible rows and dividers from current _selectedIndex / _viewStart. */
    void drawRowArea();

    /**
     * @brief Draw a single row.
     * @param screenRow On-screen row index (0..VISIBLE_ROWS-1).
     * @param absIndex  Absolute item index this row represents.
     * @param selected  Whether this row is the highlighted selection.
     */
    void drawRow(uint8_t screenRow, uint8_t absIndex, bool selected);
};

#endif // LIST_MENU_SCREEN_H
```

- [ ] **Step 2: Verify it builds**

```bash
pio run
```

Expected: `SUCCESS`.

- [ ] **Step 3: Commit**

```bash
git add src/menu/ListMenuScreen.h
git commit -m "feat: add ListMenuScreen class declaration"
```

---

## Task 3: Implement `ListMenuScreen.cpp` — skeleton + lifecycle

**Files:**
- Create: `src/menu/ListMenuScreen.cpp`

- [ ] **Step 1: Create the file with constructor, helpers, and lifecycle methods**

```cpp
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

uint8_t ListMenuScreen::totalItems() const {
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

// ---- Input (stubs — implemented in Task 4) ----

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
```

- [ ] **Step 2: Verify it builds**

```bash
pio run
```

Expected: `SUCCESS` (stubs satisfy all pure-virtual overrides).

- [ ] **Step 3: Commit**

```bash
git add src/menu/ListMenuScreen.cpp
git commit -m "feat: add ListMenuScreen skeleton with lifecycle stubs"
```

---

## Task 4: Implement drawing — `drawTitleBar`, `drawRow`, `drawRowArea`

**Files:**
- Modify: `src/menu/ListMenuScreen.cpp`

Replace the three stub drawing methods with real implementations.

- [ ] **Step 1: Replace `drawTitleBar`**

```cpp
void ListMenuScreen::drawTitleBar() {
    _tft.setTextColor(COL_TITLE, COL_BG);
    _tft.setTextSize(2);
    int titleX = (SCR_W - (int)strlen(_title) * 12) / 2;
    _tft.setCursor(titleX, (TITLE_H - 16) / 2);
    _tft.print(_title);
}
```

- [ ] **Step 2: Replace `drawRow`**

```cpp
void ListMenuScreen::drawRow(uint8_t screenRow, uint8_t absIndex, bool selected) {
    int y = TITLE_H + screenRow * ROW_H;
    uint16_t textColor = selected ? COL_TEXT_SEL : COL_TEXT_UNSEL;

    // Clear the row
    _tft.fillRect(0, y, SCR_W, ROW_H, COL_BG);

    // Number box: 24×24px, centred in the 30px left column
    int numBoxX = (NUM_COL_W - NUM_BOX_SIZE) / 2; // = 3
    int numBoxY = y + (ROW_H - NUM_BOX_SIZE) / 2;  // = y + 14

    if (selected) {
        _tft.drawRect(numBoxX, numBoxY, NUM_BOX_SIZE, NUM_BOX_SIZE, COL_NUM_BOX);
    }

    // Row number text (1-based), size 2 = 12×16px per char
    char numStr[3];
    snprintf(numStr, sizeof(numStr), "%d", screenRow + 1);
    int numTextX = numBoxX + (NUM_BOX_SIZE - 12) / 2; // = numBoxX + 6
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
```

- [ ] **Step 3: Replace `drawRowArea`**

```cpp
void ListMenuScreen::drawRowArea() {
    for (uint8_t i = 0; i < VISIBLE_ROWS; i++) {
        uint8_t absIndex = _viewStart + i;
        if (absIndex < totalItems()) {
            drawRow(i, absIndex, absIndex == _selectedIndex);
        } else {
            // No item here — clear to background
            _tft.fillRect(0, TITLE_H + i * ROW_H, SCR_W, ROW_H, COL_BG);
        }
    }

    // Dividers between rows (VISIBLE_ROWS-1 lines, not after the last row)
    for (uint8_t i = 0; i < VISIBLE_ROWS - 1; i++) {
        int divY = TITLE_H + (i + 1) * ROW_H;
        _tft.drawFastHLine(0, divY, SCR_W, COL_DIVIDER);
    }
}
```

- [ ] **Step 4: Verify it builds**

```bash
pio run
```

Expected: `SUCCESS`.

- [ ] **Step 5: Commit**

```bash
git add src/menu/ListMenuScreen.cpp
git commit -m "feat: implement ListMenuScreen drawing (title bar, rows, dividers)"
```

---

## Task 5: Implement input handling — `onEncoderChange` and `onButtonPress`

**Files:**
- Modify: `src/menu/ListMenuScreen.cpp`

Replace the two input stubs.

- [ ] **Step 1: Replace `onEncoderChange`**

```cpp
void ListMenuScreen::onEncoderChange(int delta) {
    int newIndex = (int)_selectedIndex + delta;
    if (newIndex < 0)                    newIndex = 0;
    if (newIndex >= (int)totalItems())   newIndex = (int)totalItems() - 1;
    _selectedIndex = (uint8_t)newIndex;

    // Scroll viewport to keep selected item visible
    if (_selectedIndex < _viewStart) {
        _viewStart = _selectedIndex;
    } else if (_selectedIndex >= _viewStart + VISIBLE_ROWS) {
        _viewStart = _selectedIndex - VISIBLE_ROWS + 1;
    }

    drawRowArea();
}
```

- [ ] **Step 2: Replace `onButtonPress`**

```cpp
void ListMenuScreen::onButtonPress() {
    if (_selectedIndex == 0) {
        _manager.pop();
    } else {
        _items[_selectedIndex - 1].action(_manager);
    }
}
```

- [ ] **Step 3: Verify it builds**

```bash
pio run
```

Expected: `SUCCESS`.

- [ ] **Step 4: Commit**

```bash
git add src/menu/ListMenuScreen.cpp
git commit -m "feat: implement ListMenuScreen input handling (scroll + select)"
```

---

## Task 6: Wire into `main.cpp` for smoke testing

Wire the Solver carousel action to push a `ListMenuScreen` with a handful of test entries so the screen can be exercised on hardware.

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Add includes and test data to `main.cpp`**

After the existing `#include` block (after line 14), add:

```cpp
#include "menu/ListItem.h"
#include "menu/ListMenuScreen.h"
```

After the `mainMenuItems` array declaration (after line 37), add:

```cpp
ListItem solverListItems[] = {
    { "Arrangement A",  [](ScreenManager&){} },
    { "Arrangement B",  [](ScreenManager&){} },
    { "Arrangement C",  [](ScreenManager&){} },
    { "Arrangement D",  [](ScreenManager&){} },
    { "Arrangement E",  [](ScreenManager&){} },
    { "Arrangement F",  [](ScreenManager&){} },
};

ListMenuScreen solverList(tft, screenManager, solverListItems, 6, "Solver");
```

- [ ] **Step 2: Wire `onSolverSelected` to push the list screen**

Replace the existing stub:

```cpp
void onSolverSelected(ScreenManager& mgr) { /* push SolverScreen when implemented */ }
```

With:

```cpp
void onSolverSelected(ScreenManager& mgr) { mgr.push(&solverList); }
```

- [ ] **Step 3: Verify it builds**

```bash
pio run
```

Expected: `SUCCESS`.

- [ ] **Step 4: Flash and smoke-test on hardware**

```bash
pio run --target upload && pio device monitor
```

Manual checks:
- Main menu appears. Navigate to "Solver" and press button → list screen appears with title "Solver".
- Row 1 "Back" is highlighted on entry; number box is outlined in white.
- Scroll down through all 6 items + Back (7 total); highlight moves through visible rows before viewport scrolls.
- Scrolling past last item does nothing (no wrap).
- Scrolling past Back (index 0) does nothing (no wrap).
- Press button on "Back" → returns to main carousel.
- Press button on any real item → no crash (stub action does nothing).

- [ ] **Step 5: Commit**

```bash
git add src/main.cpp
git commit -m "feat: wire ListMenuScreen smoke test into Solver action"
```

---

## Self-Review

**Spec coverage:**

| Spec requirement | Task |
|---|---|
| Vertical list with numbered rows | Task 4 (`drawRow`) |
| Title bar at top | Task 4 (`drawTitleBar`) |
| 4 visible rows, configurable constant in .h | Task 2 (`VISIBLE_ROWS`) |
| Black bg, grey unselected text, white selected text | Task 4 |
| Faint dividers between rows | Task 4 (`drawRowArea`) |
| White box around selected number | Task 4 (`drawRow`) |
| Highlight moves in-viewport before scrolling | Task 5 (`onEncoderChange`) |
| No wrap-around at start/end | Task 5 (clamping) |
| Back always first, auto-injected, calls pop() | Task 3 (`totalItems`, `labelFor`), Task 5 (`onButtonPress`) |
| Edge case: only Back (count=0) | Covered by `drawRowArea` — rows 2-4 empty, row 1 shows "Back" selected |
| Snap (no animation) | No lerp state anywhere |

**Placeholder scan:** No TBDs. All code blocks are complete.

**Type consistency:**
- `totalItems()` returns `uint8_t`, used in comparisons with signed int via explicit cast in `onEncoderChange` — correct.
- `labelFor(uint8_t)` used in `drawRow` — matches declaration.
- `VISIBLE_ROWS` is `uint8_t`; loop variable `i` is `uint8_t` — consistent.
- `drawRow` signature `(uint8_t screenRow, uint8_t absIndex, bool selected)` matches declaration and all call sites.
