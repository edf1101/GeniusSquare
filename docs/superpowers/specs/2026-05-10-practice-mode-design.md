# Practice Mode Design

**Date:** 2026-05-10  
**Status:** Approved

## Overview

Practice mode lets a user select one of 100 pre-generated puzzle arrangements and race against the clock to fill the board. After selecting an arrangement from the existing `ArrangementMenuScreen`, the player is taken through a two-phase game screen, then a results screen.

## Screen Architecture & Navigation

Two new screens:

- `PracticeGameScreen` — owns all in-game logic for one selected arrangement
- `PracticeScoreScreen` — displays results after the game ends

Navigation flow:
```
Main Menu → Practice → ArrangementMenuScreen
  → [select item] → PracticeGameScreen (PLACING phase)
  → [all blockers placed] → PracticeGameScreen (PLAYING phase)
  → [grid full] → push PracticeScoreScreen
  → [press back] → pop twice (score + game), return to ArrangementMenuScreen
```

Both screens are allocated as static objects in `main.cpp`, matching the pattern used by `solverMenu` and `practiceMenu`. Each `practiceItems[i].action` callback (currently a no-op) is updated to call `practiceGame.setArrangement(i, practiceItems[i].arrangement)` and push the game screen.

## `PracticeGameScreen`

### Layout (landscape 280×240)

- **Title bar** — 30px, label "Practice"
- **Left panel** — 144×144px live 6×6 grid (24px cells, matching solver screen style)
  - Target cell: yellow outline
  - Incorrectly occupied cells: red highlight
  - Confirmed blocker cells: standard blocker circle (light grey)
- **Right panel** — 120px wide, below title bar
  - `PLACING` phase: "Place blocker" / large coordinate (e.g. "A4") / progress ("3 / 7") / error message if invalid ("Wrong blocker! Remove it.")
  - `PLAYING` phase: "Solve it!" label / large running timer ("01:23")

### Coordinate Display

`Coord{x, y}` → column letter `'A' + x`, row number `y + 1`. Example: `{0, 3}` → "A4".

### Phase Logic

**`PLACING`** — scans grid every 200ms:
1. Target cell `grid[y][x]` must be `true`
2. No cells outside the already-confirmed set plus the target may be `true`
- Both conditions met → advance to next blocker (enter `PLAYING` if all 7 done)
- Wrong cell occupied → show error text + red highlight on that cell

Blockers are presented in vector order.

**`PLAYING`** — scans grid every 200ms. Timer counts up from 0:00 via `millis()`. When all 36 cells are `true` → stop timer, push `PracticeScoreScreen`.

### `setArrangement(index, blockers)`

Stores the arrangement index and blocker coordinate list, resets all phase state (phase = `PLACING`, confirmed count = 0, timer = 0, error cleared). Safe to call before pushing the screen.

## `PracticeScoreScreen`

### Layout

- **Title bar** — "Results"
- **Body** — centred vertically:
  - "Your time: MM:SS"
  - "Best time: MM:SS" (or "--:--" if `practiceItems[i].seconds == 0.0f`)
  - "New best!" in green if player's time beats the stored best
- **Back button** — pops twice to return to `ArrangementMenuScreen`

### Scoring (RAM only — no persistence)

Best times live in `practiceItems[i].seconds` (initialised to `0.0f` in `setup()`). If the player's time is better (lower) than the current best, `practiceItems[i].seconds` is updated in RAM so the arrangement menu shows the new best on resume. EEPROM/Preferences persistence is deferred to a future task.

## Wiring in `main.cpp`

```cpp
static PracticeScoreScreen practiceScore(tft, screenManager);
static PracticeGameScreen  practiceGame(tft, screenManager, practiceScore);
```

Each item's action:
```cpp
practiceItems[i].action = [](ScreenManager& mgr) {
    practiceGame.setArrangement(i, practiceItems[i].arrangement);
    mgr.push(&practiceGame);
};
```

## Out of Scope (deferred)

- EEPROM / Preferences persistence of best times
- Multiplayer game mode (shares the same game flow but with extra broadcast logic)
- Random arrangement option in the arrangement menu carousel
