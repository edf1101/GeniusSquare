# Solution Display Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Display the solver result on the SolverMenuScreen grid as coloured filled rounded rectangles per piece cell, with a spinner animation while solving and automatic clear on arrangement change.

**Architecture:** `SolverTask` captures `Board::getSpace()` into a member vector before signalling `_done` (acquire/release safe), exposing it via `getSolutionGrid()`. `SolverMenuScreen` polls `isDone()` each `update()` tick, copies the grid on the rising edge, marks cells dirty, and renders coloured `fillRoundRect` calls per piece cell. A spinner cycles `_spinnerDots` 0–3 every 400 ms while the solve is in progress; the side panel shows Solving./Solving../Solving.../Solved!/No sln as appropriate.

**Tech Stack:** C++17, ESP32-S3 Arduino framework, TFT_eSPI, PlatformIO (`pio run` to build)

---

### Task 1: Add `getSolutionGrid()` to `SolverTask`

**Files:**
- Modify: `src/solver/SolverTask.h`
- Modify: `src/solver/SolverTask.cpp`

The solution grid is written on Core 0 before `_done.store(true, release)` and read on Core 1 after `_done.load(acquire)` — the existing atomic ordering makes this safe without any additional synchronisation.

- [ ] **Step 1: Add member and declaration to `SolverTask.h`**

In the `public:` section, after `bool hasSolution() const;`, add:

```cpp
/** @brief Returns the solved board grid (UUID per cell, 0 = empty). Valid only when hasSolution() is true. */
std::vector<std::vector<int>> getSolutionGrid() const;
```

In the `private:` section, after `std::atomic<bool> _foundSolution{false};`, add:

```cpp
std::vector<std::vector<int>> _solutionGrid;
```

- [ ] **Step 2: Capture grid in `taskFn` before signalling done**

In `SolverTask.cpp`, inside `taskFn`, the current sequence is:

```cpp
self->_foundSolution.store(solved);
if (solved) self->_board->printSolution();
self->_done.store(true, std::memory_order_release);
```

Change it to:

```cpp
self->_foundSolution.store(solved);
if (solved) {
    self->_solutionGrid = self->_board->getSpace();
    self->_board->printSolution();
}
self->_done.store(true, std::memory_order_release);
```

- [ ] **Step 3: Implement `getSolutionGrid()`**

Add at the end of `SolverTask.cpp`:

```cpp
std::vector<std::vector<int>> SolverTask::getSolutionGrid() const {
    return _solutionGrid;
}
```

- [ ] **Step 4: Verify compilation**

```bash
pio run
```

Expected: `SUCCESS` with no errors.

---

### Task 2: Extend `SolverMenuScreen` with solution state fields, constants, and method declarations

**Files:**
- Modify: `src/menu/SolverMenuScreen.h`

Layout maths:
- Cell interior starts at `cellX + GRID_LINE_W` (2 px grid line on left/top edge).
- Interior size: `CELL_SIZE - GRID_LINE_W = 22 px`.
- With 2 px padding each side: rect size = 18 px square, corner radius 3.

- [ ] **Step 1: Add layout constants to `SolverMenuScreen.h`**

Inside `class SolverMenuScreen`, in the `// ---- Layout: grid ----` constants block, after `FADE_DURATION_S`, add:

```cpp
static constexpr int   PIECE_PAD       = 2;   ///< px between grid line and piece rect edge
static constexpr int   PIECE_RECT_SIZE = CELL_SIZE - GRID_LINE_W - 2 * PIECE_PAD; ///< 18 px
static constexpr int   PIECE_RECT_R    = 3;   ///< corner radius of piece rect
static constexpr unsigned long SPINNER_INTERVAL_MS = 400;
```

- [ ] **Step 2: Add solution state fields**

In the `private:` section, after the `// ---- Solver ----` block (after `_lastBlockers`), add:

```cpp
// ---- Solution display ----
int   _solutionGrid[6][6];   ///< UUID per cell from getSolutionGrid(); 0 = empty
bool  _showSolution;         ///< true once a solution has been copied and should be rendered
bool  _wasDone;              ///< previous isDone() value — used to detect rising edge
bool  _pieceDirty[6][6];    ///< cells needing a piece-rect redraw
uint8_t       _spinnerDots;  ///< 0–3: maps to "Solving." / "Solving.." / "Solving..." / "Solving.."
unsigned long _spinnerMs;    ///< millis() when _spinnerDots was last advanced
```

- [ ] **Step 3: Add new private method declarations**

In the `// ---- Grid helpers ----` section, after `renderCellFaded`, add:

```cpp
void renderCellPiece(int r, int c);
void clearSolution();
```

- [ ] **Step 4: Verify compilation**

```bash
pio run
```

Expected: `SUCCESS` with no errors.

---

### Task 3: Initialise new fields in constructor, `onEnter()`, and `onResume()`

**Files:**
- Modify: `src/menu/SolverMenuScreen.cpp`

`_wasDone` must be initialised to `true` so the first `update()` frame (where `isDone()` returns `true` because `_active=false`) does not falsely trigger the solution-copy rising edge.

- [ ] **Step 1: Extend the constructor initialiser list**

The current constructor body in `SolverMenuScreen.cpp` opens with:

```cpp
SolverMenuScreen::SolverMenuScreen(TFT_eSPI& tft, ScreenManager& manager)
    : _tft(tft), _manager(manager),
      _selectedIndex(0),
      _lastMs(millis()), _dirty(false),
      _panelDirty(false), _solvable(false),
      _lastScanMs(0)
{
    memset(_grid,      0, sizeof(_grid));
    memset(_cellAlpha, 0, sizeof(_cellAlpha));
    memset(_cellDirty, 0, sizeof(_cellDirty));
    _reason[0] = '\0';
}
```

Replace it with:

```cpp
SolverMenuScreen::SolverMenuScreen(TFT_eSPI& tft, ScreenManager& manager)
    : _tft(tft), _manager(manager),
      _selectedIndex(0),
      _lastMs(millis()), _dirty(false),
      _panelDirty(false), _solvable(false),
      _lastScanMs(0),
      _showSolution(false), _wasDone(true),
      _spinnerDots(0), _spinnerMs(0)
{
    memset(_grid,        0, sizeof(_grid));
    memset(_cellAlpha,   0, sizeof(_cellAlpha));
    memset(_cellDirty,   0, sizeof(_cellDirty));
    memset(_solutionGrid,0, sizeof(_solutionGrid));
    memset(_pieceDirty,  0, sizeof(_pieceDirty));
    _reason[0] = '\0';
}
```

- [ ] **Step 2: Extend `onEnter()` reset block**

The current `onEnter()` has this reset block:

```cpp
memset(_grid,      0, sizeof(_grid));
memset(_cellAlpha, 0, sizeof(_cellAlpha));
memset(_cellDirty, 0, sizeof(_cellDirty));
_reason[0] = '\0';
```

Replace it with:

```cpp
memset(_grid,        0, sizeof(_grid));
memset(_cellAlpha,   0, sizeof(_cellAlpha));
memset(_cellDirty,   0, sizeof(_cellDirty));
memset(_solutionGrid,0, sizeof(_solutionGrid));
memset(_pieceDirty,  0, sizeof(_pieceDirty));
_reason[0]     = '\0';
_showSolution  = false;
_wasDone       = true;
_spinnerDots   = 0;
_spinnerMs     = millis();
```

- [ ] **Step 3: Extend `onResume()` reset block identically**

The current `onResume()` has the same four-line reset block. Replace it with the same six-line block from Step 2.

- [ ] **Step 4: Verify compilation**

```bash
pio run
```

Expected: `SUCCESS` with no errors.

---

### Task 4: Implement `clearSolution()` and wire into `checkValidity()`

**Files:**
- Modify: `src/menu/SolverMenuScreen.cpp`

`clearSolution()` marks all 36 piece-cell slots dirty so `render()` will erase the coloured rects on the next frame. It is a no-op when `_showSolution` is already false to avoid unnecessary redraws.

- [ ] **Step 1: Implement `clearSolution()`**

Add after the `checkValidity()` function in `SolverMenuScreen.cpp`:

```cpp
void SolverMenuScreen::clearSolution() {
    if (!_showSolution) return;
    _showSolution = false;
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++)
            _pieceDirty[r][c] = true;
    _panelDirty = true;
}
```

- [ ] **Step 2: Call `clearSolution()` in `checkValidity()` when arrangement changes or becomes invalid**

In `checkValidity()`, the current solvable branch reads:

```cpp
if (_solvable) {
    if (coords != _lastBlockers) {
        Serial.println("[SolverMenu] Valid arrangement detected, starting solve");
        _lastBlockers = coords;
        _solver.start(coords);
    }
} else {
    if (!_lastBlockers.empty()) {
        Serial.println("[SolverMenu] Arrangement changed/invalid, stopping solve");
        _solver.stop();
        _lastBlockers.clear();
    }
}
```

Replace with:

```cpp
if (_solvable) {
    if (coords != _lastBlockers) {
        Serial.println("[SolverMenu] Valid arrangement detected, starting solve");
        clearSolution();
        _wasDone    = true;   // reset edge detector for new solve
        _spinnerDots = 0;
        _spinnerMs   = millis();
        _lastBlockers = coords;
        _solver.start(coords);
    }
} else {
    if (!_lastBlockers.empty()) {
        Serial.println("[SolverMenu] Arrangement changed/invalid, stopping solve");
        clearSolution();
        _solver.stop();
        _lastBlockers.clear();
    }
}
```

- [ ] **Step 3: Verify compilation**

```bash
pio run
```

Expected: `SUCCESS` with no errors.

---

### Task 5: Implement solution-copy rising edge and spinner in `update()`

**Files:**
- Modify: `src/menu/SolverMenuScreen.cpp`

- [ ] **Step 1: Add solution-copy and spinner logic to `update()`**

The current `update()` body ends after the cell-alpha fade loop. Add the following block immediately after that loop (before the closing `}`):

```cpp
    // Solution-copy: detect isDone() rising edge
    bool nowDone = _solver.isDone();
    if (nowDone && !_wasDone) {
        if (_solver.hasSolution()) {
            auto grid = _solver.getSolutionGrid();
            for (int r = 0; r < GRID_ROWS; r++)
                for (int c = 0; c < GRID_COLS; c++) {
                    _solutionGrid[r][c] = grid[r][c];
                    _pieceDirty[r][c]   = true;
                }
            _showSolution = true;
        }
        _panelDirty = true;
    }
    _wasDone = nowDone;

    // Spinner: advance dot count while solve is in progress
    if (_solvable && !nowDone) {
        if (now - _spinnerMs >= SPINNER_INTERVAL_MS) {
            _spinnerMs   = now;
            _spinnerDots = (_spinnerDots + 1) % 4;
            _panelDirty  = true;
        }
    }
```

- [ ] **Step 2: Verify compilation**

```bash
pio run
```

Expected: `SUCCESS` with no errors.

---

### Task 6: Implement `renderCellPiece()` and wire into `render()`

**Files:**
- Modify: `src/menu/SolverMenuScreen.cpp`
- Must add `#include "solver/Piece.h"` to `SolverMenuScreen.cpp` (check if already present)

`renderCellPiece` draws a coloured `fillRoundRect` for piece cells. When `_showSolution` is false or the cell is a physical blocker, it fills the cell interior with `COL_BG` instead (erasing any stale rect).

- [ ] **Step 1: Add `#include "solver/Piece.h"` if not present**

Check the top of `src/menu/SolverMenuScreen.cpp`. If `#include "solver/Piece.h"` is not there, add it after the existing `#include` lines.

- [ ] **Step 2: Implement `renderCellPiece()`**

Add after `renderCellFaded()` in `SolverMenuScreen.cpp`:

```cpp
void SolverMenuScreen::renderCellPiece(int r, int c) {
    int cellX = GRID_X + c * CELL_SIZE;
    int cellY = GRID_Y + r * CELL_SIZE;
    int rx    = cellX + GRID_LINE_W + PIECE_PAD;
    int ry    = cellY + GRID_LINE_W + PIECE_PAD;

    if (_showSolution && _solutionGrid[r][c] != 0 && !_grid[r][c]) {
        Colour col    = Piece::getPieceColourByUUID(_solutionGrid[r][c]);
        uint16_t c565 = _tft.color565(col.r, col.g, col.b);
        _tft.fillRoundRect(rx, ry, PIECE_RECT_SIZE, PIECE_RECT_SIZE, PIECE_RECT_R, c565);
    } else if (!_grid[r][c]) {
        // Erase any stale piece rect
        _tft.fillRect(cellX + GRID_LINE_W, cellY + GRID_LINE_W,
                      CELL_SIZE - GRID_LINE_W, CELL_SIZE - GRID_LINE_W, COL_BG);
    }
    // Blocker cells (_grid[r][c]==true) are handled by renderCellFaded — leave them alone.
}
```

- [ ] **Step 3: Wire `_pieceDirty` into `render()`**

The current `render()` body is:

```cpp
void SolverMenuScreen::render() {
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (_cellDirty[r][c]) {
                _cellDirty[r][c] = false;
                renderCellFaded(r, c, _cellAlpha[r][c]);
            }
        }
    }

    if (_panelDirty) {
        _panelDirty = false;
        drawSidePanel();
    }

    if (!_dirty) return;
    _dirty = false;
    updateBorder();
}
```

Replace with:

```cpp
void SolverMenuScreen::render() {
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (_cellDirty[r][c]) {
                _cellDirty[r][c] = false;
                renderCellFaded(r, c, _cellAlpha[r][c]);
            }
            if (_pieceDirty[r][c]) {
                _pieceDirty[r][c] = false;
                renderCellPiece(r, c);
            }
        }
    }

    if (_panelDirty) {
        _panelDirty = false;
        drawSidePanel();
    }

    if (!_dirty) return;
    _dirty = false;
    updateBorder();
}
```

- [ ] **Step 4: Verify compilation**

```bash
pio run
```

Expected: `SUCCESS` with no errors.

---

### Task 7: Update `drawSidePanel()` for spinner and solved/no-solution states

**Files:**
- Modify: `src/menu/SolverMenuScreen.cpp`

The spinner uses a 4-frame cycle: `_spinnerDots` 0→1→2→3. The label array maps: 0→"Solving.", 1→"Solving..", 2→"Solving...", 3→"Solving..". At text size 2 (12 px/char), "Solving..." (10 chars) = 120 px = exactly `PANEL_W`, so it fits.

- [ ] **Step 1: Replace the state-word section in `drawSidePanel()`**

The current state-word section in `drawSidePanel()` reads:

```cpp
    // --- State word (size 2) ---
    int y = PANEL_Y + 2;
    _tft.setTextSize(2);
    if (_solvable) {
        _tft.setTextColor(COL_SOLVABLE, COL_BG);
        _tft.setCursor(PANEL_X, y);
        _tft.print("Solvable");
    } else {
        _tft.setTextColor(COL_UNSOLVABLE, COL_BG);
        _tft.setCursor(PANEL_X, y);
        _tft.print("Unsolvable");
        // Reason (size 2, below state word)
        if (_reason[0] != '\0') {
            _tft.setTextSize(2);
            _tft.setCursor(PANEL_X, y + STATE_LINE_H);
            _tft.print(_reason);
        }
    }
```

Replace it with:

```cpp
    // --- State word (size 2) ---
    static const char* spinLabels[4] = { "Solving.", "Solving..", "Solving...", "Solving.." };
    int y = PANEL_Y + 2;
    _tft.setTextSize(2);
    if (!_solvable) {
        _tft.setTextColor(COL_UNSOLVABLE, COL_BG);
        _tft.fillRect(PANEL_X, y, PANEL_W, STATE_LINE_H + REASON_LINE_H, COL_BG);
        _tft.setCursor(PANEL_X, y);
        _tft.print("Unsolvable");
        if (_reason[0] != '\0') {
            _tft.setCursor(PANEL_X, y + STATE_LINE_H);
            _tft.print(_reason);
        }
    } else if (!_solver.isDone()) {
        _tft.setTextColor(COL_SOLVABLE, COL_BG);
        _tft.fillRect(PANEL_X, y, PANEL_W, STATE_LINE_H, COL_BG);
        _tft.setCursor(PANEL_X, y);
        _tft.print(spinLabels[_spinnerDots]);
    } else if (_showSolution) {
        _tft.setTextColor(COL_SOLVABLE, COL_BG);
        _tft.fillRect(PANEL_X, y, PANEL_W, STATE_LINE_H, COL_BG);
        _tft.setCursor(PANEL_X, y);
        _tft.print("Solved!");
    } else {
        _tft.setTextColor(COL_UNSOLVABLE, COL_BG);
        _tft.fillRect(PANEL_X, y, PANEL_W, STATE_LINE_H, COL_BG);
        _tft.setCursor(PANEL_X, y);
        _tft.print("No sln");
    }
```

- [ ] **Step 2: Verify compilation**

```bash
pio run
```

Expected: `SUCCESS` with no errors.

---

### Task 8: Integration test via serial monitor

- [ ] **Step 1: Upload and open monitor**

```bash
pio run --target upload && pio device monitor
```

- [ ] **Step 2: Place fewer than 7 blockers**

Expected: panel shows "Unsolvable" + reason ("Too few"). No coloured rects on grid.

- [ ] **Step 3: Place exactly 7 blockers in a valid arrangement**

Expected: panel cycles "Solving." → "Solving.." → "Solving..." → "Solving.." every 400 ms. Grid shows only beige blocker circles.

- [ ] **Step 4: Wait for solve to complete**

Expected: panel switches to "Solved!". Grid fills with coloured rounded rectangles for each piece cell (blocker cells remain as beige circles). Serial log should show `[Board] solve() done ptr=... — FOUND`.

- [ ] **Step 5: Remove one blocker**

Expected: coloured rects disappear immediately. Panel returns to "Unsolvable" / "Solving..." as appropriate for the new arrangement.

- [ ] **Step 6: Re-place to a valid 7-blocker arrangement**

Expected: spinner resumes, then new solution appears with colours matching the new solve.

- [ ] **Step 7: Press Back**

Expected: `_solver.stop()` called from `onExit()`, screen exits cleanly. No crash or hang.
