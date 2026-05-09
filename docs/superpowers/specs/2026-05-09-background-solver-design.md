# Background Solver Design

**Date:** 2026-05-09
**Status:** Approved

## Overview

When `SolverMenuScreen` detects a valid 7-blocker arrangement, it immediately starts solving it on Core 0 (background FreeRTOS task) while Core 1 continues running the main Arduino loop. When the arrangement changes or the screen exits, the in-progress solve is cancelled and the Board is deleted. All lifecycle events are logged to Serial.

---

## Components

### 1. `Board` (modified — `src/solver/Board.h/.cpp`)

**New fields:**
- `volatile bool _cancel` — set externally to request early abort; checked alongside the time limit

**New public methods:**
- `void cancel()` — sets `_cancel = true`
- `void printSolution()` — prints `toString()` to Serial; called after a successful solve

**Modified private method:**
- `recursiveSolve()` — both early-exit checks become `rootBoard->getOutOfTime() || rootBoard->_cancel`

**Debug Serial prints added to:**
- Constructor — prints blocker list (using `Coord::toString()`)
- Destructor — prints pointer-deleted confirmation
- `solve()` — prints "solving started" on entry, "found" or "not found" on exit
- `printSolution()` — prints `toString()` output

### 2. `SolverTask` (new — `src/solver/SolverTask.h/.cpp`)

Encapsulates the FreeRTOS task and Board lifecycle. `SolverMenuScreen` is the only consumer.

**Public API:**

| Method | Behaviour |
|---|---|
| `SolverTask()` | Creates the binary semaphore |
| `~SolverTask()` | Calls `stop()` |
| `void start(std::vector<Coord> blockers)` | Calls `stop()` first, allocates Board, spawns Core 0 task |
| `void stop()` | Cancels Board, blocks on semaphore until task exits, deletes Board |
| `bool isDone() const` | True when task has exited (success, timeout, or cancel) |
| `bool hasSolution() const` | True when `solve()` returned true |

**Private fields:**
- `Board* _board` — heap-allocated; null when no solve is running
- `TaskHandle_t _handle` — null when no task is running
- `SemaphoreHandle_t _doneSem` — binary semaphore; task gives it on exit, `stop()` takes it
- `volatile bool _done` — set by task function just before signalling semaphore
- `volatile bool _foundSolution` — set by task function after `solve()` returns

**Static task function (pinned to Core 0, stack 8192 bytes, priority 1):**
```
taskFn(void* param):
    call _board->solve()
    set _foundSolution = result
    if result: call _board->printSolution()
    set _done = true
    xSemaphoreGive(_doneSem)
    vTaskDelete(NULL)          // FreeRTOS tasks must not return
```

**`start()` sequence:**
1. Call `stop()` (cleans up previous solve if any)
2. `xSemaphoreTake(_doneSem, 0)` — consume any leftover signal from a task that finished naturally before `stop()` was called
3. Allocate `new Board(blockers, 1, 300)` — solutionLimit=1, timeLimit=300s
4. Reset `_done = false`, `_foundSolution = false`
5. `xTaskCreatePinnedToCore(taskFn, "solver", 8192, this, 1, &_handle, 0)`
6. Serial print confirming task created

**`stop()` sequence:**
1. Return immediately if `_board == nullptr` (nothing running)
2. If `!_done`: call `_board->cancel()` (sets `_cancel = true`)
3. `xSemaphoreTake(_doneSem, portMAX_DELAY)` — waits for task to signal completion
4. `_handle = nullptr` (task already self-deleted via `vTaskDelete(NULL)`)
5. `delete _board; _board = nullptr`
6. `_done = false`
7. Serial print confirming cleanup complete

### 3. `SolverMenuScreen` (modified — `src/menu/SolverMenuScreen.h/.cpp`)

**New fields:**
- `SolverTask _solver` — member (not pointer); default-constructed in member init list
- `std::vector<Coord> _lastBlockers` — last blocker set passed to `_solver.start()`

**`checkValidity()` changes:**

After setting `_solvable`:
- If `_solvable == true`:
  - Build sorted `coords` vector (already row-major from nested loop, so ordering is consistent)
  - If `coords != _lastBlockers`: Serial print "new valid arrangement detected", call `_solver.start(coords)`, set `_lastBlockers = coords`
  - Otherwise: no-op (same arrangement, solve already running or done)
- If `_solvable == false` and `!_lastBlockers.empty()`:
  - Serial print "arrangement invalid, cancelling solve"
  - Call `_solver.stop()`
  - `_lastBlockers.clear()`

**`onExit()` changes:**
- Call `_solver.stop()`
- `_lastBlockers.clear()`

---

## Serial Debug Output Summary

| Source | Event | Message format |
|---|---|---|
| `Board` constructor | Board created | `[Board] Created ptr=0x… blockers=[A1, B3, …]` |
| `Board` destructor | Board deleted | `[Board] Destroyed ptr=0x…` |
| `Board::solve()` entry | Solve starts | `[Board] solve() started` |
| `Board::solve()` exit | Solve ends | `[Board] solve() done — FOUND` / `NOT FOUND` |
| `Board::printSolution()` | Solution printed | `[Board] Solution:\n<toString() output>` |
| `SolverTask::start()` | Task spawned | `[SolverTask] start() — task created on Core 0` |
| `SolverTask::stop()` | Cleanup begins | `[SolverTask] stop() — cancelling` |
| `SolverTask::stop()` | Cleanup done | `[SolverTask] stop() — cleaned up` |
| `SolverMenuScreen` | New valid arrangement | `[SolverMenu] Valid arrangement detected, starting solve` |
| `SolverMenuScreen` | Arrangement invalidated | `[SolverMenu] Arrangement changed/invalid, stopping solve` |

---

## Data Placement

- `Board` is heap-allocated on Core 1 (inside `start()`); heap is shared between cores on ESP32-S3, so Core 0 can access it freely.
- `_cancel` and `_done` are `volatile` to prevent compiler reordering across cores.
- No mutex is needed: the semaphore in `stop()` ensures Core 1 does not touch `_board` while Core 0 is still inside `solve()`.

---

## Stack Size Rationale

Each level of `recursiveSolve` copies a `Board` (~240 bytes stack) and a `Piece` (~128 bytes stack). With up to 10 pieces (10 recursion levels), peak stack is ~4-5 KB. 8192 bytes gives a comfortable margin.

---

## Out of Scope

- Displaying the solution on-screen (future work)
- Multiple solutions (solutionLimit is fixed at 1)
- Persisting solutions across screen transitions
