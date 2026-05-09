# Grid Scanner Design

**Date:** 2026-05-09

## Overview

A hardware abstraction module for reading the 6×6 resistive sensor matrix on the GeniusSquare ESP32-S3 device. Exposes free functions under a `GridScanner` namespace — no class, no dynamic allocation.

## File Location

```
src/hardware/GridScanner.h
src/hardware/GridScanner.cpp
```

`src/input/` is reserved for interactive input devices (encoder, button). The grid matrix is a passive sensor, so `src/hardware/` is the appropriate home.

## Public API

```cpp
namespace GridScanner {
    void init();
    std::vector<std::vector<float>> analogReadMatrix();
    std::vector<std::vector<bool>>  digitalReadMatrix(float threshold = 0.122f);
}
```

### `init()`

Called once from `setup()`. Configures:
- `analogSetAttenuation(ADC_11db)` for full 0–3.3V range
- Row pins (`{14, 47, 48, 38, 41, 42}`) as `OUTPUT`, driven `HIGH` (inactive)
- Column pins (`{7, 8, 10, 9, 1, 2}`) as `ANALOG` with pull-ups and pull-downs explicitly disabled via `gpio_pullup_dis` / `gpio_pulldown_dis`

### `analogReadMatrix()`

Scans the full 6×6 matrix and returns a `vector<vector<float>>` of normalized values in `[0.0, 1.0]` (raw ADC ÷ 4095).

Scan sequence per row:
1. Drive row pin `LOW` (active)
2. Wait 800µs for signal settling
3. Dummy `analogRead(colPins[0])` + 50µs to prime the ADC MUX
4. For each column: double-read (throwaway + 10µs delay + actual read), store normalized value
5. Drive row pin `HIGH` (inactive)
6. Wait 100µs for column lines to drain before next row

Matrix dimensions are derived from the sizes of the row and column pin arrays.

### `digitalReadMatrix(float threshold)`

Calls `analogReadMatrix()` internally, then maps each value: `>= threshold → true`, `< threshold → false`. Returns `vector<vector<bool>>`.

Default threshold `0.122f` corresponds to `500 / 4095` — matching the `THRESHOLD = 500` constant in the reference code.

## Internal State

Pin arrays and timing constants are `static const` values inside `GridScanner.cpp`. They are not exposed in the header.

```cpp
static const int ROW_PINS[]  = { 14, 47, 48, 38, 41, 42 };
static const int COL_PINS[]  = {  7,  8, 10,  9,  1,  2 };
static const int NUM_ROWS    = sizeof(ROW_PINS) / sizeof(ROW_PINS[0]);
static const int NUM_COLS    = sizeof(COL_PINS) / sizeof(COL_PINS[0]);

static const int ROW_SETTLE_US  = 800;
static const int ADC_PRIME_US   = 50;
static const int ROW_DRAIN_US   = 100;
static const int ADC_SETTLE_US  = 10;
static const int ADC_MAX        = 4095;
```

## Integration

In `main.cpp` `setup()`:
```cpp
GridScanner::init();
```

In game logic, to read the board state:
```cpp
auto state = GridScanner::digitalReadMatrix();       // default threshold
auto raw   = GridScanner::analogReadMatrix();        // if raw data needed
```
