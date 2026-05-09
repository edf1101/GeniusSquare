# Background Solver Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Run `Board::solve()` on ESP32 Core 0 as a FreeRTOS task; start automatically when a valid 7-blocker arrangement is detected, cancel and restart when the arrangement changes, stop cleanly when the screen exits — with Serial debug throughout.

**Architecture:** A new `SolverTask` class in `src/solver/` owns the `Board*`, FreeRTOS task handle, and a binary semaphore for shutdown synchronisation. `Board` gains a volatile cancel flag checked inside `recursiveSolve`, plus Serial debug prints and a `printSolution()` method. `SolverMenuScreen` holds a `SolverTask` member and calls `start()`/`stop()` from `checkValidity()` and `onExit()`.

**Tech Stack:** C++17, ESP32-S3 Arduino framework, FreeRTOS (via ESP-IDF), PlatformIO (`pio run` to build)

---

### Task 1: Add `Maths::coordsToString` helper

**Files:**
- Modify: `src/utils/math/maths.h`

`Coord` is defined after `Maths` in the header, so the method is declared with a forward declaration and defined inline at the bottom of the file.

- [ ] **Step 1: Forward-declare `Coord` before the `Maths` class**

At the top of `maths.h`, after the `#include` lines and before `class Maths`, add:

```cpp
struct Coord; // forward declaration for Maths::coordsToString
```

- [ ] **Step 2: Add `coordsToString` declaration to the `Maths` class**

Inside `class Maths { public: ... }`, after the existing `MatAsString` declaration:

```cpp
static std::string coordsToString(const std::vector<Coord>& coords);
```

- [ ] **Step 3: Add inline implementation after the `Coord` struct definition**

At the very bottom of `maths.h`, before `#endif`:

```cpp
inline std::string Maths::coordsToString(const std::vector<Coord>& coords) {
    std::string s = "[";
    for (size_t i = 0; i < coords.size(); i++) {
        s += coords[i].toString();
        if (i + 1 < coords.size()) s += ", ";
    }
    s += "]";
    return s;
}
```

- [ ] **Step 4: Verify compilation**

```bash
pio run
```

Expected: `SUCCESS` with no errors.

---

### Task 2: Extend `Board` with cancel, serial debug, and `printSolution`

**Files:**
- Modify: `src/solver/Board.h`
- Modify: `src/solver/Board.cpp`

- [ ] **Step 1: Add new fields and method declarations to `Board.h`**

In the `public:` section, after `bool solve();`, add:

```cpp
void cancel();
void printSolution();
~Board();
```

In the `private:` section, after `unsigned long startSolveTime = 0;`, add:

```cpp
volatile bool _cancel = false;
```

- [ ] **Step 2: Add `#include "utils/math/maths.h"` is already present — verify `Arduino.h` is included**

`Board.cpp` already has `#include "Arduino.h"` (used by `millis()`). Confirm it is present at the top of `Board.cpp`. No change needed if so.

- [ ] **Step 3: Add serial debug print to the `Board` constructor**

At the end of `Board::Board(...)`, after the blocker placement loop, add:

```cpp
Serial.printf("[Board] Created ptr=%p blockers=%s\n",
    (void*)this,
    Maths::coordsToString(this->blockers).c_str());
```

- [ ] **Step 4: Implement `Board::~Board()`**

Add after the constructor implementation in `Board.cpp`:

```cpp
Board::~Board() {
    Serial.printf("[Board] Destroyed ptr=%p\n", (void*)this);
}
```

- [ ] **Step 5: Implement `Board::cancel()`**

```cpp
void Board::cancel() {
    _cancel = true;
}
```

- [ ] **Step 6: Add `_cancel` check to both exit points in `recursiveSolve`**

`recursiveSolve` has two `if (rootBoard->getOutOfTime())` guards. Change both to:

```cpp
if (rootBoard->getOutOfTime() || rootBoard->_cancel) {
    return false;
}
```

- [ ] **Step 7: Add serial debug prints to `Board::solve()`**

At the top of `solve()`, before `startSolveTime = millis();`:

```cpp
Serial.printf("[Board] solve() started ptr=%p\n", (void*)this);
```

At the end of `solve()`, replace `return !solutions.empty();` with:

```cpp
bool found = !solutions.empty();
Serial.printf("[Board] solve() done ptr=%p — %s\n",
    (void*)this, found ? "FOUND" : "NOT FOUND");
return found;
```

- [ ] **Step 8: Implement `Board::printSolution()`**

```cpp
void Board::printSolution() {
    Serial.println("[Board] Solution:");
    Serial.print(toString().c_str());
    Serial.println();
}
```

- [ ] **Step 9: Verify compilation**

```bash
pio run
```

Expected: `SUCCESS` with no errors.

---

### Task 3: Create `SolverTask`

**Files:**
- Create: `src/solver/SolverTask.h`
- Create: `src/solver/SolverTask.cpp`

`SolverTask` is the only class that calls FreeRTOS task APIs. It owns `Board*` and `TaskHandle_t` and cleans them up in `stop()`.

- [ ] **Step 1: Create `src/solver/SolverTask.h`**

```cpp
/*
 * Created by Ed Fillingham on 09/05/2026.
 *
 * Manages a background FreeRTOS solver task on Core 0.
 * Call start() when a valid arrangement is detected and stop() to cancel.
 */

#ifndef SOLVER_TASK_H
#define SOLVER_TASK_H

#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "utils/math/maths.h"
#include "solver/Board.h"

class SolverTask {
public:
    SolverTask();
    ~SolverTask();

    /** @brief Cancel any running solve, allocate a new Board, and spawn the Core 0 task. */
    void start(std::vector<Coord> blockers);

    /** @brief Cancel the running solve and block until the task exits, then free the Board. */
    void stop();

    /** @brief True once the task has exited (solution found, timed out, or cancelled). */
    bool isDone() const;

    /** @brief True if the last solve found a solution. */
    bool hasSolution() const;

private:
    static void taskFn(void* param);

    Board*            _board          = nullptr;
    TaskHandle_t      _handle         = nullptr;
    SemaphoreHandle_t _doneSem        = nullptr;
    volatile bool     _done           = false;
    volatile bool     _foundSolution  = false;
    bool              _active         = false;

    static constexpr int   TASK_STACK_SIZE  = 8192;
    static constexpr int   TASK_PRIORITY    = 1;
    static constexpr int   TASK_CORE        = 0;
    static constexpr int   SOLUTION_LIMIT   = 1;
    static constexpr int   TIME_LIMIT_S     = 300;
};

#endif // SOLVER_TASK_H
```

- [ ] **Step 2: Create `src/solver/SolverTask.cpp`**

```cpp
/*
 * Created by Ed Fillingham on 09/05/2026.
*/

#include "solver/SolverTask.h"
#include "Arduino.h"

SolverTask::SolverTask() {
    _doneSem = xSemaphoreCreateBinary();
}

SolverTask::~SolverTask() {
    stop();
    if (_doneSem) vSemaphoreDelete(_doneSem);
}

void SolverTask::start(std::vector<Coord> blockers) {
    stop();

    // Consume any leftover signal from a task that finished naturally before stop() was called.
    xSemaphoreTake(_doneSem, 0);

    _board = new Board(blockers, SOLUTION_LIMIT, TIME_LIMIT_S);
    _done          = false;
    _foundSolution = false;
    _active        = true;

    xTaskCreatePinnedToCore(
        taskFn, "solver", TASK_STACK_SIZE,
        this, TASK_PRIORITY, &_handle, TASK_CORE);

    Serial.printf("[SolverTask] start() — task created on Core %d ptr=%p\n",
        TASK_CORE, (void*)_board);
}

void SolverTask::stop() {
    if (!_active) return;

    Serial.printf("[SolverTask] stop() — cancelling ptr=%p\n", (void*)_board);

    if (!_done) _board->cancel();

    xSemaphoreTake(_doneSem, portMAX_DELAY); // blocks until taskFn signals completion

    _handle = nullptr; // task already deleted itself via vTaskDelete(NULL)
    delete _board;
    _board  = nullptr;
    _active = false;
    _done   = false;

    Serial.println("[SolverTask] stop() — cleaned up");
}

bool SolverTask::isDone() const {
    return !_active || _done;
}

bool SolverTask::hasSolution() const {
    return _foundSolution;
}

void SolverTask::taskFn(void* param) {
    SolverTask* self = static_cast<SolverTask*>(param);

    Serial.printf("[SolverTask] task running on Core %d\n", xPortGetCoreID());

    bool solved = self->_board->solve();
    self->_foundSolution = solved;

    if (solved) self->_board->printSolution();

    self->_done = true;
    xSemaphoreGive(self->_doneSem);
    vTaskDelete(NULL); // FreeRTOS tasks must not return
}
```

- [ ] **Step 3: Verify compilation**

```bash
pio run
```

Expected: `SUCCESS` with no errors.

---

### Task 4: Wire `SolverTask` into `SolverMenuScreen`

**Files:**
- Modify: `src/menu/SolverMenuScreen.h`
- Modify: `src/menu/SolverMenuScreen.cpp`

- [ ] **Step 1: Add `SolverTask` and `_lastBlockers` to `SolverMenuScreen.h`**

Add the include near the top of the header (after the existing includes):

```cpp
#include "solver/SolverTask.h"
```

In the `private:` section, after the existing `// ---- Grid / scan state ----` block, add:

```cpp
// ---- Solver ----
SolverTask          _solver;
std::vector<Coord>  _lastBlockers;
```

- [ ] **Step 2: Modify `checkValidity()` in `SolverMenuScreen.cpp` to trigger `_solver`**

Replace the entire `checkValidity()` body with:

```cpp
void SolverMenuScreen::checkValidity() {
    int count = 0;
    std::vector<Coord> coords;
    coords.reserve(GRID_ROWS * GRID_COLS);

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (_grid[r][c]) {
                count++;
                coords.push_back(Coord{c, r});
            }
        }
    }

    if (count < 7) {
        _solvable = false;
        strncpy(_reason, "Too few", sizeof(_reason) - 1);
    } else if (count > 7) {
        _solvable = false;
        strncpy(_reason, "Too many", sizeof(_reason) - 1);
    } else {
        if (CombinationGenerator::validCombination(coords)) {
            _solvable = true;
            _reason[0] = '\0';
        } else {
            _solvable = false;
            strncpy(_reason, "Invalid", sizeof(_reason) - 1);
        }
    }
    _reason[sizeof(_reason) - 1] = '\0';

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
}
```

- [ ] **Step 3: Add `_solver.stop()` and `_lastBlockers.clear()` to `onExit()`**

Replace the current empty `onExit()`:

```cpp
void SolverMenuScreen::onExit() {
    _solver.stop();
    _lastBlockers.clear();
}
```

- [ ] **Step 4: Verify compilation**

```bash
pio run
```

Expected: `SUCCESS` with no errors.

---

### Task 5: Integration test via serial monitor

- [ ] **Step 1: Upload firmware and open serial monitor**

```bash
pio run --target upload && pio device monitor
```

- [ ] **Step 2: Navigate to Solver screen and place fewer than 7 blockers**

Expected serial output: nothing solver-related (too few blockers).

- [ ] **Step 3: Place exactly 7 blockers in a valid arrangement**

Expected serial output (in order):
```
[SolverMenu] Valid arrangement detected, starting solve
[Board] Created ptr=0x... blockers=[(x, y), ...]
[SolverTask] start() — task created on Core 0 ptr=0x...
[SolverTask] task running on Core 0
[Board] solve() started ptr=0x...
[Board] solve() done ptr=0x... — FOUND   (or NOT FOUND)
[Board] Solution:
  <coloured board grid>
```

- [ ] **Step 4: Remove one blocker (invalidate the arrangement)**

Expected serial output:
```
[SolverMenu] Arrangement changed/invalid, stopping solve
[SolverTask] stop() — cancelling ptr=0x...
[Board] Destroyed ptr=0x...
[SolverTask] stop() — cleaned up
```

If the solve had already finished before removal, `stop()` still runs — `_done` is true so `cancel()` is skipped, the semaphore is taken immediately, and cleanup proceeds.

- [ ] **Step 5: Press Back to exit the Solver screen**

Expected: `_solver.stop()` is called from `onExit()`. If no solve is active, it returns immediately (no Serial output, since `_active` is false).

- [ ] **Step 6: Re-enter Solver screen and place a new valid arrangement**

Verify a fresh `[Board] Created ptr=0x...` print appears with a different pointer value, confirming the previous Board was freed and a new one allocated.
