# ArrangementMenuScreen Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add `ArrangementMenuScreen` — a 2-row menu screen that displays a blocker-grid preview and score info per row — plus extract shared border animation into `BorderAnimator`.

**Architecture:** Extract the animated glow-border state and drawing helpers from `ListMenuScreen` into a standalone `BorderAnimator` struct; refactor `ListMenuScreen` to use it; then build `ArrangementMenuScreen` (inherits `Screen` directly) using the same `BorderAnimator` and the same scroll/dirty-flag patterns. The new screen enforces exactly 2 arrangement items and auto-injects a Back row.

**Tech Stack:** C++17, ESP32-S3, Arduino/PlatformIO, TFT_eSPI, `src/utils/math/maths.h` (provides `Coord`)

---

## File Map

| Action | Path | Responsibility |
|--------|------|----------------|
| Create | `src/menu/BorderAnimator.h` | Animation state + static draw helpers (no TFT stored) |
| Create | `src/menu/BorderAnimator.cpp` | Implementations of `advance()`, `blendColor()`, draw helpers |
| Modify | `src/menu/ListMenuScreen.h` | Replace 4 border vars with `BorderAnimator _border` |
| Modify | `src/menu/ListMenuScreen.cpp` | Route border calls through `_border` and static helpers |
| Create | `src/menu/ArrangementItem.h` | `Difficulty` enum + `ArrangementItem` struct |
| Create | `src/menu/ArrangementMenuScreen.h` | Class declaration |
| Create | `src/menu/ArrangementMenuScreen.cpp` | Full implementation |

---

## Task 1: Create `BorderAnimator.h`

**Files:**
- Create: `src/menu/BorderAnimator.h`

- [ ] **Step 1: Write the header**

```cpp
/*
 * Created by Ed Fillingham on 17/04/2026.
 *
 * Shared border animation state and drawing helpers for menu screens.
 * Extracts the animated glow-border logic from ListMenuScreen so multiple
 * screens can reuse it without duplication.
 */

#ifndef BORDER_ANIMATOR_H
#define BORDER_ANIMATOR_H

#include <TFT_eSPI.h>
#include <math.h>

/**
 * @brief Manages animated glow-border state for a selected menu row.
 *
 * Holds phase/envelope state. Call advance() each update tick.
 * Use the static draw helpers to render the border to the TFT.
 */
struct BorderAnimator {
    float    phase         = 0.f;   ///< Sine-wave phase (radians)
    float    envelope      = 0.f;   ///< Fade-in envelope [0.0, 1.0]
    float    lastThickness = -1.0f; ///< Thickness drawn last frame (-1 = nothing drawn)
    uint16_t lastColor     = 0;     ///< Colour drawn last frame

    /**
     * @brief Advance phase and fade envelope toward 1.
     * @param dt Delta time in seconds since last call.
     */
    void advance(float dt);

    /**
     * @brief Linear interpolation between two RGB565 colours.
     * @param bg    Background colour.
     * @param fg    Foreground colour.
     * @param alpha Blend factor [0.0, 1.0].
     * @return Blended RGB565 colour.
     */
    static uint16_t blendColor(uint16_t bg, uint16_t fg, float alpha);

    /**
     * @brief Draw a uniform-thickness border with rounded corners around a box.
     *        Integer part of rExact = solid fill; fractional part = 1px AA fringe.
     *        The AA fringe cleans up ghost pixels on shrink — no erase pass needed.
     * @param tft      TFT display reference.
     * @param x        Box left edge.
     * @param y        Box top edge.
     * @param w        Box width.
     * @param h        Box height.
     * @param rExact   Fractional border thickness.
     * @param colour   Border colour (pass bgColour to erase).
     * @param bgColour Background colour used for AA fringe blending.
     */
    static void drawBorderWithRoundedCorners(TFT_eSPI& tft,
                                              int x, int y, int w, int h,
                                              float rExact, uint16_t colour,
                                              uint16_t bgColour);

    /**
     * @brief Fill a quarter-disc at a box corner (used by drawBorderWithRoundedCorners).
     * @param tft      TFT display reference.
     * @param cx       Corner x coordinate.
     * @param cy       Corner y coordinate.
     * @param quadrant 0=TL, 1=TR, 2=BR, 3=BL.
     * @param r        Disc radius (integer border thickness).
     * @param colour   Fill colour.
     */
    static void drawQuarterDisc(TFT_eSPI& tft,
                                 int cx, int cy, int quadrant,
                                 int r, uint16_t colour);

    // Animation constants — shared by all screens
    static constexpr float BORDER_T_MIN       = 1.0f;
    static constexpr float BORDER_T_MAX       = 3.0f;
    static constexpr float BORDER_SPEED       = 4.0f;   ///< rad/sec
    static constexpr float BORDER_INTRO_SPEED = 8.0f;   ///< envelope fade-in rate
    static constexpr float BORDER_CHANGE_TOL  = 0.01f;  ///< min change to trigger redraw
};

#endif // BORDER_ANIMATOR_H
```

---

## Task 2: Create `BorderAnimator.cpp`

**Files:**
- Create: `src/menu/BorderAnimator.cpp`

- [ ] **Step 1: Write the implementation**

```cpp
/*
 * Created by Ed Fillingham on 17/04/2026.
 *
 * BorderAnimator — shared glow-border animation for menu screens.
 */

#include "menu/BorderAnimator.h"

void BorderAnimator::advance(float dt) {
    phase += BORDER_SPEED * dt;
    if (phase >= 2.0f * (float)M_PI) phase -= 2.0f * (float)M_PI;
    envelope = fminf(envelope + BORDER_INTRO_SPEED * dt, 1.0f);
}

uint16_t BorderAnimator::blendColor(uint16_t bg, uint16_t fg, float alpha) {
    uint8_t r = ((bg >> 11) & 0x1F) + (int)(((int)((fg >> 11) & 0x1F) - (int)((bg >> 11) & 0x1F)) * alpha);
    uint8_t g = ((bg >>  5) & 0x3F) + (int)(((int)((fg >>  5) & 0x3F) - (int)((bg >>  5) & 0x3F)) * alpha);
    uint8_t b = ((bg      ) & 0x1F) + (int)(((int)( fg        & 0x1F) - (int)( bg        & 0x1F)) * alpha);
    return (uint16_t)((r << 11) | (g << 5) | b);
}

void BorderAnimator::drawQuarterDisc(TFT_eSPI& tft, int cx, int cy, int quadrant, int r, uint16_t colour) {
    for (int d = 0; d < r; d++) {
        int span = (int)sqrtf((float)(r * r - d * d));
        if (span <= 0) continue;
        switch (quadrant) {
            case 0: tft.drawFastHLine(cx - span, cy - 1 - d, span, colour); break;
            case 1: tft.drawFastHLine(cx,         cy - 1 - d, span, colour); break;
            case 2: tft.drawFastHLine(cx,         cy + d,     span, colour); break;
            case 3: tft.drawFastHLine(cx - span,  cy + d,     span, colour); break;
        }
    }
}

void BorderAnimator::drawBorderWithRoundedCorners(TFT_eSPI& tft,
                                                   int x, int y, int w, int h,
                                                   float rExact, uint16_t colour,
                                                   uint16_t bgColour) {
    int r = (int)rExact;
    float frac = rExact - (float)r;
    uint16_t aaColour = blendColor(bgColour, colour, frac);

    if (r > 0) {
        tft.fillRect(x,     y - r, w,     r, colour);  // Top
        tft.fillRect(x + w, y,     r,     h, colour);  // Right
        tft.fillRect(x,     y + h, w,     r, colour);  // Bottom
        tft.fillRect(x - r, y,     r,     h, colour);  // Left
        drawQuarterDisc(tft, x,     y,     0, r, colour);  // TL
        drawQuarterDisc(tft, x + w, y,     1, r, colour);  // TR
        drawQuarterDisc(tft, x + w, y + h, 2, r, colour);  // BR
        drawQuarterDisc(tft, x,     y + h, 3, r, colour);  // BL
    }

    if (frac > 0.01f) {
        tft.drawFastHLine(x, y - r - 1, w, aaColour);
        tft.drawFastHLine(x, y + h + r, w, aaColour);
        tft.drawFastVLine(x - r - 1, y, h, aaColour);
        tft.drawFastVLine(x + w + r, y, h, aaColour);

        int ro = r + 1;
        for (int d = 0; d <= r; d++) {
            int spanO = (int)sqrtf((float)(ro * ro - d * d));
            int spanI = (d < r) ? (int)sqrtf((float)(r * r - d * d)) : 0;
            int diff  = spanO - spanI;
            if (diff <= 0) continue;
            tft.drawFastHLine(x     - spanO, y     - 1 - d, diff, aaColour);
            tft.drawFastHLine(x + w + spanI, y     - 1 - d, diff, aaColour);
            tft.drawFastHLine(x + w + spanI, y + h + d,     diff, aaColour);
            tft.drawFastHLine(x     - spanO, y + h + d,     diff, aaColour);
        }
    }
}
```

---

## Task 3: Refactor `ListMenuScreen.h` to use `BorderAnimator`

**Files:**
- Modify: `src/menu/ListMenuScreen.h`

- [ ] **Step 1: Replace the 4 border member variables and related constants/methods**

In `ListMenuScreen.h`:

1. Add `#include "menu/BorderAnimator.h"` below the existing includes.

2. In the `private:` section, replace:
```cpp
    // ---- Animation state ----
    float         _borderPhase;          ///< Sine wave phase (radians)
    float         _borderEnvelope;       ///< Fade-in envelope [0.0, 1.0]
    float         _lastBorderThickness;  ///< -1.0 = no border currently drawn
    uint16_t      _lastBorderColor;      ///< Colour from previous frame (change detection)
```
with:
```cpp
    // ---- Animation state ----
    BorderAnimator _border;  ///< Animated glow border for the selected row
```

3. Remove the border animation constants block:
```cpp
    // ---- Border animation constants ----
    static constexpr float    BORDER_T_MIN       = 1.0f;
    static constexpr float    BORDER_T_MAX       = 3.0f;
    static constexpr float    BORDER_SPEED       = 4.0f;
    static constexpr float    BORDER_INTRO_SPEED = 8.0f;
```
(These are now `BorderAnimator::BORDER_T_MIN` etc.)

4. Remove the three private method declarations:
```cpp
    void drawBorderWithRoundedCorners(int x, int y, int w, int h, float rExact, uint16_t colour);
    void drawQuarterDisc(int cx, int cy, int quadrant, int r, uint16_t colour);
    uint16_t blendColor(uint16_t bg, uint16_t fg, float alpha);
```
(These are now static methods on `BorderAnimator`.)

---

## Task 4: Refactor `ListMenuScreen.cpp` to use `BorderAnimator`

**Files:**
- Modify: `src/menu/ListMenuScreen.cpp`

- [ ] **Step 1: Update the constructor initialiser list**

Replace:
```cpp
      _borderPhase(0.0f), _borderEnvelope(0.0f), _lastBorderThickness(-1.0f),
      _lastBorderColor(0), _lastMs(millis()), _dirty(false)
```
with:
```cpp
      _lastMs(millis()), _dirty(false)
```
(`_border` default-initialises correctly: phase=0, envelope=0, lastThickness=-1, lastColor=0.)

- [ ] **Step 2: Update `onEnter()` and `onResume()`**

In both `onEnter()` and `onResume()`, replace:
```cpp
    _borderPhase         = 0.0f;
    _borderEnvelope      = 0.0f;
    _lastBorderThickness = -1.0f;
    _lastBorderColor     = 0;
```
with:
```cpp
    _border.phase         = 0.0f;
    _border.envelope      = 0.0f;
    _border.lastThickness = -1.0f;
    _border.lastColor     = 0;
```

- [ ] **Step 3: Update `update()`**

Replace the phase-advance and envelope-fade block:
```cpp
    // Advance sine wave phase for border thickness animation (4 rad/sec).
    _borderPhase += BORDER_SPEED * dt;
    if (_borderPhase >= 2.0f * (float)M_PI) _borderPhase -= 2.0f * (float)M_PI;

    // Fade in border envelope from 0 to 1 (~125ms to full opacity).
    // Once at 1.0, stays at full opacity while screen is active.
    _borderEnvelope = min(_borderEnvelope + BORDER_INTRO_SPEED * dt, 1.0f);
```
with:
```cpp
    _border.advance(dt);
```

- [ ] **Step 4: Update `onEncoderChange()`**

Replace:
```cpp
    _borderEnvelope      = 0.0f;
    _borderPhase         = 0.0f;
    _lastBorderThickness = -1.0f;
```
with:
```cpp
    _border.envelope      = 0.0f;
    _border.phase         = 0.0f;
    _border.lastThickness = -1.0f;
```

Also update the border-erase call from:
```cpp
            drawBorderWithRoundedCorners(oldBoxX, oldBoxY, NUM_BOX_SIZE, NUM_BOX_SIZE, _lastBorderThickness, COL_BG);
```
to:
```cpp
            BorderAnimator::drawBorderWithRoundedCorners(_tft, oldBoxX, oldBoxY, NUM_BOX_SIZE, NUM_BOX_SIZE, _border.lastThickness, COL_BG, COL_BG);
```

- [ ] **Step 5: Update `updateNumberBoxBorder()`**

Replace field accesses and local calls:

| Old | New |
|-----|-----|
| `BORDER_T_MIN` | `BorderAnimator::BORDER_T_MIN` |
| `BORDER_T_MAX` | `BorderAnimator::BORDER_T_MAX` |
| `_borderPhase` | `_border.phase` |
| `_borderEnvelope` | `_border.envelope` |
| `_lastBorderThickness` | `_border.lastThickness` |
| `_lastBorderColor` | `_border.lastColor` |
| `blendColor(COL_BG, COL_BORDER, _borderEnvelope)` | `BorderAnimator::blendColor(COL_BG, COL_BORDER, _border.envelope)` |
| `drawBorderWithRoundedCorners(boxX, boxY, NUM_BOX_SIZE, NUM_BOX_SIZE, _lastBorderThickness, COL_BG)` | `BorderAnimator::drawBorderWithRoundedCorners(_tft, boxX, boxY, NUM_BOX_SIZE, NUM_BOX_SIZE, _border.lastThickness, COL_BG, COL_BG)` |
| `drawBorderWithRoundedCorners(boxX, boxY, NUM_BOX_SIZE, NUM_BOX_SIZE, exactThickness, borderColour)` | `BorderAnimator::drawBorderWithRoundedCorners(_tft, boxX, boxY, NUM_BOX_SIZE, NUM_BOX_SIZE, exactThickness, borderColour, COL_BG)` |

- [ ] **Step 6: Delete the three method bodies**

Remove the complete implementations of:
- `ListMenuScreen::blendColor()`
- `ListMenuScreen::drawQuarterDisc()`
- `ListMenuScreen::drawBorderWithRoundedCorners()`

---

## Task 5: Build to verify ListMenuScreen refactor

**Files:** (none created)

- [ ] **Step 1: Run the build**

```bash
pio run
```

Expected: clean compile, zero errors and zero warnings about undefined references or missing members.

If it fails, trace the error back to a missed field rename in Tasks 3–4 and fix it.

- [ ] **Step 2: Commit**

```bash
git add src/menu/BorderAnimator.h src/menu/BorderAnimator.cpp \
        src/menu/ListMenuScreen.h src/menu/ListMenuScreen.cpp
git commit -m "refactor: extract BorderAnimator from ListMenuScreen"
```

---

## Task 6: Create `ArrangementItem.h`

**Files:**
- Create: `src/menu/ArrangementItem.h`

- [ ] **Step 1: Write the header**

```cpp
/*
 * Created by Ed Fillingham on 17/04/2026.
 *
 * Data types for ArrangementMenuScreen items.
 */

#ifndef ARRANGEMENT_ITEM_H
#define ARRANGEMENT_ITEM_H

#include <vector>
#include "utils/math/maths.h"  // Coord

class ScreenManager; // forward declaration

/**
 * @brief Puzzle difficulty level.
 */
enum class Difficulty : uint8_t { EASY, MED, HARD };

/**
 * @brief A single entry in an ArrangementMenuScreen.
 */
struct ArrangementItem {
    Difficulty          difficulty;   ///< Difficulty label displayed on the row
    float               seconds;      ///< Previous best score (0.0f = no score → "--:--")
    std::vector<Coord>  arrangement;  ///< Exactly 7 blocker coordinates for the grid preview
    void (*action)(ScreenManager&);   ///< Called when this item is confirmed
};

#endif // ARRANGEMENT_ITEM_H
```

---

## Task 7: Create `ArrangementMenuScreen.h`

**Files:**
- Create: `src/menu/ArrangementMenuScreen.h`

- [ ] **Step 1: Write the header**

```cpp
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
    static constexpr int GRID_X     = SCR_W - GRID_SZ - 8;        // 188 (left edge of grid)
    static constexpr int GRID_Y_OFF = (ROW_H - GRID_SZ) / 2;      // 10 (top padding within row)

    // ---- Colour palette (RGB565) ----
    static constexpr uint16_t COL_BG         = 0x0000; ///< Black background
    static constexpr uint16_t COL_TEXT_SEL   = 0xFFFF; ///< Selected row text — white
    static constexpr uint16_t COL_TEXT_UNSEL = 0xAD75; ///< Unselected row text — grey
    static constexpr uint16_t COL_DIVIDER    = 0x2965; ///< Row divider — dark grey
    static constexpr uint16_t COL_BORDER     = 0xFFFF; ///< Animated glow border — white
    static constexpr uint16_t COL_TITLE      = 0xAD75; ///< Title bar text — grey
    static constexpr uint16_t COL_GRID_LINE  = 0xFFFF; ///< Grid cell lines — white
    static constexpr uint16_t COL_BLOCKER    = 0x7BEF; ///< Blocker cell fill — light grey

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
     * @brief Update the animated glow border on the selected row.
     *        Delta-renders: only issues TFT commands when thickness/colour changed.
     */
    void updateSelectedBorder();
};

#endif // ARRANGEMENT_MENU_SCREEN_H
```

---

## Task 8: Create `ArrangementMenuScreen.cpp`

**Files:**
- Create: `src/menu/ArrangementMenuScreen.cpp`

- [ ] **Step 1: Write the full implementation**

```cpp
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
    _selectedIndex        = 0;
    _viewStart            = 0;
    _border.phase         = 0.0f;
    _border.envelope      = 0.0f;
    _border.lastThickness = -1.0f;
    _border.lastColor     = 0;
    _lastMs               = millis();
    _dirty                = false;
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
    _border.phase         = 0.0f;
    _border.envelope      = 0.0f;
    _border.lastThickness = -1.0f;
    _border.lastColor     = 0;
    _lastMs               = millis();
    _dirty                = false;
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
    _border.envelope      = 0.0f;
    _border.phase         = 0.0f;
    _border.lastThickness = -1.0f;

    if (_viewStart != prevViewStart) {
        drawRowArea();
    } else {
        int prevSlot = (int)prevIndex    - (int)_viewStart;
        int newSlot  = (int)_selectedIndex - (int)_viewStart;
        if (prevSlot >= 0 && prevSlot < (int)VISIBLE_ROWS)
            drawRow(prevIndex,     (uint8_t)prevSlot);
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
    // Divider between row 0 and row 1
    int divY = TITLE_H + ROW_H - 1;
    _tft.drawFastHLine(0, divY, SCR_W, COL_DIVIDER);

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
        int by = rowTop + (ROW_H - 16) / 2;   // Vertically centred (16px = text height)
        _tft.setCursor(bx, by);
        _tft.print("Back");
        return;
    }

    const ArrangementItem& item = _items[absIndex - 1];

    // Build text strings
    static const char* DIFF_LABELS[] = { "Easy", "Med", "Hard" };
    char line1[24];
    snprintf(line1, sizeof(line1), "Diff: %s", DIFF_LABELS[(uint8_t)item.difficulty]);

    char line2[24];
    if (item.seconds <= 0.0f) {
        snprintf(line2, sizeof(line2), "Score: --:--");
    } else {
        int total = (int)item.seconds;
        snprintf(line2, sizeof(line2), "Score: %02d:%02d", total / 60, total % 60);
    }

    // Two text lines, scale 2 (12×16px), vertically centred pair in row
    // Line pair height: 16 + 8 + 16 = 40px; top pad = (105-40)/2 = 32px
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
 * vertically centred in the row.
 *
 * @param rowTop  Pixel y of the row's top edge.
 * @param coords  Blocker coordinates to fill (each x/y in [0,5]).
 */
void ArrangementMenuScreen::drawArrangementGrid(int rowTop, const std::vector<Coord>& coords) {
    int gx = GRID_X;
    int gy = rowTop + GRID_Y_OFF;

    // Draw grid lines (7 horizontal + 7 vertical = 6×6 cells)
    for (int i = 0; i <= GRID_CELLS; i++) {
        _tft.drawFastVLine(gx + i * CELL_SZ, gy, GRID_SZ, COL_GRID_LINE);
        _tft.drawFastHLine(gx, gy + i * CELL_SZ, GRID_SZ, COL_GRID_LINE);
    }

    // Fill blocker cells (inset 1px to preserve grid lines)
    for (const Coord& c : coords) {
        int cx = gx + c.x * CELL_SZ + 1;
        int cy = gy + c.y * CELL_SZ + 1;
        _tft.fillRect(cx, cy, CELL_SZ - 1, CELL_SZ - 1, COL_BLOCKER);
    }
}

/**
 * @brief Update the animated glow border around the selected row.
 *
 * Delta-renders: only issues TFT commands when the computed thickness or colour
 * differs from last frame by more than BORDER_CHANGE_TOL (0.01).
 * The border outlines the full row rectangle (2px inset from row edges).
 */
void ArrangementMenuScreen::updateSelectedBorder() {
    int rowSlot = (int)_selectedIndex - (int)_viewStart;
    int rowTop  = TITLE_H + rowSlot * ROW_H;

    // Border box: full row, 2px inset
    int bx = 2;
    int by = rowTop + 2;
    int bw = SCR_W - 4;
    int bh = ROW_H - 4;

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
```

---

## Task 9: Build and commit

**Files:** (none created)

- [ ] **Step 1: Run the build**

```bash
pio run
```

Expected: clean compile, zero errors. Common issues to check if it fails:
- Missing `#include <assert.h>` — add to `.cpp` if needed
- `Coord` not found — confirm `#include "utils/math/maths.h"` resolves (check include path in `platformio.ini`)
- `fminf` / `fabsf` not found — add `#include <math.h>` if needed

- [ ] **Step 2: Commit**

```bash
git add src/menu/ArrangementItem.h \
        src/menu/ArrangementMenuScreen.h \
        src/menu/ArrangementMenuScreen.cpp
git commit -m "feat: add ArrangementMenuScreen with blocker grid preview"
```

---

## Task 10: Wire up a test in `main.cpp` and flash

- [ ] **Step 1: Add test items and screen to `main.cpp`**

Near the top of `main.cpp`, after existing includes:
```cpp
#include "menu/ArrangementMenuScreen.h"
```

In the global scope (alongside existing screen declarations), add:
```cpp
static ArrangementItem testArrangementItems[2] = {
    {
        Difficulty::EASY,
        87.5f,   // 1:27
        { {0,0},{1,2},{3,1},{5,4},{2,5},{4,3},{0,5} },
        [](ScreenManager& m) { /* placeholder */ }
    },
    {
        Difficulty::HARD,
        0.0f,    // no score yet
        { {1,0},{3,3},{5,1},{0,4},{2,2},{4,0},{3,5} },
        [](ScreenManager& m) { /* placeholder */ }
    }
};
static ArrangementMenuScreen arrangementScreen(tft, manager,
                                               testArrangementItems, 2,
                                               "Choose Arrangement");
```

Push the screen temporarily in `setup()` (or wire it to a menu item):
```cpp
manager.push(&arrangementScreen);
```

- [ ] **Step 2: Build and upload**

```bash
pio run --target upload && pio device monitor
```

- [ ] **Step 3: Verify on device**

Check each item in the list:
- [ ] Title bar shows "Choose Arrangement" in grey
- [ ] Row 0 shows "Back" centred; row 1 shows "Diff: Easy" + "Score: 01:27" + 7 grey blocker cells on a white 6×6 grid
- [ ] Row 2 (scroll down) shows "Diff: Hard" + "Score: --:--" + different blocker positions
- [ ] Selected row shows animated white glow border around the whole row (fades in over ~125ms)
- [ ] Encoder scrolls selection up/down; border moves to new row and fades in
- [ ] Button on Back calls `manager.pop()`; button on an arrangement row calls its action

- [ ] **Step 4: Test `setItems()` runtime update**

In `loop()`, after a 3-second delay, call:
```cpp
static bool updated = false;
if (!updated && millis() > 3000) {
    testArrangementItems[0].seconds = 45.0f;  // 0:45
    arrangementScreen.setItems(testArrangementItems, 2);
    updated = true;
}
```

Expected: score on row 1 updates from "Score: 01:27" to "Score: 00:45" within one render cycle.

- [ ] **Step 5: Remove test scaffolding from `main.cpp` and commit**

Revert the test additions from `main.cpp` (or replace with the real wiring if ready).

```bash
git add src/main.cpp
git commit -m "test: verify ArrangementMenuScreen on device"
```
