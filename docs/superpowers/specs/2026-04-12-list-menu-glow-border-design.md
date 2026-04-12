# ListMenuScreen Glow Border & Layout Design

**Date:** 2026-04-12
**Project:** GeniusSquare ESP32-S3 Firmware

---

## Overview

Enhance `ListMenuScreen` with three visual improvements:
1. Larger number text and wider left column — numbers and labels shift right
2. Animated glowing border around the selected row's number box, matching the carousel's pulsing outline
3. Smooth fade-in on each new selection (resets to zero on selection change, fades to full brightness)

All changes are confined to `src/menu/ListMenuScreen.h` and `src/menu/ListMenuScreen.cpp`. No other files are modified.

---

## Layout Changes

Three constexpr values change in `ListMenuScreen.h`:

| Constant | Old | New | Effect |
|---|---|---|---|
| `NUM_TEXT_SIZE` | 2 | 3 | Font scale: 12×16px → 18×24px per character |
| `NUM_BOX_SIZE` | 24 | 40 | Sized for 2-digit size-3 text (36px) + 2px padding each side |
| `NUM_COL_W` | 30 | 50 | Left column wide enough for 40px box + 5px margin each side |

Label `x` changes from `NUM_COL_W + 4 = 34` to `NUM_COL_W + 8 = 58` — both number column and label shift right.

Number text centering uses `NUM_TEXT_SIZE * 6` (18px) as char width and `NUM_TEXT_SIZE * 8` (24px) as char height. All other layout values (ROW_H, TITLE_H, SCR_W, SCR_H, divider positions) are unchanged.

---

## Animation State

Six new private fields added to `ListMenuScreen`:

```cpp
float         _borderPhase;          ///< Sine wave phase in radians
float         _borderEnvelope;       ///< Fade-in envelope [0.0, 1.0]
float         _lastBorderThickness;  ///< -1.0 = no border currently drawn
uint16_t      _lastBorderColor;      ///< Colour from previous frame (for change detection)
unsigned long _lastMs;               ///< Timestamp of last update() call, for delta-time
bool          _dirty;                ///< True when render() should call updateNumberBoxBorder()
```

Initialised in constructor: `_borderPhase(0)`, `_borderEnvelope(0)`, `_lastBorderThickness(-1)`, `_lastBorderColor(0)`, `_lastMs(0)`, `_dirty(false)`.

Also reset in `onEnter()` and `onResume()`.

---

## Animation Constants (tunable in header)

```cpp
static constexpr float BORDER_T_MIN       = 1.0f;  ///< Minimum border thickness (px)
static constexpr float BORDER_T_MAX       = 3.0f;  ///< Maximum border thickness (px)
static constexpr float BORDER_SPEED       = 4.0f;  ///< Phase advance rate (rad/sec)
static constexpr float BORDER_INTRO_SPEED = 8.0f;  ///< Envelope fade-in rate (units/sec, ~125ms)
static constexpr uint16_t COL_BORDER      = 0xFFFF; ///< Border colour — white
```

---

## update() — Called Every Loop

```
dt = (millis() - _lastMs) / 1000.0f
_lastMs = millis()
_borderPhase += BORDER_SPEED * dt   (wrap at 2π)
_borderEnvelope = min(_borderEnvelope + BORDER_INTRO_SPEED * dt, 1.0f)
_dirty = true
```

---

## render() — Called Every Loop

```
if (!_dirty) return
_dirty = false
updateNumberBoxBorder()
```

---

## updateNumberBoxBorder()

Called from `render()` every frame. Only issues TFT commands when thickness or colour actually changed.

```
boxX = (NUM_COL_W - NUM_BOX_SIZE) / 2
boxY = TITLE_H + (_selectedIndex - _viewStart) * ROW_H + (ROW_H - NUM_BOX_SIZE) / 2

exactThickness = BORDER_T_MIN + (BORDER_T_MAX - BORDER_T_MIN) * (0.5 + 0.5 * sin(_borderPhase))
borderColour   = blendColor(COL_BG, COL_BORDER, _borderEnvelope)

// Early-exit: nothing changed
if (|exactThickness - _lastBorderThickness| < 0.01 && borderColour == _lastBorderColor) return

// Too faded: clear old border and bail
if (_borderEnvelope < 0.02) {
    if (_lastBorderThickness >= 0) drawNumberBoxBorder(boxX, boxY, _lastBorderThickness, COL_BG)
    _lastBorderThickness = -1
    _lastBorderColor = 0
    return
}

// First frame: draw without erase
if (_lastBorderThickness < 0) {
    drawNumberBoxBorder(boxX, boxY, exactThickness, borderColour)
} else {
    // Erase old, draw new (avoids ghost pixels when thickness changes)
    drawNumberBoxBorder(boxX, boxY, _lastBorderThickness, COL_BG)
    drawNumberBoxBorder(boxX, boxY, exactThickness, borderColour)
}

_lastBorderThickness = exactThickness
_lastBorderColor = borderColour
```

---

## drawNumberBoxBorder()

Draws `round(thickness)` concentric `drawRect` outlines outside the number box:

```cpp
void ListMenuScreen::drawNumberBoxBorder(int boxX, int boxY, float thickness, uint16_t colour) {
    int t = (int)roundf(thickness);
    for (int i = 0; i < t; i++) {
        _tft.drawRect(boxX - 1 - i, boxY - 1 - i,
                      NUM_BOX_SIZE + 2 + 2*i,
                      NUM_BOX_SIZE + 2 + 2*i,
                      colour);
    }
}
```

No rounded corners — the number box is a small square and plain concentric rects produce a clean glow at this scale.

---

## blendColor() — Private Helper

RGB565 linear interpolation, copied from `CarouselMenuScreen`:

```cpp
uint16_t ListMenuScreen::blendColor(uint16_t bg, uint16_t fg, float alpha) {
    uint8_t r = ((bg >> 11) & 0x1F) + (int)(((int)((fg >> 11) & 0x1F) - (int)((bg >> 11) & 0x1F)) * alpha);
    uint8_t g = ((bg >>  5) & 0x3F) + (int)(((int)((fg >>  5) & 0x3F) - (int)((bg >>  5) & 0x3F)) * alpha);
    uint8_t b = ((bg      ) & 0x1F) + (int)(((int)( fg        & 0x1F) - (int)( bg        & 0x1F)) * alpha);
    return (uint16_t)((r << 11) | (g << 5) | b);
}
```

---

## Selection Change Behaviour

On `onEncoderChange()`, after updating `_selectedIndex` and `_viewStart`:

1. `_borderEnvelope = 0.0f` — resets fade-in for new row
2. `_borderPhase = 0.0f` — consistent animation start point
3. `_lastBorderThickness = -1.0f` — forces first-frame draw
4. `drawRowArea()` — full redraw clears the old border as part of row clearing

The old border disappears instantly (swallowed by the row fill), and the new border fades in from zero on the new row.

---

## Files Changed

| File | Change |
|---|---|
| `src/menu/ListMenuScreen.h` | New fields, constants (`NUM_TEXT_SIZE`, `NUM_BOX_SIZE`, `NUM_COL_W`, border animation constants), new private method declarations |
| `src/menu/ListMenuScreen.cpp` | `update()`, `render()`, `updateNumberBoxBorder()`, `drawNumberBoxBorder()`, `blendColor()` implemented; `drawRow()` updated for new layout |

---

## Out of Scope

- Rounded corners on the number box border
- Anti-aliasing on the border edges
- Fade-out animation on the old row (cleared instantly by `drawRowArea()`)
- Changes to `CarouselMenuScreen` or any other file
