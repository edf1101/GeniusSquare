# ListMenuScreen Glow Border & Layout Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a pulsing animated glow border to the selected row's number box in `ListMenuScreen`, and update the layout so numbers are larger and everything shifts right.

**Architecture:** All changes are confined to `ListMenuScreen.h` and `ListMenuScreen.cpp`. Animation state (`_borderPhase`, `_borderEnvelope`, etc.) lives in `ListMenuScreen` and is driven by `update()`/`render()` every loop. The border redraws only when thickness or colour changes (same efficiency strategy as `CarouselMenuScreen`). On selection change, the envelope resets to zero so the glow fades in fresh on the new row.

**Tech Stack:** C++17, Arduino framework, TFT_eSPI, PlatformIO (`pio run` to build).

---

## File Map

| File | Action | Change |
|---|---|---|
| `src/menu/ListMenuScreen.h` | Modify | Layout constants, animation constants, new fields, new method declarations |
| `src/menu/ListMenuScreen.cpp` | Modify | `drawRow()` layout, `update()`, `render()`, new helpers, reset logic |

---

## Task 1: Update `ListMenuScreen.h` — layout constants, animation state, method declarations

**Files:**
- Modify: `src/menu/ListMenuScreen.h`

- [ ] **Step 1: Replace the entire header with this updated version**

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

    // ---- Animation state ----
    float         _borderPhase;          ///< Sine wave phase (radians)
    float         _borderEnvelope;       ///< Fade-in envelope [0.0, 1.0]
    float         _lastBorderThickness;  ///< -1.0 = no border currently drawn
    uint16_t      _lastBorderColor;      ///< Colour from previous frame (change detection)
    unsigned long _lastMs;               ///< Timestamp of last update() call
    bool          _dirty;                ///< True when render() should call updateNumberBoxBorder()

    // ---- Layout constants (landscape 280×240) ----
    static constexpr int SCR_W        = 280;
    static constexpr int SCR_H        = 240;
    static constexpr int TITLE_H      = 30;
    static constexpr int ROW_H        = (SCR_H - TITLE_H) / VISIBLE_ROWS; ///< 52px; 2px gap at screen bottom is intentional (black bg, invisible)
    static constexpr int NUM_TEXT_SIZE = 3;   ///< TFT_eSPI text scale for row numbers (18×24px per char)
    static constexpr int NUM_COL_W    = 50;   ///< Width of the left number column
    static constexpr int NUM_BOX_SIZE = 40;   ///< Side length of the number box square

    // ---- Border animation constants ----
    static constexpr float    BORDER_T_MIN       = 1.0f;   ///< Minimum glow border thickness (px)
    static constexpr float    BORDER_T_MAX       = 3.0f;   ///< Maximum glow border thickness (px)
    static constexpr float    BORDER_SPEED       = 4.0f;   ///< Phase advance rate (rad/sec)
    static constexpr float    BORDER_INTRO_SPEED = 8.0f;   ///< Envelope fade-in rate (~125ms to full)

    // ---- Colour palette (RGB565) ----
    static constexpr uint16_t COL_BG         = 0x0000; ///< Background — black
    static constexpr uint16_t COL_TEXT_SEL   = 0xFFFF; ///< Selected row text — white
    static constexpr uint16_t COL_TEXT_UNSEL = 0xAD75; ///< Unselected row text — grey
    static constexpr uint16_t COL_DIVIDER    = 0x2965; ///< Row divider line — dark grey
    static constexpr uint16_t COL_BORDER     = 0xFFFF; ///< Animated glow border — white
    static constexpr uint16_t COL_TITLE      = 0xAD75; ///< Title bar text — light grey

    /** @brief Total navigable items including the auto-injected Back entry. */
    uint16_t totalItems() const;

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

    /**
     * @brief Update the animated glow border on the selected row's number box.
     *        Only issues TFT commands when thickness or colour has changed.
     *        Called from render() every frame.
     */
    void updateNumberBoxBorder();

    /**
     * @brief Draw (or erase) the glow border around the number box.
     * @param boxX      Left edge of the number box.
     * @param boxY      Top edge of the number box.
     * @param thickness Float thickness; drawn as round(thickness) concentric drawRect calls.
     * @param colour    Border colour (use COL_BG to erase).
     */
    void drawNumberBoxBorder(int boxX, int boxY, float thickness, uint16_t colour);

    /**
     * @brief Blend two RGB565 colours by linear interpolation.
     * @param bg    Background colour.
     * @param fg    Foreground colour.
     * @param alpha Blend factor [0.0, 1.0].
     * @return Blended colour.
     */
    uint16_t blendColor(uint16_t bg, uint16_t fg, float alpha);
};

#endif // LIST_MENU_SCREEN_H
```

- [ ] **Step 2: Verify it builds**

```bash
pio run
```

Expected: `SUCCESS`. (The .cpp still has old implementations — they will compile against the updated header because the signatures for all existing methods are unchanged.)

- [ ] **Step 3: Commit**

```bash
git add src/menu/ListMenuScreen.h
git commit -m "feat: update ListMenuScreen header — layout, animation state, new method declarations"
```

---

## Task 2: Update `drawRow()` for new layout and remove static box outline

**Files:**
- Modify: `src/menu/ListMenuScreen.cpp`

The static `drawRect` box outline is removed from `drawRow()` — the animated glow border (rendered by `updateNumberBoxBorder()`) replaces it. Number text uses `NUM_TEXT_SIZE = 3`. Label shifts right to `NUM_COL_W + 8 = 58`.

Also add `#include <math.h>` at the top of the file (needed for `sinf`, `fabsf`, `roundf` in Tasks 3–5).

- [ ] **Step 1: Add `#include <math.h>` after the existing includes**

Find:
```cpp
#include "menu/ListMenuScreen.h"
#include <stdio.h>
#include <string.h>
```

Replace with:
```cpp
#include "menu/ListMenuScreen.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
```

- [ ] **Step 2: Replace `drawRow()` with the updated version**

Find and replace the entire `drawRow` function:

```cpp
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
```

- [ ] **Step 3: Verify it builds**

```bash
pio run
```

Expected: `SUCCESS`.

- [ ] **Step 4: Commit**

```bash
git add src/menu/ListMenuScreen.cpp
git commit -m "feat: update drawRow — larger numbers, shifted layout, remove static box outline"
```

---

## Task 3: Implement `blendColor()` and `drawNumberBoxBorder()`

**Files:**
- Modify: `src/menu/ListMenuScreen.cpp`

Add both helper functions at the end of the file, after `drawRowArea()`.

- [ ] **Step 1: Append `blendColor()` and `drawNumberBoxBorder()` to the end of `ListMenuScreen.cpp`**

```cpp
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
```

- [ ] **Step 2: Verify it builds**

```bash
pio run
```

Expected: `SUCCESS`.

- [ ] **Step 3: Commit**

```bash
git add src/menu/ListMenuScreen.cpp
git commit -m "feat: add blendColor and drawNumberBoxBorder helpers to ListMenuScreen"
```

---

## Task 4: Implement `updateNumberBoxBorder()`

**Files:**
- Modify: `src/menu/ListMenuScreen.cpp`

Add `updateNumberBoxBorder()` after `drawNumberBoxBorder()`. This is the core animation logic — called from `render()` every frame, but only issues TFT commands when something changed.

- [ ] **Step 1: Append `updateNumberBoxBorder()` to the end of `ListMenuScreen.cpp`**

```cpp
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
```

- [ ] **Step 2: Verify it builds**

```bash
pio run
```

Expected: `SUCCESS`.

- [ ] **Step 3: Commit**

```bash
git add src/menu/ListMenuScreen.cpp
git commit -m "feat: implement updateNumberBoxBorder animation logic"
```

---

## Task 5: Implement `update()` and `render()`

**Files:**
- Modify: `src/menu/ListMenuScreen.cpp`

Replace the two empty stubs. `update()` advances the animation clock; `render()` calls `updateNumberBoxBorder()` when dirty.

- [ ] **Step 1: Replace the `update()` stub**

Find:
```cpp
void ListMenuScreen::update() {}
```

Replace with:
```cpp
void ListMenuScreen::update() {
    unsigned long now = millis();
    float dt = (now - _lastMs) / 1000.0f;
    _lastMs = now;

    _borderPhase += BORDER_SPEED * dt;
    if (_borderPhase >= 2.0f * (float)M_PI) _borderPhase -= 2.0f * (float)M_PI;

    _borderEnvelope = min(_borderEnvelope + BORDER_INTRO_SPEED * dt, 1.0f);

    _dirty = true;
}
```

- [ ] **Step 2: Replace the `render()` stub**

Find:
```cpp
void ListMenuScreen::render() {}
```

Replace with:
```cpp
void ListMenuScreen::render() {
    if (!_dirty) return;
    _dirty = false;
    updateNumberBoxBorder();
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
git commit -m "feat: implement update() and render() for ListMenuScreen border animation"
```

---

## Task 6: Initialise and reset animation state — constructor, `onEnter()`, `onResume()`, `onEncoderChange()`

**Files:**
- Modify: `src/menu/ListMenuScreen.cpp`

Wire the new animation fields into the four places that need to initialise or reset them.

- [ ] **Step 1: Update the constructor member initialiser list**

Find:
```cpp
ListMenuScreen::ListMenuScreen(TFT_eSPI& tft, ScreenManager& manager,
                               const ListItem* items, uint8_t count,
                               const char* title)
    : _tft(tft), _manager(manager), _items(items), _count(count), _title(title),
      _selectedIndex(0), _viewStart(0)
{
}
```

Replace with:
```cpp
ListMenuScreen::ListMenuScreen(TFT_eSPI& tft, ScreenManager& manager,
                               const ListItem* items, uint8_t count,
                               const char* title)
    : _tft(tft), _manager(manager), _items(items), _count(count), _title(title),
      _selectedIndex(0), _viewStart(0),
      _borderPhase(0.0f), _borderEnvelope(0.0f), _lastBorderThickness(-1.0f),
      _lastBorderColor(0), _lastMs(0), _dirty(false)
{
}
```

- [ ] **Step 2: Update `onEnter()` to reset animation state**

Find:
```cpp
void ListMenuScreen::onEnter() {
    _selectedIndex = 0;
    _viewStart     = 0;
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawRowArea();
}
```

Replace with:
```cpp
void ListMenuScreen::onEnter() {
    _selectedIndex       = 0;
    _viewStart           = 0;
    _borderPhase         = 0.0f;
    _borderEnvelope      = 0.0f;
    _lastBorderThickness = -1.0f;
    _lastBorderColor     = 0;
    _lastMs              = millis();
    _dirty               = false;
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawRowArea();
}
```

- [ ] **Step 3: Update `onResume()` to reset animation state**

Find:
```cpp
void ListMenuScreen::onResume() {
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawRowArea();
}
```

Replace with:
```cpp
void ListMenuScreen::onResume() {
    _borderPhase         = 0.0f;
    _borderEnvelope      = 0.0f;
    _lastBorderThickness = -1.0f;
    _lastBorderColor     = 0;
    _lastMs              = millis();
    _dirty               = false;
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawRowArea();
}
```

- [ ] **Step 4: Update `onEncoderChange()` to reset animation state on selection change**

Find:
```cpp
    if (_selectedIndex == prevIndex) return; // already at boundary, nothing to redraw

    // Scroll viewport to keep selected item visible
    if (_selectedIndex < _viewStart) {
        _viewStart = _selectedIndex;
    } else if (_selectedIndex >= _viewStart + VISIBLE_ROWS) {
        _viewStart = _selectedIndex - VISIBLE_ROWS + 1;
    }

    drawRowArea();
```

Replace with:
```cpp
    if (_selectedIndex == prevIndex) return; // already at boundary, nothing to redraw

    // Scroll viewport to keep selected item visible
    if (_selectedIndex < _viewStart) {
        _viewStart = _selectedIndex;
    } else if (_selectedIndex >= _viewStart + VISIBLE_ROWS) {
        _viewStart = _selectedIndex - VISIBLE_ROWS + 1;
    }

    // Reset border so it fades in fresh on the new row
    _borderEnvelope      = 0.0f;
    _borderPhase         = 0.0f;
    _lastBorderThickness = -1.0f;

    drawRowArea();
```

- [ ] **Step 5: Verify it builds**

```bash
pio run
```

Expected: `SUCCESS`.

- [ ] **Step 6: Flash and smoke-test on hardware**

```bash
pio run --target upload && pio device monitor
```

Manual checks:
- Main menu appears. Navigate to Solver, press button → list screen appears.
- Numbers are larger (size 3), number column and labels are shifted further right.
- On entry, the selected row's number box border fades in smoothly (~125ms).
- Border pulses continuously once fully faded in (thickness oscillates between 1–3px).
- Scroll down — border disappears on the old row instantly (cleared by drawRowArea), fades in on the new row.
- Scroll to boundary (Back or last item) and hold encoder — no stutter (early-return fires).
- Press Back → returns to main carousel.

- [ ] **Step 7: Commit**

```bash
git add src/menu/ListMenuScreen.cpp
git commit -m "feat: wire animation state init/reset into constructor, onEnter, onResume, onEncoderChange"
```

---

## Self-Review

**Spec coverage:**

| Spec requirement | Task |
|---|---|
| `NUM_TEXT_SIZE = 3` (larger numbers) | Task 1 (header), Task 2 (drawRow) |
| `NUM_BOX_SIZE = 40`, `NUM_COL_W = 50` (shifted right) | Task 1 |
| `labelX = NUM_COL_W + 8 = 58` | Task 2 |
| Border animation constants (`BORDER_T_MIN/MAX`, `BORDER_SPEED`, `BORDER_INTRO_SPEED`) | Task 1 |
| Animation state fields | Task 1 |
| `blendColor()` helper | Task 3 |
| `drawNumberBoxBorder()` helper | Task 3 |
| `updateNumberBoxBorder()` — efficiency (only redraw on change) | Task 4 |
| `update()` advances phase + envelope | Task 5 |
| `render()` dispatches to `updateNumberBoxBorder()` | Task 5 |
| Fade-in resets on selection change | Task 6 (`onEncoderChange`) |
| Animation state reset on `onEnter` / `onResume` | Task 6 |
| Constructor initialises all new fields | Task 6 |
| Static `drawRect` box outline removed (replaced by animated border) | Task 2 |
| `VISIBLE_ROWS` restored to 4 | Task 1 |

**Placeholder scan:** None found. All code blocks are complete.

**Type consistency:**
- `drawNumberBoxBorder(int, int, float, uint16_t)` — declared in Task 1, implemented in Task 3, called in Task 4. ✓
- `updateNumberBoxBorder()` — declared in Task 1, implemented in Task 4, called in Task 5 (`render()`). ✓
- `blendColor(uint16_t, uint16_t, float)` — declared in Task 1, implemented in Task 3, called in Task 4. ✓
- `_lastBorderThickness` set to `-1.0f` in Task 1 (constructor) and Task 6; read in Task 4. ✓
- `_borderEnvelope` initialised to `0.0f` in constructor (Task 6) and reset in Task 6; advanced in Task 5. ✓
