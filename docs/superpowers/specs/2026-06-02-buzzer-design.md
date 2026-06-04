# Buzzer Integration Design

**Date:** 2026-06-02  
**Status:** Approved

## Overview

Integrate a passive buzzer on pin 46 into the GeniusSquare firmware. Sounds play asynchronously via a non-blocking note sequencer updated from the main loop. All effects are short and snappy. Volume is fixed at maximum (50% duty cycle).

---

## Architecture

A global `Buzzer` instance lives in `main.cpp` alongside `tft` and `screenManager`. Screens call `Buzzer::play(SoundEffect)` directly — no changes to the `Screen` interface or `ScreenManager`.

The ESP32-S3 `ledc` peripheral drives the buzzer in hardware; no ISR or RTOS task is needed. `Buzzer::update()` is called every `loop()` iteration to advance the note sequencer.

---

## `Buzzer` Class (`src/utils/Buzzer.h` / `.cpp`)

### `Note` struct

```cpp
struct Note {
    uint16_t freq;        // Hz; 0 = silence (rest)
    uint16_t durationMs;
};
```

### `SoundEffect` enum

```cpp
enum class SoundEffect : uint8_t {
    POWER_ON,
    MENU_TICK,
    MENU_SELECT,
    COUNTDOWN_TICK,
    GAME_START,
    PIECE_PLACED,
    WIN,
    NEW_BEST,
    POWER_OFF,
};
```

### Public API

```cpp
class Buzzer {
public:
    explicit Buzzer(uint8_t pin);
    void begin();                      // ledcSetup + ledcAttachPin
    void play(SoundEffect effect);     // interrupt current sound, start new sequence
    void update();                     // advance sequencer — call from loop()
};
```

### Internals

- Static queue: `Note _queue[8]` — ring buffer with `_head`, `_tail`, `_count`.
- `unsigned long _noteStartMs` — when the current note started.
- `bool _playing` — true while a note sequence is in progress.
- `play()` preempts any in-progress sound immediately (no sound is worth waiting for).
- `update()` checks `millis() - _noteStartMs >= currentNote.durationMs`, advances head, calls `ledcWriteTone()` or `ledcWrite(channel, 0)` for silence.

### ledc Configuration

| Parameter | Value |
|-----------|-------|
| Pin | 46 |
| Channel | `LEDC_CHANNEL_0` |
| Resolution | 8-bit |
| Duty cycle | 128 (50% — max loudness) |
| Silence | `ledcWrite(channel, 0)` |

---

## Sound Sequences

| Effect | Notes | Total duration |
|--------|-------|----------------|
| `POWER_ON` | E5(150ms) → G5(150ms) → B5(200ms) | 500ms |
| `MENU_TICK` | A4(40ms) | 40ms |
| `MENU_SELECT` | C5(60ms) → E5(80ms) | 140ms |
| `COUNTDOWN_TICK` | A4(120ms) | 120ms |
| `GAME_START` | E5(100ms) → A5(200ms) | 300ms |
| `PIECE_PLACED` | G4(50ms) | 50ms |
| `WIN` | C5(100ms) → E5(100ms) → G5(200ms) | 400ms |
| `NEW_BEST` | C5(80ms) → E5(80ms) → G5(80ms) → C6(250ms) | 490ms |
| `POWER_OFF` | G4(100ms) → E4(100ms) → C4(200ms) | 400ms |

Frequencies (Hz): C4=261, E4=330, G4=392, A4=440, C5=523, E5=659, G5=784, A5=880, B5=988, C6=1047.

---

## Integration Points

| Call site | File | Event |
|-----------|------|-------|
| `setup()` — after display init | `main.cpp` | `POWER_ON` |
| `onPowerOffSelected()` — before latch loop | `main.cpp` | `POWER_OFF` (+ `delay(450)` to let it finish) |
| `onEncoderChange()` | `CarouselMenuScreen.cpp`, `ListMenuScreen.cpp`, `ArrangementMenuScreen.cpp`, `SolverMenuScreen.cpp` | `MENU_TICK` |
| `onButtonPress()` — on confirm | `CarouselMenuScreen.cpp`, `ListMenuScreen.cpp`, `ArrangementMenuScreen.cpp`, `SolverMenuScreen.cpp` | `MENU_SELECT` |
| `scanGrid()` — when `_confirmedCount` increases | `PracticeGameScreen.cpp` | `PIECE_PLACED` |
| `update()` — when `_countdownRemaining` decreases | `PracticeGameScreen.cpp` | `COUNTDOWN_TICK` |
| `update()` — when phase flips to `PLAYING` | `PracticeGameScreen.cpp` | `GAME_START` |
| `finishGame()` | `PracticeGameScreen.cpp` | `NEW_BEST` or `WIN` |
| `loop()` — every iteration | `main.cpp` | `buzzer.update()` |

### Note on `POWER_OFF`

The power-off path enters an infinite `while(1)` loop immediately. A `delay(450)` is inserted before the latch goes LOW so the 400ms descending sequence has time to complete.

---

## Files Changed

| File | Change |
|------|--------|
| `src/utils/Buzzer.h` | New file |
| `src/utils/Buzzer.cpp` | New file |
| `src/main.cpp` | Add `Buzzer buzzer(46)`, `buzzer.begin()`, `buzzer.update()`, `POWER_ON`/`POWER_OFF` calls |
| `src/menu/CarouselMenuScreen.cpp` | Add `MENU_TICK` / `MENU_SELECT` calls |
| `src/menu/ListMenuScreen.cpp` | Add `MENU_TICK` / `MENU_SELECT` calls |
| `src/menu/ArrangementMenuScreen.cpp` | Add `MENU_TICK` / `MENU_SELECT` calls |
| `src/menu/SolverMenuScreen.cpp` | Add `MENU_TICK` / `MENU_SELECT` calls |
| `src/menu/PracticeGameScreen.cpp` | Add `PIECE_PLACED`, `COUNTDOWN_TICK`, `GAME_START`, `WIN`/`NEW_BEST` calls |
