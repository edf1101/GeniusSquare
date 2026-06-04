# Buzzer Integration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Integrate a passive buzzer on pin 46 to play non-blocking sound effects at key UI and game events.

**Architecture:** A global `Buzzer` singleton holds a static note queue advanced by `update()` each loop iteration. The ESP32-S3 `ledc` peripheral generates PWM in hardware so tones play without blocking the main loop. Screens call `Buzzer::play(SoundEffect)` directly, preempting any in-progress sound.

**Tech Stack:** ESP32-S3 Arduino (PlatformIO), `ledc` peripheral (`ledcSetup`, `ledcAttachPin`, `ledcWriteTone`, `ledcWrite`), C++17.

---

## File Map

| File | Action | Responsibility |
|------|--------|----------------|
| `src/utils/Buzzer.h` | Create | `SoundEffect` enum, `Note` struct, `Buzzer` class declaration |
| `src/utils/Buzzer.cpp` | Create | Note sequences, ledc setup, non-blocking sequencer |
| `src/main.cpp` | Modify | Global `buzzer(46)`, `begin()`, `update()`, `POWER_ON`/`POWER_OFF` |
| `src/menu/CarouselMenuScreen.cpp` | Modify | `MENU_TICK` on encoder, `MENU_SELECT` on button |
| `src/menu/ListMenuScreen.cpp` | Modify | `MENU_TICK` on encoder, `MENU_SELECT` on button |
| `src/menu/ArrangementMenuScreen.cpp` | Modify | `MENU_TICK` on encoder, `MENU_SELECT` on button |
| `src/menu/SolverMenuScreen.cpp` | Modify | `MENU_TICK` on encoder, `MENU_SELECT` on button |
| `src/menu/PracticeGameScreen.cpp` | Modify | `PIECE_PLACED`, `COUNTDOWN_TICK`, `GAME_START`, `WIN`/`NEW_BEST` |

---

## Task 1: Create `Buzzer.h`

**Files:**
- Create: `src/utils/Buzzer.h`

- [ ] **Step 1: Create the header**

```cpp
/*
 * Created by Ed Fillingham on 02/06/2026.
 *
 * Non-blocking buzzer driver. Plays predefined sound effect sequences
 * via the ESP32-S3 ledc peripheral. Call update() every loop iteration.
 */

#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

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

class Buzzer {
public:
    explicit Buzzer(uint8_t pin);

    /** @brief Initialise ledc peripheral. Call once in setup(). */
    void begin();

    /**
     * @brief Start playing a sound effect, interrupting any current sound.
     * @param effect The sound effect to play.
     */
    void play(SoundEffect effect);

    /** @brief Advance the note sequencer. Call every loop() iteration. */
    void update();

    struct Note {
        uint16_t freq;        ///< Hz; 0 = silence
        uint16_t durationMs;
    };

private:
    static constexpr uint8_t  CHANNEL    = 0;
    static constexpr uint8_t  RESOLUTION = 8;
    static constexpr uint8_t  DUTY       = 128; ///< 50% duty — max loudness
    static constexpr uint8_t  QUEUE_SIZE = 8;

    uint8_t       _pin;
    Note          _queue[QUEUE_SIZE];
    uint8_t       _head;
    uint8_t       _count;
    unsigned long _noteStartMs;
    bool          _playing;

    void enqueue(const Note* notes, uint8_t len);
    void startNote(const Note& note);
    void silence();
};

#endif // BUZZER_H
```

---

## Task 2: Create `Buzzer.cpp`

**Files:**
- Create: `src/utils/Buzzer.cpp`

- [ ] **Step 1: Create the implementation**

```cpp
/*
 * Created by Ed Fillingham on 02/06/2026.
 *
 * Buzzer — non-blocking sound effect sequencer using ESP32-S3 ledc.
 */

#include "utils/Buzzer.h"

// ---- Frequency constants (Hz) ----
static constexpr uint16_t C4  = 261;
static constexpr uint16_t E4  = 330;
static constexpr uint16_t G4  = 392;
static constexpr uint16_t A4  = 440;
static constexpr uint16_t C5  = 523;
static constexpr uint16_t E5  = 659;
static constexpr uint16_t G5  = 784;
static constexpr uint16_t A5  = 880;
static constexpr uint16_t B5  = 988;
static constexpr uint16_t C6  = 1047;

// ---- Sound sequences ----
static const Buzzer::Note SEQ_POWER_ON[]      = {{E5,150},{G5,150},{B5,200}};
static const Buzzer::Note SEQ_MENU_TICK[]     = {{A4, 40}};
static const Buzzer::Note SEQ_MENU_SELECT[]   = {{C5, 60},{E5, 80}};
static const Buzzer::Note SEQ_COUNTDOWN[]     = {{A4,120}};
static const Buzzer::Note SEQ_GAME_START[]    = {{E5,100},{A5,200}};
static const Buzzer::Note SEQ_PIECE_PLACED[]  = {{G4, 50}};
static const Buzzer::Note SEQ_WIN[]           = {{C5,100},{E5,100},{G5,200}};
static const Buzzer::Note SEQ_NEW_BEST[]      = {{C5, 80},{E5, 80},{G5, 80},{C6,250}};
static const Buzzer::Note SEQ_POWER_OFF[]     = {{G4,100},{E4,100},{C4,200}};

Buzzer::Buzzer(uint8_t pin)
    : _pin(pin), _head(0), _count(0), _noteStartMs(0), _playing(false)
{
    memset(_queue, 0, sizeof(_queue));
}

void Buzzer::begin() {
    ledcSetup(CHANNEL, 2000, RESOLUTION);
    ledcAttachPin(_pin, CHANNEL);
    ledcWrite(CHANNEL, 0);
}

void Buzzer::play(SoundEffect effect) {
    silence(); // stop any current note immediately
    _head  = 0;
    _count = 0;

    switch (effect) {
        case SoundEffect::POWER_ON:
            enqueue(SEQ_POWER_ON,     sizeof(SEQ_POWER_ON)     / sizeof(Note)); break;
        case SoundEffect::MENU_TICK:
            enqueue(SEQ_MENU_TICK,    sizeof(SEQ_MENU_TICK)    / sizeof(Note)); break;
        case SoundEffect::MENU_SELECT:
            enqueue(SEQ_MENU_SELECT,  sizeof(SEQ_MENU_SELECT)  / sizeof(Note)); break;
        case SoundEffect::COUNTDOWN_TICK:
            enqueue(SEQ_COUNTDOWN,    sizeof(SEQ_COUNTDOWN)    / sizeof(Note)); break;
        case SoundEffect::GAME_START:
            enqueue(SEQ_GAME_START,   sizeof(SEQ_GAME_START)   / sizeof(Note)); break;
        case SoundEffect::PIECE_PLACED:
            enqueue(SEQ_PIECE_PLACED, sizeof(SEQ_PIECE_PLACED) / sizeof(Note)); break;
        case SoundEffect::WIN:
            enqueue(SEQ_WIN,          sizeof(SEQ_WIN)          / sizeof(Note)); break;
        case SoundEffect::NEW_BEST:
            enqueue(SEQ_NEW_BEST,     sizeof(SEQ_NEW_BEST)     / sizeof(Note)); break;
        case SoundEffect::POWER_OFF:
            enqueue(SEQ_POWER_OFF,    sizeof(SEQ_POWER_OFF)    / sizeof(Note)); break;
    }
}

void Buzzer::update() {
    if (!_playing) return;

    if (millis() - _noteStartMs >= _queue[_head].durationMs) {
        _head  = (_head + 1) % QUEUE_SIZE;
        _count--;
        if (_count == 0) {
            _playing = false;
            silence();
            return;
        }
        startNote(_queue[_head]);
    }
}

// ---- Private ----

void Buzzer::enqueue(const Note* notes, uint8_t len) {
    if (len == 0) return;
    uint8_t tail = (_head + _count) % QUEUE_SIZE;
    for (uint8_t i = 0; i < len && _count < QUEUE_SIZE; i++) {
        _queue[tail] = notes[i];
        tail = (tail + 1) % QUEUE_SIZE;
        _count++;
    }
    startNote(_queue[_head]);
    _playing = true;
}

void Buzzer::startNote(const Note& note) {
    _noteStartMs = millis();
    if (note.freq == 0) {
        ledcWrite(CHANNEL, 0);
    } else {
        ledcWriteTone(CHANNEL, note.freq);
        ledcWrite(CHANNEL, DUTY);
    }
}

void Buzzer::silence() {
    ledcWrite(CHANNEL, 0);
}
```

- [ ] **Step 2: Build to verify it compiles**

```bash
pio run
```

Expected: build succeeds with no errors.

---

## Task 3: Wire `Buzzer` into `main.cpp`

**Files:**
- Modify: `src/main.cpp`

Current state of relevant sections (for reference):
- Line 28: last `#define` (`LATCH_PIN 21`)
- Line 36: `TFT_eSPI tft = TFT_eSPI();` — global objects block starts here
- Line 116: `void setup() {`
- Line 118: `Serial.begin(115200);` — first line of setup body
- Lines 94-102: `onPowerOffSelected` function
- Line 188: `rot.poll();` — first line of loop body

- [ ] **Step 1: Add `#include` and global instance**

After `#include "utils/PracticeScores.h"` (line 23), add:
```cpp
#include "utils/Buzzer.h"
```

After the `TFT_eSPI tft = TFT_eSPI();` line (line 36), add:
```cpp
/// Buzzer on pin 46 — non-blocking ledc-driven sound effects
Buzzer buzzer(46);
```

- [ ] **Step 2: Call `buzzer.begin()` and play `POWER_ON` in `setup()`**

In `setup()`, after `GridScanner::init();`, add:
```cpp
    buzzer.begin();
    buzzer.play(SoundEffect::POWER_ON);
```

- [ ] **Step 3: Play `POWER_OFF` in `onPowerOffSelected()` with blocking delay**

Replace the `onPowerOffSelected` function body:
```cpp
void onPowerOffSelected(ScreenManager&)
{
    buzzer.play(SoundEffect::POWER_OFF);
    delay(450); // allow the 400ms sequence to finish before cutting power
    while (1)
    {
        digitalWrite(LATCH_PIN, LOW);
        pinMode(LATCH_PIN, OUTPUT);
        delay(500);
    }
}
```

- [ ] **Step 4: Call `buzzer.update()` in `loop()`**

In `loop()`, after `screenManager.render();`, add:
```cpp
    buzzer.update();
```

- [ ] **Step 5: Build to verify**

```bash
pio run
```

Expected: build succeeds with no errors.

---

## Task 4: Add sounds to menu screens

**Files:**
- Modify: `src/menu/CarouselMenuScreen.cpp`
- Modify: `src/menu/ListMenuScreen.cpp`
- Modify: `src/menu/ArrangementMenuScreen.cpp`
- Modify: `src/menu/SolverMenuScreen.cpp`

All four screens need the same treatment: `MENU_TICK` on any encoder movement, `MENU_SELECT` on button press. The `Buzzer` global is accessed via `extern` declaration at the top of each `.cpp`.

- [ ] **Step 1: Update `CarouselMenuScreen.cpp`**

Add after the existing `#include` block at the top of `src/menu/CarouselMenuScreen.cpp`:
```cpp
#include "utils/Buzzer.h"
extern Buzzer buzzer;
```

In `onEncoderChange()` (line 139), add at the very start of the function body (before the `if (_count == 0) return;` guard):
```cpp
    buzzer.play(SoundEffect::MENU_TICK);
```

In `onButtonPress()` (line 153), add at the very start of the function body (before the `if (_count == 0) return;` guard):
```cpp
    buzzer.play(SoundEffect::MENU_SELECT);
```

- [ ] **Step 2: Update `ListMenuScreen.cpp`**

Add after the existing `#include` block at the top of `src/menu/ListMenuScreen.cpp`:
```cpp
#include "utils/Buzzer.h"
extern Buzzer buzzer;
```

In `onEncoderChange()` (line 154), add after the `if (_selectedIndex == prevIndex) return;` early-exit (line 163), so the tick only fires when the selection actually changes:
```cpp
    buzzer.play(SoundEffect::MENU_TICK);
```

In `onButtonPress()` (line 206), add at the very start of the function body:
```cpp
    buzzer.play(SoundEffect::MENU_SELECT);
```

- [ ] **Step 3: Update `ArrangementMenuScreen.cpp`**

Add after the existing `#include` block at the top of `src/menu/ArrangementMenuScreen.cpp`:
```cpp
#include "utils/Buzzer.h"
extern Buzzer buzzer;
```

In `onEncoderChange()` (line 120), add after the `if (_selectedIndex == prevIndex) return;` early-exit (line 128):
```cpp
    buzzer.play(SoundEffect::MENU_TICK);
```

In `onButtonPress()` (line 169), add at the very start of the function body:
```cpp
    buzzer.play(SoundEffect::MENU_SELECT);
```

- [ ] **Step 4: Update `SolverMenuScreen.cpp`**

Add after the existing `#include` block at the top of `src/menu/SolverMenuScreen.cpp`:
```cpp
#include "utils/Buzzer.h"
extern Buzzer buzzer;
```

In `onEncoderChange()` (line 189), add inside the `if (oldIndex != _selectedIndex)` block, as the first line inside it:
```cpp
        buzzer.play(SoundEffect::MENU_TICK);
```

In `onButtonPress()` (line 217), add at the very start of the function body:
```cpp
    buzzer.play(SoundEffect::MENU_SELECT);
```

- [ ] **Step 5: Build to verify**

```bash
pio run
```

Expected: build succeeds with no errors.

---

## Task 5: Add sounds to `PracticeGameScreen`

**Files:**
- Modify: `src/menu/PracticeGameScreen.cpp`

Four events:
1. `PIECE_PLACED` — when `_confirmedCount` increases in `scanGrid()` (line 344)
2. `COUNTDOWN_TICK` — when `_countdownRemaining` decreases in `update()` (line 100)
3. `GAME_START` — when phase transitions to `PLAYING` in `update()` (line 93)
4. `WIN` or `NEW_BEST` — in `finishGame()` (line 397)

- [ ] **Step 1: Add include and extern**

Add after the existing `#include` block at the top of `src/menu/PracticeGameScreen.cpp`:
```cpp
#include "utils/Buzzer.h"
extern Buzzer buzzer;
```

- [ ] **Step 2: Add `PIECE_PLACED` in `scanGrid()`**

In `scanGrid()`, the block at line 344 reads:
```cpp
        if (placed != _confirmedCount) {
            _confirmedCount = placed;
            _panelDirty = true;
        }
```

Replace it with:
```cpp
        if (placed != _confirmedCount) {
            if (placed > _confirmedCount) buzzer.play(SoundEffect::PIECE_PLACED);
            _confirmedCount = placed;
            _panelDirty = true;
        }
```

- [ ] **Step 3: Add `COUNTDOWN_TICK` and `GAME_START` in `update()`**

In `update()`, the countdown block at line 99 reads:
```cpp
            if (rem != _countdownRemaining) {
                _countdownRemaining = rem;
                _panelDirty = true;
            }
```

Replace it with:
```cpp
            if (rem != _countdownRemaining) {
                _countdownRemaining = rem;
                _panelDirty = true;
                buzzer.play(SoundEffect::COUNTDOWN_TICK);
            }
```

The phase transition at line 93 reads:
```cpp
            _phase          = Phase::PLAYING;
            _timerStartMs   = curMs;
            _elapsedSeconds = 0.0f;
            _panelDirty     = true;
```

Replace it with:
```cpp
            _phase          = Phase::PLAYING;
            _timerStartMs   = curMs;
            _elapsedSeconds = 0.0f;
            _panelDirty     = true;
            buzzer.play(SoundEffect::GAME_START);
```

- [ ] **Step 4: Add `WIN` / `NEW_BEST` in `finishGame()`**

In `finishGame()`, at line 400 the `if (isNewBest)` block reads:
```cpp
    if (isNewBest) {
        *_bestSeconds = _elapsedSeconds;
        uint8_t toStore = (_elapsedSeconds >= 255.0f) ? 255u : (uint8_t)_elapsedSeconds;
        if (toStore == 0) toStore = 1;
        PracticeScores::save(_arrangementIndex, toStore);
    }
    float best = (_bestSeconds != nullptr) ? *_bestSeconds : 0.0f;
    _scoreScreen.setResult(_elapsedSeconds, best, isNewBest);
    _manager.push(&_scoreScreen);
```

Replace it with:
```cpp
    if (isNewBest) {
        *_bestSeconds = _elapsedSeconds;
        uint8_t toStore = (_elapsedSeconds >= 255.0f) ? 255u : (uint8_t)_elapsedSeconds;
        if (toStore == 0) toStore = 1;
        PracticeScores::save(_arrangementIndex, toStore);
        buzzer.play(SoundEffect::NEW_BEST);
    } else {
        buzzer.play(SoundEffect::WIN);
    }
    float best = (_bestSeconds != nullptr) ? *_bestSeconds : 0.0f;
    _scoreScreen.setResult(_elapsedSeconds, best, isNewBest);
    _manager.push(&_scoreScreen);
```

- [ ] **Step 5: Build to verify**

```bash
pio run
```

Expected: build succeeds with no errors.

- [ ] **Step 6: Upload and verify on device**

```bash
pio run --target upload && pio device monitor
```

Verify manually:
- Power on → 3-note rising chime plays
- Rotating encoder on any menu screen → short tick on each step
- Pressing button on any menu item → two-note confirm sound
- Entering a practice game, placing blockers → soft tick per blocker confirmed
- Countdown 3→2→1 → tick each second
- GO! transition → rising two-note
- Completing the board → arpeggio (WIN or NEW_BEST)
- Selecting "Power Off" → descending 3-note before device cuts power
