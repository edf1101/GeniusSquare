# Practice Mode Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement the two-phase practice game screen and results screen so players can select an arrangement, place blockers, solve the puzzle timed, and see their result.

**Architecture:** A `PracticeGameScreen` owns a two-phase state machine (`PLACING` → `PLAYING`), uses `GridScanner::digitalReadMatrix()` every 200 ms to drive both phases, and pushes `PracticeScoreScreen` on completion. `PracticeScoreScreen` shows elapsed vs. best time, updates the in-RAM best if beaten, and double-pops back to `ArrangementMenuScreen` on button press. Both are static objects in `main.cpp`; each `practiceItems[i].action` callback calls `setArrangement()` and pushes the game screen.

**Tech Stack:** C++17, Arduino/PlatformIO, TFT_eSPI (SPI display), `GridScanner::digitalReadMatrix()`, no EEPROM (RAM-only best times for now).

---

## File Map

| Action | Path | Responsibility |
|--------|------|----------------|
| Create | `src/menu/PracticeScoreScreen.h` | Results screen interface |
| Create | `src/menu/PracticeScoreScreen.cpp` | Results screen implementation |
| Create | `src/menu/PracticeGameScreen.h` | Game screen interface + layout constants |
| Create | `src/menu/PracticeGameScreen.cpp` | Two-phase game logic |
| Modify | `src/main.cpp` | Static screen objects + action callbacks |

---

## Task 1: `PracticeScoreScreen`

**Files:**
- Create: `src/menu/PracticeScoreScreen.h`
- Create: `src/menu/PracticeScoreScreen.cpp`

- [ ] **Step 1: Write `PracticeScoreScreen.h`**

```cpp
/*
 * Created by Ed Fillingham on 10/05/2026.
 *
 * Displays the result after a practice game: the player's time,
 * the current best for the arrangement, and a "New best!" banner if beaten.
 * A single button press pops both this screen and the game screen,
 * returning to the arrangement selection menu.
 */

#ifndef PRACTICE_SCORE_SCREEN_H
#define PRACTICE_SCORE_SCREEN_H

#include <TFT_eSPI.h>
#include <stdio.h>
#include "menu/Screen.h"
#include "menu/ScreenManager.h"

class PracticeScoreScreen : public Screen {
public:
    /**
     * @brief Construct the score screen.
     * @param tft     Shared display reference.
     * @param manager Shared ScreenManager reference.
     */
    PracticeScoreScreen(TFT_eSPI& tft, ScreenManager& manager);

    /**
     * @brief Set result data before pushing this screen.
     * @param elapsedSeconds Time the player took.
     * @param bestSeconds    Best time for this arrangement (already updated in RAM if new best).
     * @param isNewBest      True if elapsedSeconds beat the previous best.
     */
    void setResult(float elapsedSeconds, float bestSeconds, bool isNewBest);

    void onEnter()            override;
    void onExit()             override;
    void onResume()           override;
    void update()             override;
    void render()             override;
    void onEncoderChange(int delta) override;
    void onButtonPress()      override;

private:
    TFT_eSPI&      _tft;
    ScreenManager& _manager;

    float _elapsedSeconds;
    float _bestSeconds;
    bool  _isNewBest;

    static constexpr int SCR_W   = 280;
    static constexpr int SCR_H   = 240;
    static constexpr int TITLE_H = 30;

    static constexpr uint16_t COL_BG       = 0x0000;
    static constexpr uint16_t COL_TEXT     = 0xFFFF;
    static constexpr uint16_t COL_SUBTEXT  = 0xAD75;
    static constexpr uint16_t COL_TITLE    = 0xAD75;
    static constexpr uint16_t COL_NEW_BEST = 0x07E0;  ///< Green

    static constexpr int BTN_W = 140;
    static constexpr int BTN_H = 32;
    static constexpr int BTN_X = (SCR_W - BTN_W) / 2;
    static constexpr int BTN_Y = SCR_H - BTN_H - 20;

    void drawAll();
    void formatTime(float seconds, char* buf, size_t bufSize) const;
};

#endif // PRACTICE_SCORE_SCREEN_H
```

- [ ] **Step 2: Write `PracticeScoreScreen.cpp`**

```cpp
/*
 * Created by Ed Fillingham on 10/05/2026.
 *
 * PracticeScoreScreen — static results screen shown after completing a practice game.
 */

#include "menu/PracticeScoreScreen.h"
#include <string.h>

PracticeScoreScreen::PracticeScoreScreen(TFT_eSPI& tft, ScreenManager& manager)
    : _tft(tft), _manager(manager),
      _elapsedSeconds(0.0f), _bestSeconds(0.0f), _isNewBest(false)
{}

void PracticeScoreScreen::setResult(float elapsedSeconds, float bestSeconds, bool isNewBest) {
    _elapsedSeconds = elapsedSeconds;
    _bestSeconds    = bestSeconds;
    _isNewBest      = isNewBest;
}

void PracticeScoreScreen::onEnter() {
    _tft.fillScreen(COL_BG);
    drawAll();
}

void PracticeScoreScreen::onExit()   {}
void PracticeScoreScreen::onResume() { _tft.fillScreen(COL_BG); drawAll(); }
void PracticeScoreScreen::update()   {}
void PracticeScoreScreen::render()   {}
void PracticeScoreScreen::onEncoderChange(int) {}

void PracticeScoreScreen::onButtonPress() {
    _manager.pop(); // pop PracticeScoreScreen; onResume() fires on PracticeGameScreen
    _manager.pop(); // pop PracticeGameScreen;  onResume() fires on ArrangementMenuScreen
}

void PracticeScoreScreen::drawAll() {
    // Title bar
    _tft.setTextColor(COL_TITLE, COL_BG);
    _tft.setTextSize(2);
    const char* title = "Results";
    int tx = (SCR_W - (int)strlen(title) * 12) / 2;
    _tft.setCursor(tx, (TITLE_H - 16) / 2);
    _tft.print(title);

    // Divider
    _tft.drawFastHLine(0, TITLE_H, SCR_W, COL_SUBTEXT);

    char elapsedBuf[8], bestBuf[8];
    formatTime(_elapsedSeconds, elapsedBuf, sizeof(elapsedBuf));
    if (_bestSeconds <= 0.0f) {
        snprintf(bestBuf, sizeof(bestBuf), "--:--");
    } else {
        formatTime(_bestSeconds, bestBuf, sizeof(bestBuf));
    }

    // Body — vertically centred block of text
    // "Your time: MM:SS" and "Best: MM:SS" at size 2 (16px tall), 8px gap
    // Starting y so block sits centred in the space between title and button
    int bodyH = 16 + 8 + 16 + (_isNewBest ? (8 + 16) : 0);
    int bodyY = TITLE_H + (BTN_Y - TITLE_H - bodyH) / 2;

    char yourLine[24];
    snprintf(yourLine, sizeof(yourLine), "Your time: %s", elapsedBuf);
    char bestLine[24];
    snprintf(bestLine, sizeof(bestLine), "Best:      %s", bestBuf);

    _tft.setTextColor(COL_TEXT, COL_BG);
    _tft.setTextSize(2);

    int lineW = (int)strlen(yourLine) * 12;
    _tft.setCursor((SCR_W - lineW) / 2, bodyY);
    _tft.print(yourLine);

    lineW = (int)strlen(bestLine) * 12;
    _tft.setCursor((SCR_W - lineW) / 2, bodyY + 24);
    _tft.print(bestLine);

    if (_isNewBest) {
        _tft.setTextColor(COL_NEW_BEST, COL_BG);
        const char* nb = "New best!";
        lineW = (int)strlen(nb) * 12;
        _tft.setCursor((SCR_W - lineW) / 2, bodyY + 52);
        _tft.print(nb);
    }

    // Back button
    _tft.drawRoundRect(BTN_X, BTN_Y, BTN_W, BTN_H, 4, COL_TEXT);
    _tft.setTextColor(COL_TEXT, COL_BG);
    _tft.setTextSize(2);
    const char* btnLabel = "Main Menu";
    lineW = (int)strlen(btnLabel) * 12;
    _tft.setCursor(BTN_X + (BTN_W - lineW) / 2, BTN_Y + (BTN_H - 16) / 2);
    _tft.print(btnLabel);
}

void PracticeScoreScreen::formatTime(float seconds, char* buf, size_t bufSize) const {
    int total = (int)seconds;
    snprintf(buf, bufSize, "%02d:%02d", total / 60, total % 60);
}
```

- [ ] **Step 3: Build to verify compilation**

```bash
pio run
```

Expected: build succeeds. If `PracticeScoreScreen` references fail to resolve, check include paths in `platformio.ini` — the `src/` directory is already on the include path.

---

## Task 2: `PracticeGameScreen` header and skeleton

**Files:**
- Create: `src/menu/PracticeGameScreen.h`
- Create: `src/menu/PracticeGameScreen.cpp` (skeleton — stubs only)

- [ ] **Step 1: Write `PracticeGameScreen.h`**

```cpp
/*
 * Created by Ed Fillingham on 10/05/2026.
 *
 * Two-phase practice game screen.
 *
 * Phase PLACING: instructs the player to place each of the 7 blockers one at a time.
 *   Uses GridScanner every 200 ms to verify the target blocker is placed and no
 *   wrong blockers are present. Advances automatically when valid.
 *
 * Phase PLAYING: counts up a timer while the player fills the board with puzzle pieces.
 *   When all 36 cells are occupied the timer stops and PracticeScoreScreen is pushed.
 *
 * Call setArrangement() before pushing this screen to configure the arrangement.
 */

#ifndef PRACTICE_GAME_SCREEN_H
#define PRACTICE_GAME_SCREEN_H

#include <TFT_eSPI.h>
#include <vector>
#include "menu/Screen.h"
#include "menu/ScreenManager.h"
#include "menu/PracticeScoreScreen.h"
#include "utils/math/maths.h"

class PracticeGameScreen : public Screen {
public:
    /**
     * @brief Construct the game screen.
     * @param tft         Shared display reference.
     * @param manager     Shared ScreenManager reference.
     * @param scoreScreen Score screen pushed when the game ends.
     */
    PracticeGameScreen(TFT_eSPI& tft, ScreenManager& manager,
                       PracticeScoreScreen& scoreScreen);

    /**
     * @brief Configure the arrangement before pushing. Resets all phase state.
     * @param index       Index of this arrangement in practiceItems[].
     * @param blockers    The 7 blocker coordinates, presented in this order.
     * @param bestSeconds Pointer to practiceItems[index].seconds for in-RAM best update.
     */
    void setArrangement(uint8_t index, const std::vector<Coord>& blockers, float* bestSeconds);

    void onEnter()            override;
    void onExit()             override;
    void onResume()           override;
    void update()             override;
    void render()             override;
    void onEncoderChange(int delta) override;
    void onButtonPress()      override;

private:
    enum class Phase : uint8_t { PLACING, PLAYING };

    TFT_eSPI&            _tft;
    ScreenManager&       _manager;
    PracticeScoreScreen& _scoreScreen;

    uint8_t              _arrangementIndex;
    std::vector<Coord>   _blockers;
    float*               _bestSeconds;

    Phase                _phase;
    uint8_t              _confirmedCount;  ///< Blockers placed so far (0–7)

    bool                 _grid[6][6];
    bool                 _cellDirty[6][6];
    bool                 _panelDirty;

    bool                 _hasError;
    uint8_t              _errorCol;
    uint8_t              _errorRow;

    float                _elapsedSeconds;
    unsigned long        _timerStartMs;
    unsigned long        _lastScanMs;
    unsigned long        _lastMs;

    // ---- Layout (landscape 280×240) ----
    static constexpr int SCR_W      = 280;
    static constexpr int SCR_H      = 240;
    static constexpr int TITLE_H    = 30;
    static constexpr int MARGIN_X   = 14;
    static constexpr int CELL_SIZE  = 24;                       ///< px per grid cell
    static constexpr int GRID_LINE_W = 2;                       ///< grid line thickness
    static constexpr int GRID_ROWS  = 6;
    static constexpr int GRID_COLS  = 6;
    static constexpr int GRID_SIZE  = CELL_SIZE * GRID_ROWS;    ///< 144 px
    static constexpr int GRID_X     = MARGIN_X;                 ///< 14
    static constexpr int GRID_Y     = TITLE_H + 4;              ///< 34
    static constexpr int PANEL_X    = GRID_X + GRID_SIZE + GRID_LINE_W; ///< 160
    static constexpr int PANEL_W    = SCR_W - PANEL_X;          ///< 120
    static constexpr int PANEL_Y    = TITLE_H + 4;              ///< 34

    static constexpr unsigned long SCAN_INTERVAL_MS = 200;

    // ---- Colours ----
    static constexpr uint16_t COL_BG        = 0x0000;
    static constexpr uint16_t COL_TEXT      = 0xFFFF;
    static constexpr uint16_t COL_SUBTEXT   = 0xAD75;
    static constexpr uint16_t COL_TITLE     = 0xAD75;
    static constexpr uint16_t COL_GRID_LINE = 0x4A49;
    static constexpr uint16_t COL_BLOCKER   = 0xDDB4;  ///< Confirmed blocker — light grey
    static constexpr uint16_t COL_TARGET    = 0xFFE0;  ///< Target cell highlight — yellow
    static constexpr uint16_t COL_ERROR     = 0xF800;  ///< Wrong cell — red

    // ---- Drawing ----
    void drawTitleBar();
    void drawGridLines();
    void drawCell(uint8_t row, uint8_t col);
    void drawPanel();
    void drawAllCells();

    // ---- Scan / logic ----
    void scanGrid();
    void finishGame();

    // ---- Helpers ----
    bool isConfirmedCell(uint8_t col, uint8_t row) const;
    bool isTargetCell(uint8_t col, uint8_t row) const;
    void formatTime(float seconds, char* buf, size_t bufSize) const;
};

#endif // PRACTICE_GAME_SCREEN_H
```

- [ ] **Step 2: Write skeleton `PracticeGameScreen.cpp`** (stubs only so it compiles)

```cpp
/*
 * Created by Ed Fillingham on 10/05/2026.
 *
 * PracticeGameScreen — two-phase practice game (blocker placement + timed solve).
 */

#include "menu/PracticeGameScreen.h"
#include "hardware/GridScanner.h"
#include <string.h>
#include <stdio.h>

PracticeGameScreen::PracticeGameScreen(TFT_eSPI& tft, ScreenManager& manager,
                                       PracticeScoreScreen& scoreScreen)
    : _tft(tft), _manager(manager), _scoreScreen(scoreScreen),
      _arrangementIndex(0), _bestSeconds(nullptr),
      _phase(Phase::PLACING), _confirmedCount(0),
      _panelDirty(false), _hasError(false), _errorCol(0), _errorRow(0),
      _elapsedSeconds(0.0f), _timerStartMs(0), _lastScanMs(0), _lastMs(0)
{
    memset(_grid,      false, sizeof(_grid));
    memset(_cellDirty, false, sizeof(_cellDirty));
}

void PracticeGameScreen::setArrangement(uint8_t index, const std::vector<Coord>& blockers, float* bestSeconds) {
    _arrangementIndex = index;
    _blockers         = blockers;
    _bestSeconds      = bestSeconds;
    // Reset all state
    _phase          = Phase::PLACING;
    _confirmedCount = 0;
    _hasError       = false;
    _elapsedSeconds = 0.0f;
    _panelDirty     = false;
    memset(_grid,      false, sizeof(_grid));
    memset(_cellDirty, false, sizeof(_cellDirty));
}

void PracticeGameScreen::onEnter() {
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawGridLines();
    drawAllCells();
    drawPanel();
    _lastScanMs = millis();
    _lastMs     = millis();
}

void PracticeGameScreen::onExit()   {}
void PracticeGameScreen::onResume() {}
void PracticeGameScreen::onEncoderChange(int) {}
void PracticeGameScreen::onButtonPress() {}

void PracticeGameScreen::update() {}
void PracticeGameScreen::render() {}

void PracticeGameScreen::drawTitleBar()  {}
void PracticeGameScreen::drawGridLines() {}
void PracticeGameScreen::drawCell(uint8_t, uint8_t) {}
void PracticeGameScreen::drawPanel()     {}
void PracticeGameScreen::drawAllCells()  {}
void PracticeGameScreen::scanGrid()      {}
void PracticeGameScreen::finishGame()    {}
bool PracticeGameScreen::isConfirmedCell(uint8_t, uint8_t) const { return false; }
bool PracticeGameScreen::isTargetCell(uint8_t, uint8_t)    const { return false; }
void PracticeGameScreen::formatTime(float, char*, size_t)  const {}
```

- [ ] **Step 3: Build to verify compilation**

```bash
pio run
```

Expected: build succeeds with no errors. The skeleton stubs satisfy all pure-virtual requirements from `Screen`.

---

## Task 3: Implement `PracticeGameScreen` — drawing

Replace the stub implementations with real ones. Edit `src/menu/PracticeGameScreen.cpp`.

- [ ] **Step 1: Implement `drawTitleBar()`**

Replace:
```cpp
void PracticeGameScreen::drawTitleBar()  {}
```
With:
```cpp
void PracticeGameScreen::drawTitleBar() {
    _tft.setTextColor(COL_TITLE, COL_BG);
    _tft.setTextSize(2);
    const char* title = "Practice";
    int tx = (SCR_W - (int)strlen(title) * 12) / 2;
    _tft.setCursor(tx, (TITLE_H - 16) / 2);
    _tft.print(title);
    _tft.drawFastHLine(0, TITLE_H, SCR_W, COL_SUBTEXT);
}
```

- [ ] **Step 2: Implement `drawGridLines()`**

Replace:
```cpp
void PracticeGameScreen::drawGridLines() {}
```
With:
```cpp
void PracticeGameScreen::drawGridLines() {
    for (int i = 0; i <= GRID_ROWS; i++) {
        _tft.fillRect(GRID_X, GRID_Y + i * CELL_SIZE, GRID_SIZE + GRID_LINE_W, GRID_LINE_W, COL_GRID_LINE);
        _tft.fillRect(GRID_X + i * CELL_SIZE, GRID_Y, GRID_LINE_W, GRID_SIZE + GRID_LINE_W, COL_GRID_LINE);
    }
}
```

- [ ] **Step 3: Implement `isConfirmedCell()` and `isTargetCell()`**

Replace:
```cpp
bool PracticeGameScreen::isConfirmedCell(uint8_t, uint8_t) const { return false; }
bool PracticeGameScreen::isTargetCell(uint8_t, uint8_t)    const { return false; }
```
With:
```cpp
bool PracticeGameScreen::isConfirmedCell(uint8_t col, uint8_t row) const {
    for (uint8_t i = 0; i < _confirmedCount; i++) {
        if ((uint8_t)_blockers[i].x == col && (uint8_t)_blockers[i].y == row) return true;
    }
    return false;
}

bool PracticeGameScreen::isTargetCell(uint8_t col, uint8_t row) const {
    if (_confirmedCount >= (uint8_t)_blockers.size()) return false;
    return (uint8_t)_blockers[_confirmedCount].x == col
        && (uint8_t)_blockers[_confirmedCount].y == row;
}
```

- [ ] **Step 4: Implement `formatTime()`**

Replace:
```cpp
void PracticeGameScreen::formatTime(float, char*, size_t) const {}
```
With:
```cpp
void PracticeGameScreen::formatTime(float seconds, char* buf, size_t bufSize) const {
    int total = (int)seconds;
    snprintf(buf, bufSize, "%02d:%02d", total / 60, total % 60);
}
```

- [ ] **Step 5: Implement `drawCell()`**

Replace:
```cpp
void PracticeGameScreen::drawCell(uint8_t, uint8_t) {}
```
With:
```cpp
void PracticeGameScreen::drawCell(uint8_t row, uint8_t col) {
    int px = GRID_X + col * CELL_SIZE + GRID_LINE_W;
    int py = GRID_Y + row * CELL_SIZE + GRID_LINE_W;
    int sz = CELL_SIZE - GRID_LINE_W;
    int cx = px + sz / 2;
    int cy = py + sz / 2;
    int cr = sz / 2 - 2;

    _tft.fillRect(px, py, sz, sz, COL_BG);

    if (_phase == Phase::PLACING) {
        if (isConfirmedCell(col, row)) {
            _tft.fillCircle(cx, cy, cr, COL_BLOCKER);
        } else if (_hasError && _errorCol == col && _errorRow == row) {
            _tft.fillRect(px, py, sz, sz, COL_ERROR);
        } else if (isTargetCell(col, row)) {
            _tft.drawRect(px, py, sz, sz, COL_TARGET);
        }
    } else {
        if (_grid[row][col]) {
            _tft.fillCircle(cx, cy, cr, COL_TEXT);
        }
    }
}
```

- [ ] **Step 6: Implement `drawAllCells()` and `drawPanel()`**

Replace:
```cpp
void PracticeGameScreen::drawAllCells()  {}
void PracticeGameScreen::drawPanel()     {}
```
With:
```cpp
void PracticeGameScreen::drawAllCells() {
    for (uint8_t r = 0; r < GRID_ROWS; r++)
        for (uint8_t c = 0; c < GRID_COLS; c++)
            drawCell(r, c);
}

void PracticeGameScreen::drawPanel() {
    // Clear panel area
    _tft.fillRect(PANEL_X, PANEL_Y, PANEL_W, SCR_H - PANEL_Y, COL_BG);

    if (_phase == Phase::PLACING) {
        // "Place blocker" label
        _tft.setTextColor(COL_SUBTEXT, COL_BG);
        _tft.setTextSize(1);
        _tft.setCursor(PANEL_X + 4, PANEL_Y + 8);
        _tft.print("Place blocker");

        // Large coordinate — e.g. "B3" at size 3 (18×24 px per char)
        if (_confirmedCount < (uint8_t)_blockers.size()) {
            Coord target = _blockers[_confirmedCount];
            char coordBuf[4];
            snprintf(coordBuf, sizeof(coordBuf), "%c%d", 'A' + target.x, target.y + 1);
            _tft.setTextColor(COL_TEXT, COL_BG);
            _tft.setTextSize(3);
            int coordW = (int)strlen(coordBuf) * 18;
            _tft.setCursor(PANEL_X + (PANEL_W - coordW) / 2, PANEL_Y + 24);
            _tft.print(coordBuf);
        }

        // Progress "N / 7" at size 2
        char progBuf[8];
        snprintf(progBuf, sizeof(progBuf), "%d / %d", _confirmedCount, (int)_blockers.size());
        _tft.setTextColor(COL_SUBTEXT, COL_BG);
        _tft.setTextSize(2);
        int progW = (int)strlen(progBuf) * 12;
        _tft.setCursor(PANEL_X + (PANEL_W - progW) / 2, PANEL_Y + 60);
        _tft.print(progBuf);

        // Error message
        if (_hasError) {
            _tft.setTextColor(COL_ERROR, COL_BG);
            _tft.setTextSize(1);
            _tft.setCursor(PANEL_X + 4, PANEL_Y + 90);
            _tft.print("Wrong!");
            _tft.setCursor(PANEL_X + 4, PANEL_Y + 100);
            _tft.print("Remove it.");
        }
    } else {
        // PLAYING phase
        _tft.setTextColor(COL_SUBTEXT, COL_BG);
        _tft.setTextSize(2);
        const char* label = "Solve it!";
        int lw = (int)strlen(label) * 12;
        _tft.setCursor(PANEL_X + (PANEL_W - lw) / 2, PANEL_Y + 30);
        _tft.print(label);

        char timeBuf[8];
        formatTime(_elapsedSeconds, timeBuf, sizeof(timeBuf));
        _tft.setTextColor(COL_TEXT, COL_BG);
        _tft.setTextSize(3);
        int tw = (int)strlen(timeBuf) * 18;
        _tft.setCursor(PANEL_X + (PANEL_W - tw) / 2, PANEL_Y + 60);
        _tft.print(timeBuf);
    }
}
```

- [ ] **Step 7: Build to verify compilation**

```bash
pio run
```

Expected: build succeeds.

---

## Task 4: Implement `PracticeGameScreen` — scan, update, render

- [ ] **Step 1: Implement `scanGrid()`**

Replace:
```cpp
void PracticeGameScreen::scanGrid() {}
```
With:
```cpp
void PracticeGameScreen::scanGrid() {
    auto raw = GridScanner::digitalReadMatrix();

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            bool newVal = raw[r][c];
            if (newVal != _grid[r][c]) {
                _grid[r][c]      = newVal;
                _cellDirty[r][c] = true;
            }
        }
    }

    if (_phase == Phase::PLACING) {
        Coord target = _blockers[_confirmedCount];
        bool targetOccupied = _grid[target.y][target.x];

        // Find the first wrong cell (occupied but not in confirmed set or target)
        bool hasError = false;
        uint8_t errCol = 0, errRow = 0;
        for (int r = 0; r < GRID_ROWS && !hasError; r++) {
            for (int c = 0; c < GRID_COLS && !hasError; c++) {
                if (!_grid[r][c]) continue;
                if (isConfirmedCell(c, r)) continue;
                if (c == target.x && r == target.y) continue;
                hasError = true;
                errCol   = c;
                errRow   = r;
            }
        }

        bool prevError = _hasError;
        uint8_t prevErrCol = _errorCol;
        uint8_t prevErrRow = _errorRow;

        _hasError = hasError;
        _errorCol = errCol;
        _errorRow = errRow;

        if (_hasError != prevError || errCol != prevErrCol || errRow != prevErrRow) {
            if (prevError)  _cellDirty[prevErrRow][prevErrCol] = true;
            if (_hasError)  _cellDirty[errRow][errCol]         = true;
            _panelDirty = true;
        }

        if (targetOccupied && !hasError) {
            _cellDirty[target.y][target.x] = true;
            _confirmedCount++;
            _panelDirty = true;

            if (_confirmedCount >= (uint8_t)_blockers.size()) {
                _phase         = Phase::PLAYING;
                _timerStartMs  = millis();
                _elapsedSeconds = 0.0f;
                for (int r = 0; r < GRID_ROWS; r++)
                    for (int c = 0; c < GRID_COLS; c++)
                        _cellDirty[r][c] = true;
                _panelDirty = true;
            } else {
                Coord next = _blockers[_confirmedCount];
                _cellDirty[next.y][next.x] = true;
            }
        }

    } else {
        bool allFull = true;
        for (int r = 0; r < GRID_ROWS && allFull; r++)
            for (int c = 0; c < GRID_COLS && allFull; c++)
                if (!_grid[r][c]) allFull = false;

        if (allFull) {
            _elapsedSeconds = (millis() - _timerStartMs) / 1000.0f;
            finishGame();
        }
    }
}
```

- [ ] **Step 2: Implement `finishGame()`**

Replace:
```cpp
void PracticeGameScreen::finishGame() {}
```
With:
```cpp
void PracticeGameScreen::finishGame() {
    bool isNewBest = (_bestSeconds != nullptr)
        && (*_bestSeconds <= 0.0f || _elapsedSeconds < *_bestSeconds);
    if (isNewBest && _bestSeconds != nullptr) {
        *_bestSeconds = _elapsedSeconds;
    }
    float best = (_bestSeconds != nullptr) ? *_bestSeconds : 0.0f;
    _scoreScreen.setResult(_elapsedSeconds, best, isNewBest);
    _manager.push(&_scoreScreen);
}
```

- [ ] **Step 3: Implement `update()` and `render()`**

Replace:
```cpp
void PracticeGameScreen::update() {}
void PracticeGameScreen::render() {}
```
With:
```cpp
void PracticeGameScreen::update() {
    unsigned long now = millis();

    if (now - _lastScanMs >= SCAN_INTERVAL_MS) {
        _lastScanMs = now;
        scanGrid();
    }

    if (_phase == Phase::PLAYING) {
        float newElapsed = (now - _timerStartMs) / 1000.0f;
        if ((int)newElapsed != (int)_elapsedSeconds) {
            _elapsedSeconds = newElapsed;
            _panelDirty     = true;
        }
        _elapsedSeconds = newElapsed;
    }

    _lastMs = now;
}

void PracticeGameScreen::render() {
    for (uint8_t r = 0; r < GRID_ROWS; r++) {
        for (uint8_t c = 0; c < GRID_COLS; c++) {
            if (_cellDirty[r][c]) {
                _cellDirty[r][c] = false;
                drawCell(r, c);
            }
        }
    }
    if (_panelDirty) {
        _panelDirty = false;
        drawPanel();
    }
}
```

- [ ] **Step 4: Build to verify compilation**

```bash
pio run
```

Expected: build succeeds with no errors or warnings.

---

## Task 5: Wire up `main.cpp` and test on device

**Files:**
- Modify: `src/menu/ArrangementItem.h`
- Modify: `src/main.cpp`

- [ ] **Step 1: Change `ArrangementItem::action` to `std::function`**

The existing raw function pointer `void (*action)(ScreenManager&)` cannot store a capturing lambda (which is needed to capture the loop index `i`). Change it to `std::function`.

In `src/menu/ArrangementItem.h`, replace:
```cpp
#include <vector>
#include "utils/math/maths.h"  // Coord
```
With:
```cpp
#include <vector>
#include <functional>
#include "utils/math/maths.h"  // Coord
```

And replace:
```cpp
    void (*action)(ScreenManager&);   ///< Called when this item is confirmed
```
With:
```cpp
    std::function<void(ScreenManager&)> action;  ///< Called when this item is confirmed
```

- [ ] **Step 2: Add includes and static screen objects**

After the existing includes in `main.cpp`, add:
```cpp
#include "menu/PracticeGameScreen.h"
#include "menu/PracticeScoreScreen.h"
```

After the line:
```cpp
ArrangementMenuScreen practiceMenu(tft, screenManager, practiceItems, PRACTICE_ITEM_COUNT, "Practice");
```

Add:
```cpp
/// Score screen shown after a practice game ends
PracticeScoreScreen practiceScore(tft, screenManager);

/// Active practice game screen — configured via setArrangement() before push
PracticeGameScreen practiceGame(tft, screenManager, practiceScore);
```

- [ ] **Step 3: Update practice item action callbacks in `setup()`**

Replace the existing loop in `setup()`:
```cpp
for (int i = 0; i < PRACTICE_ITEM_COUNT; i++) {
    PuzzleDifficulty packed = getDifficulty(i);
    switch (packed) {
        case PuzzleDifficulty::HARD:   practiceItems[i].difficulty = Difficulty::HARD; break;
        case PuzzleDifficulty::MEDIUM: practiceItems[i].difficulty = Difficulty::MED; break;
        case PuzzleDifficulty::EASY:   practiceItems[i].difficulty = Difficulty::EASY; break;
        default:                       practiceItems[i].difficulty = Difficulty::EASY; break;
    }
    practiceItems[i].seconds     = 0.0f;
    practiceItems[i].arrangement = CombinationGenerator::generateCombinations(i);
    practiceItems[i].action      = [](ScreenManager&) {};
}
```

With:
```cpp
for (int i = 0; i < PRACTICE_ITEM_COUNT; i++) {
    PuzzleDifficulty packed = getDifficulty(i);
    switch (packed) {
        case PuzzleDifficulty::HARD:   practiceItems[i].difficulty = Difficulty::HARD; break;
        case PuzzleDifficulty::MEDIUM: practiceItems[i].difficulty = Difficulty::MED; break;
        case PuzzleDifficulty::EASY:   practiceItems[i].difficulty = Difficulty::EASY; break;
        default:                       practiceItems[i].difficulty = Difficulty::EASY; break;
    }
    practiceItems[i].seconds     = 0.0f;
    practiceItems[i].arrangement = CombinationGenerator::generateCombinations(i);
    practiceItems[i].action      = [i](ScreenManager& mgr) {
        practiceGame.setArrangement(i, practiceItems[i].arrangement, &practiceItems[i].seconds);
        mgr.push(&practiceGame);
    };
}
```

- [ ] **Step 4: Build and upload**

```bash
pio run --target upload && pio device monitor
```

Expected: firmware uploads, serial monitor opens at 115200 baud.

- [ ] **Step 5: Manual test — PLACING phase**

1. Navigate: Main Menu → Practice → select any arrangement
2. Game screen appears with "Practice" title bar, 6×6 grid, and "Place blocker" + first coordinate on the right panel
3. Place the correct physical blocker on the indicated cell — grid updates within 200 ms, progress count advances ("1 / 7", "2 / 7", …)
4. Place a blocker on the wrong cell — "Wrong! Remove it." appears in red, that cell turns red. Remove the wrong blocker — error clears
5. Complete all 7 blockers — right panel switches to "Solve it!" and timer starts

- [ ] **Step 6: Manual test — PLAYING phase and score screen**

1. After all 7 blockers placed, timer counts up visibly (MM:SS)
2. Place puzzle pieces until all 36 cells are occupied — timer stops, score screen appears immediately
3. Score screen shows "Your time: MM:SS" and "Best: --:--" (no prior best)
4. Press button — returns to arrangement selection menu
5. Select same arrangement again, complete it faster — score screen shows "New best!" in green and "Best: MM:SS" reflects the first run's time (previously stored in RAM)
