# Grid Scanner Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement a `GridScanner` namespace module that reads the 6×6 resistive sensor matrix and returns normalised analog or boolean occupancy data.

**Architecture:** Free functions under the `GridScanner` namespace in `src/hardware/`. `init()` configures pins once from `setup()`. `analogReadMatrix()` scans the hardware and returns `vector<vector<float>>` in [0.0, 1.0]. `digitalReadMatrix()` calls `analogReadMatrix()` internally and applies a threshold to produce `vector<vector<bool>>`.

**Tech Stack:** C++17, Arduino framework, ESP32-S3 Arduino ADC API (`analogRead`, `analogSetAttenuation`), ESP-IDF GPIO API (`gpio_pullup_dis`, `gpio_pulldown_dis`).

---

## File Map

| Action | Path | Responsibility |
|--------|------|----------------|
| Create | `src/hardware/GridScanner.h` | Public API declaration |
| Create | `src/hardware/GridScanner.cpp` | Hardware scan implementation |
| Modify | `src/main.cpp` | Call `GridScanner::init()` in `setup()` |

---

### Task 1: Create `GridScanner.h`

**Files:**
- Create: `src/hardware/GridScanner.h`

> Note: There is no automated test suite in this project. Verification is done via `pio run` (build check) and serial monitor output.

- [ ] **Step 1: Create the header file**

```cpp
/*
 * Created by Ed Fillingham on 09/05/2026.
 *
 * Hardware abstraction for the 6×6 resistive sensor matrix.
 */

#ifndef GENIUS_SQUARE_GRID_SCANNER_H
#define GENIUS_SQUARE_GRID_SCANNER_H

#include <vector>

namespace GridScanner {

    /**
     * @brief Initialise sensor matrix hardware.
     *
     * Sets ADC attenuation, row pins as OUTPUT (idle HIGH),
     * and column pins as ANALOG with pull-ups/pull-downs disabled.
     * Call once from setup().
     */
    void init();

    /**
     * @brief Scan the matrix and return normalised ADC readings.
     *
     * @return 6×6 vector of floats in [0.0, 1.0] (raw ADC ÷ 4095).
     *         Outer index is row, inner index is column.
     */
    std::vector<std::vector<float>> analogReadMatrix();

    /**
     * @brief Scan the matrix and return boolean occupancy state.
     *
     * Calls analogReadMatrix() internally.
     *
     * @param threshold Values >= threshold map to true. Default 0.122f (≈ 500/4095).
     * @return 6×6 vector of bools. Outer index is row, inner index is column.
     */
    std::vector<std::vector<bool>> digitalReadMatrix(float threshold = 0.122f);

} // namespace GridScanner

#endif // GENIUS_SQUARE_GRID_SCANNER_H
```

- [ ] **Step 2: Verify it builds**

```bash
pio run
```

Expected: build succeeds (header is not yet included anywhere, but must compile clean on its own if included).

- [ ] **Step 3: Commit**

```bash
git add src/hardware/GridScanner.h
git commit -m "feat: add GridScanner header"
```

---

### Task 2: Implement `init()` and `analogReadMatrix()`

**Files:**
- Create: `src/hardware/GridScanner.cpp`

- [ ] **Step 1: Create the implementation file**

```cpp
/*
 * Created by Ed Fillingham on 09/05/2026.
 *
 * Implementation of the GridScanner hardware abstraction.
 */

#include "GridScanner.h"
#include <Arduino.h>
#include <driver/gpio.h>

static const int ROW_PINS[] = { 14, 47, 48, 38, 41, 42 };
static const int COL_PINS[] = {  7,  8, 10,  9,  1,  2 };
static const int NUM_ROWS   = sizeof(ROW_PINS) / sizeof(ROW_PINS[0]);
static const int NUM_COLS   = sizeof(COL_PINS) / sizeof(COL_PINS[0]);

static const int   ROW_SETTLE_US = 800;
static const int   ADC_PRIME_US  = 50;
static const int   ROW_DRAIN_US  = 100;
static const int   ADC_SETTLE_US = 10;
static const float ADC_MAX_F     = 4095.0f;

void GridScanner::init() {
    analogSetAttenuation(ADC_11db);

    for (int i = 0; i < NUM_ROWS; i++) {
        pinMode(ROW_PINS[i], OUTPUT);
        digitalWrite(ROW_PINS[i], HIGH);
    }

    for (int i = 0; i < NUM_COLS; i++) {
        pinMode(COL_PINS[i], ANALOG);
        gpio_pulldown_dis((gpio_num_t)COL_PINS[i]);
        gpio_pullup_dis((gpio_num_t)COL_PINS[i]);
    }
}

std::vector<std::vector<float>> GridScanner::analogReadMatrix() {
    std::vector<std::vector<float>> matrix(NUM_ROWS, std::vector<float>(NUM_COLS, 0.0f));

    for (int r = 0; r < NUM_ROWS; r++) {
        digitalWrite(ROW_PINS[r], LOW);
        delayMicroseconds(ROW_SETTLE_US);

        // Prime the ADC MUX after the voltage swing on this row
        analogRead(COL_PINS[0]);
        delayMicroseconds(ADC_PRIME_US);

        for (int c = 0; c < NUM_COLS; c++) {
            // Throwaway read to clear ADC capacitor from previous channel
            analogRead(COL_PINS[c]);
            delayMicroseconds(ADC_SETTLE_US);
            matrix[r][c] = analogRead(COL_PINS[c]) / ADC_MAX_F;
        }

        digitalWrite(ROW_PINS[r], HIGH);
        delayMicroseconds(ROW_DRAIN_US);
    }

    return matrix;
}
```

- [ ] **Step 2: Build to check for compile errors**

```bash
pio run
```

Expected: build succeeds with no errors. `digitalReadMatrix` is declared but not yet defined — this will produce a linker error only if it is called, which it isn't yet. If the linker complains, add a stub (see next task).

- [ ] **Step 3: Commit**

```bash
git add src/hardware/GridScanner.cpp
git commit -m "feat: implement GridScanner init and analogReadMatrix"
```

---

### Task 3: Implement `digitalReadMatrix()`

**Files:**
- Modify: `src/hardware/GridScanner.cpp`

- [ ] **Step 1: Append `digitalReadMatrix` to `GridScanner.cpp`**

Add the following function after `analogReadMatrix()` in `src/hardware/GridScanner.cpp`:

```cpp
std::vector<std::vector<bool>> GridScanner::digitalReadMatrix(float threshold) {
    auto analog = analogReadMatrix();
    std::vector<std::vector<bool>> result(NUM_ROWS, std::vector<bool>(NUM_COLS, false));

    for (int r = 0; r < NUM_ROWS; r++) {
        for (int c = 0; c < NUM_COLS; c++) {
            result[r][c] = analog[r][c] >= threshold;
        }
    }

    return result;
}
```

- [ ] **Step 2: Build**

```bash
pio run
```

Expected: build succeeds, no errors or warnings.

- [ ] **Step 3: Commit**

```bash
git add src/hardware/GridScanner.cpp
git commit -m "feat: implement GridScanner digitalReadMatrix"
```

---

### Task 4: Wire into `main.cpp` and verify on hardware

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Add the include to `main.cpp`**

In `src/main.cpp`, add after the existing includes:

```cpp
#include "hardware/GridScanner.h"
```

- [ ] **Step 2: Call `GridScanner::init()` in `setup()`**

In `setup()`, after the `analogSetAttenuation(ADC_11db)` line, replace it with:

```cpp
GridScanner::init();
```

The `analogSetAttenuation(ADC_11db)` call that was in `setup()` is now handled inside `GridScanner::init()` — remove the original line to avoid a duplicate call.

- [ ] **Step 3: Add temporary serial verification to `setup()`**

At the end of `setup()`, add:

```cpp
// Temporary: print one analog scan to serial to verify matrix reads
auto analog = GridScanner::analogReadMatrix();
for (int r = 0; r < 6; r++) {
    for (int c = 0; c < 6; c++) {
        Serial.printf("%.3f ", analog[r][c]);
    }
    Serial.println();
}
Serial.println("--- digital (threshold 0.122) ---");
auto digital = GridScanner::digitalReadMatrix();
for (int r = 0; r < 6; r++) {
    for (int c = 0; c < 6; c++) {
        Serial.printf("%d ", digital[r][c] ? 1 : 0);
    }
    Serial.println();
}
```

- [ ] **Step 4: Build, upload, and check serial output**

```bash
pio run --target upload && pio device monitor
```

Expected serial output: a 6×6 grid of floats near 0.0 for empty cells (no piece placed). Cells with a game piece present should read notably higher (towards 1.0). The digital grid should show `0` for empty and `1` for occupied cells.

- [ ] **Step 5: Remove the temporary verification block**

Delete the serial print block added in Step 3 from `setup()`.

- [ ] **Step 6: Build to confirm clean**

```bash
pio run
```

Expected: build succeeds.

- [ ] **Step 7: Commit**

```bash
git add src/hardware/GridScanner.h src/hardware/GridScanner.cpp src/main.cpp
git commit -m "feat: wire GridScanner into main setup"
```
