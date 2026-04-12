# Menu System Design

**Date:** 2026-04-11
**Project:** GeniusSquare ESP32-S3 Firmware

---

## Overview

A modular, screen-stack-based UI system for the GeniusSquare device. Handles all navigation (main menu, sub-menus, game screens, score screens) through a unified `Screen` interface managed by a `ScreenManager` stack. The first concrete implementation is a `CarouselMenuScreen` for the main menu.

---

## Architecture

### `Screen` — abstract interface

Every screen in the application (menus, game, score, solver) implements this interface:

```cpp
class Screen {
public:
    virtual void onEnter() = 0;                    // pushed onto stack — full redraw
    virtual void onExit() = 0;                     // popped from stack — cleanup
    virtual void onResume() = 0;                   // returned to after child screen popped — redraw static regions
    virtual void update() = 0;                     // animation/state tick, every loop
    virtual void render() = 0;                     // draw to TFT, every loop
    virtual void onEncoderChange(int delta) = 0;
    virtual void onButtonPress() = 0;
    virtual ~Screen() = default;
};
```

### `ScreenManager` — stack owner

- Holds a fixed-size array of `Screen*` (no heap allocation at runtime)
- `push(Screen*)` — calls `onEnter()` on the new screen
- `pop()` — calls `onExit()` on top, calls `onResume()` on the screen below
- `update()` and `render()` delegate to the top-of-stack screen
- `onEncoderChange(int delta)` and `onButtonPress()` forward to top-of-stack screen
- Stack size is a compile-time constant (e.g. 8 — sufficient for all planned navigation depths)

Navigation example:
```
Main carousel → [push] Multiplayer sub-menu → [push] Host/Join screen → [push] Game screen → [push] Score screen → [pop ×4]
```

### `MenuItem` — plain data struct

```cpp
struct MenuItem {
    const uint8_t* bitmap;       // PROGMEM bitmap array; nullptr = use char fallback
    uint8_t        bitmapW;
    uint8_t        bitmapH;
    char           placeholderChar;  // displayed when bitmap is nullptr
    const char*    label;            // text shown below the tile
    void (*action)(ScreenManager&);  // called on button press when this item is selected
};
```

Raw function pointer used instead of `std::function` to avoid heap allocation at construction time.

---

## CarouselMenuScreen

The first concrete `Screen` implementation. Displays a horizontally scrolling carousel of `MenuItem` tiles: one large centered tile (selected), smaller dimmed tiles on each side.

### Layout (280×240 landscape display)

| Region | Y range | Behaviour |
|---|---|---|
| Title bar | 0–30px | Drawn once on `onEnter()`/`onResume()` |
| Tile area | 30–190px | Cleared and redrawn every animation frame |
| Label bar | 190–240px | Redrawn only when selected item changes |

Limiting redraws to the tile strip keeps SPI traffic low.

### Tile sizing

Adafruit GFX has no bitmap scaling, so tile size is determined by role rather than continuous interpolation:

- **Center tile** (selected): ~90×90px rounded rectangle background, full accent colour
- **Side tiles** (±1 from center): ~58×58px rounded rectangle, muted colour
- Items beyond ±1 from center: not drawn

Tile width/height lerps between `centerSize` and `sideSize` as a function of distance from center, giving a smooth grow/shrink effect on the background rect even though the icon itself is drawn at native size centered within it.

### Animation

Two floats drive all animation:

- `float _animOffset` — current visual position (0.0 = item 0 centered)
- `float _targetOffset` — destination (integer, advances by encoder delta)

Each `update()` tick:

```cpp
float dt = (millis() - _lastMs) / 1000.0f;
_lastMs = millis();
_animOffset += (_targetOffset - _animOffset) * min(dt * 12.0f, 1.0f);
if (abs(_animOffset - _targetOffset) < 0.01f) _animOffset = _targetOffset;
```

Delta-time makes animation speed frame-rate independent. The `12.0f` spring constant is tunable. No `delay()` in `loop()` while animating.

### Selection indicator

The center tile receives two additional visual treatments:
1. Accent background colour (distinct from muted side tile colour)
2. Dashed border: loop around the tile perimeter drawing every other pixel in a bright highlight colour

### Icon rendering

```cpp
if (item.bitmap != nullptr) {
    tft.drawBitmap(iconX, iconY, item.bitmap, item.bitmapW, item.bitmapH, iconColour);
} else {
    tft.drawChar(iconX, iconY, item.placeholderChar, iconColour, tileColour, 3);
}
```

Icon colour matches the tile's accent/muted colour, keeping the palette consistent.

### Wrapping

Wraps by default — scrolling past the last item loops to the first. Constructor takes `bool wrap = true` to disable for linear lists (e.g. Practice arrangement selector).

### Extensibility

`CarouselMenuScreen` takes a pointer to a `MenuItem` array and a count — no templates, no fixed size. Handles 1, 2, 3, 4+ items without modification. A future `GridMenuScreen` will implement the same `Screen` interface independently, sharing only `MenuItem` and `ScreenManager`.

---

## Input Handling

### `ButtonWrapper` — new class

Mirrors the existing `RotaryWrapper` pattern:

```cpp
class ButtonWrapper {
public:
    ButtonWrapper(uint8_t pin, std::function<void()> onPress);
    void poll();
private:
    uint8_t _pin;
    bool _lastState;
    unsigned long _lastPressMs;
    std::function<void()> _onPress;
};
```

- Time-based debounce: 50ms
- Fires `onPress` on falling edge (pin goes LOW)
- Pin 16 (existing hardware)

### main loop

```cpp
void loop() {
    rot.poll();       // fires encoder callback → ScreenManager::onEncoderChange
    button.poll();    // fires button callback  → ScreenManager::onButtonPress
    screenManager.update();
    screenManager.render();
}
```

No `delay()`. Animation pacing handled by delta-time lerp inside each screen's `update()`.

---

## File Structure

```
src/
  main.cpp                        # setup(), loop() — hardware wiring only

  input/
    ButtonWrapper.h/.cpp          # Debounced button (usable by any screen, not menu-specific)

  RotaryWrapper.h/.cpp            # Existing — unchanged

  menu/
    Screen.h                      # Pure interface — header only, no .cpp
    ScreenManager.h/.cpp          # Stack, input forwarding, update/render delegation
    MenuItem.h                    # Plain data struct — header only, no .cpp
    CarouselMenuScreen.h/.cpp     # Carousel implementation

  assets/
    icons/                        # Bitmap arrays as .h files (e.g. icon_solver.h)
                                  # Empty for now; placeholder chars used until bitmaps added
```

---

## Out of Scope (this spec)

- `GridMenuScreen` — future, not designed or implemented here
- Actual bitmap assets — placeholder chars used throughout initial implementation
- Multiplayer sub-menu, Practice carousel, Game screen, Score screen — future screens; they implement `Screen` but are not specified here