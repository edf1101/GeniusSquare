# Menu System Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a modular screen-stack navigation system with a carousel main menu, driven by a rotary encoder and button on an ESP32-S3 with a 280×240 ST7789 TFT.

**Architecture:** A `ScreenManager` owns a fixed-size stack of `Screen*` objects and forwards input events and update/render ticks to the top screen. `CarouselMenuScreen` implements `Screen` and renders a horizontally animated carousel of `MenuItem` tiles — large centered selected tile, smaller dimmed side tiles — using delta-time lerp for smooth animation. Static screen regions (title bar, label bar) are drawn once on entry; only the tile strip is redrawn each frame.

**Tech Stack:** C++17, PlatformIO, Arduino framework, Adafruit GFX + Adafruit ST7789 library, ESP32-S3 (N16R8).

---

## File Map

| File | Action | Responsibility |
|---|---|---|
| `src/input/ButtonWrapper.h` | Create | Debounced button class declaration |
| `src/input/ButtonWrapper.cpp` | Create | Debounced button implementation |
| `src/menu/Screen.h` | Create | Pure abstract `Screen` interface |
| `src/menu/MenuItem.h` | Create | Plain `MenuItem` data struct |
| `src/menu/ScreenManager.h` | Create | `ScreenManager` class declaration |
| `src/menu/ScreenManager.cpp` | Create | `ScreenManager` stack implementation |
| `src/menu/CarouselMenuScreen.h` | Create | `CarouselMenuScreen` class declaration |
| `src/menu/CarouselMenuScreen.cpp` | Create | Full carousel rendering and animation |
| `src/assets/icons/` | Create dir | Future bitmap arrays — empty for now |
| `src/main.cpp` | Modify | Replace grid-scan prototype with menu system |

---

## Task 1: ButtonWrapper

**Files:**
- Create: `src/input/ButtonWrapper.h`
- Create: `src/input/ButtonWrapper.cpp`

- [ ] **Step 1: Create `src/input/ButtonWrapper.h`**

```cpp
/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Provides a debounced button input with a callback interface,
 * mirroring the RotaryWrapper pattern.
 */

#ifndef BUTTON_WRAPPER_H
#define BUTTON_WRAPPER_H

#include <Arduino.h>
#include <functional>

class ButtonWrapper {
public:
    /**
     * @brief Construct a ButtonWrapper.
     * @param pin     GPIO pin the button is connected to (active LOW).
     * @param onPress Callback fired on a debounced falling edge.
     */
    ButtonWrapper(uint8_t pin, std::function<void()> onPress);

    /**
     * @brief Poll the button state. Call once per loop iteration.
     */
    void poll();

private:
    uint8_t _pin;
    bool _lastState;
    unsigned long _lastPressMs;
    std::function<void()> _onPress;

    static constexpr unsigned long DEBOUNCE_MS = 50;
};

#endif // BUTTON_WRAPPER_H
```

- [ ] **Step 2: Create `src/input/ButtonWrapper.cpp`**

```cpp
/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Provides a debounced button input with a callback interface,
 * mirroring the RotaryWrapper pattern.
 */

#include "input/ButtonWrapper.h"

/**
 * @brief Construct a ButtonWrapper and configure the pin.
 */
ButtonWrapper::ButtonWrapper(uint8_t pin, std::function<void()> onPress)
    : _pin(pin), _lastState(HIGH), _lastPressMs(0), _onPress(onPress) {
    pinMode(_pin, INPUT);
}

/**
 * @brief Poll the button. Fires onPress on a debounced falling edge (HIGH → LOW).
 */
void ButtonWrapper::poll() {
    bool currentState = digitalRead(_pin);
    unsigned long now = millis();

    if (_lastState == HIGH && currentState == LOW
            && (now - _lastPressMs) > DEBOUNCE_MS) {
        _lastPressMs = now;
        if (_onPress) _onPress();
    }

    _lastState = currentState;
}
```

- [ ] **Step 3: Verify build compiles**

```bash
pio run
```

Expected: Build succeeds with no errors.

---

## Task 2: Screen interface and MenuItem struct

**Files:**
- Create: `src/menu/Screen.h`
- Create: `src/menu/MenuItem.h`

- [ ] **Step 1: Create `src/menu/Screen.h`**

```cpp
/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Abstract interface that every application screen must implement.
 * All screens (menus, game, score, solver) share this contract.
 */

#ifndef SCREEN_H
#define SCREEN_H

class Screen {
public:
    /**
     * @brief Called when this screen is pushed onto the ScreenManager stack.
     *        Draw all static UI regions and initialise state here.
     */
    virtual void onEnter() = 0;

    /**
     * @brief Called when this screen is popped from the stack.
     *        Release any resources held by the screen.
     */
    virtual void onExit() = 0;

    /**
     * @brief Called when a child screen has been popped and this screen
     *        is once again on top. Redraw static regions without full reinitialisation.
     */
    virtual void onResume() = 0;

    /**
     * @brief Animation and state update tick. Called every loop iteration.
     */
    virtual void update() = 0;

    /**
     * @brief Draw the current frame to the TFT. Called every loop iteration.
     */
    virtual void render() = 0;

    /**
     * @brief Called when the rotary encoder value changes.
     * @param delta Signed change in encoder steps.
     */
    virtual void onEncoderChange(int delta) = 0;

    /**
     * @brief Called when the button is pressed.
     */
    virtual void onButtonPress() = 0;

    virtual ~Screen() = default;
};

#endif // SCREEN_H
```

- [ ] **Step 2: Create `src/menu/MenuItem.h`**

```cpp
/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Plain data struct representing a single navigable menu item.
 * No logic — consumed by CarouselMenuScreen and future menu screens.
 */

#ifndef MENU_ITEM_H
#define MENU_ITEM_H

#include <Arduino.h>

class ScreenManager; // forward declaration — avoids circular include

/**
 * @brief A single entry in a menu screen.
 */
struct MenuItem {
    const uint8_t* bitmap;           ///< PROGMEM 1-bit bitmap; nullptr = use placeholderChar
    uint8_t        bitmapW;          ///< Bitmap width in pixels
    uint8_t        bitmapH;          ///< Bitmap height in pixels
    char           placeholderChar;  ///< Displayed centred in tile when bitmap is nullptr
    const char*    label;            ///< Text label shown in the label bar when selected
    void (*action)(ScreenManager&);  ///< Called when this item is confirmed with the button
};

#endif // MENU_ITEM_H
```

- [ ] **Step 3: Verify build compiles**

```bash
pio run
```

Expected: Build succeeds with no errors.

---

## Task 3: ScreenManager

**Files:**
- Create: `src/menu/ScreenManager.h`
- Create: `src/menu/ScreenManager.cpp`

- [ ] **Step 1: Create `src/menu/ScreenManager.h`**

```cpp
/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Manages the application screen stack. Owns navigation (push/pop)
 * and forwards encoder and button events to the active screen.
 */

#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "menu/Screen.h"

class ScreenManager {
public:
    ScreenManager();

    /**
     * @brief Push a screen onto the stack and call its onEnter().
     * @param screen Pointer to a Screen instance. Must remain valid for the screen's lifetime.
     */
    void push(Screen* screen);

    /**
     * @brief Pop the top screen (calls onExit()), then calls onResume() on the screen below.
     *        No-op if the stack is empty.
     */
    void pop();

    /**
     * @brief Forward a signed encoder delta to the active screen.
     */
    void onEncoderChange(int delta);

    /**
     * @brief Forward a button press to the active screen.
     */
    void onButtonPress();

    /**
     * @brief Call update() on the active screen.
     */
    void update();

    /**
     * @brief Call render() on the active screen.
     */
    void render();

    /**
     * @brief Returns true if at least one screen is on the stack.
     */
    bool hasScreen() const;

private:
    static constexpr uint8_t MAX_DEPTH = 8;
    Screen* _stack[MAX_DEPTH];
    uint8_t _depth;
};

#endif // SCREEN_MANAGER_H
```

- [ ] **Step 2: Create `src/menu/ScreenManager.cpp`**

```cpp
/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Manages the application screen stack.
 */

#include "menu/ScreenManager.h"

ScreenManager::ScreenManager() : _depth(0) {
    for (uint8_t i = 0; i < MAX_DEPTH; i++) _stack[i] = nullptr;
}

/**
 * @brief Push a screen onto the stack and call its onEnter().
 */
void ScreenManager::push(Screen* screen) {
    if (_depth >= MAX_DEPTH) return;
    _stack[_depth++] = screen;
    screen->onEnter();
}

/**
 * @brief Pop the top screen and resume the one below.
 */
void ScreenManager::pop() {
    if (_depth == 0) return;
    _stack[--_depth]->onExit();
    _stack[_depth] = nullptr;
    if (_depth > 0) _stack[_depth - 1]->onResume();
}

void ScreenManager::onEncoderChange(int delta) {
    if (_depth > 0) _stack[_depth - 1]->onEncoderChange(delta);
}

void ScreenManager::onButtonPress() {
    if (_depth > 0) _stack[_depth - 1]->onButtonPress();
}

void ScreenManager::update() {
    if (_depth > 0) _stack[_depth - 1]->update();
}

void ScreenManager::render() {
    if (_depth > 0) _stack[_depth - 1]->render();
}

bool ScreenManager::hasScreen() const {
    return _depth > 0;
}
```

- [ ] **Step 3: Verify build compiles**

```bash
pio run
```

Expected: Build succeeds with no errors.

---

## Task 4: CarouselMenuScreen — header and stub

**Files:**
- Create: `src/menu/CarouselMenuScreen.h`
- Create: `src/menu/CarouselMenuScreen.cpp` (stubbed)

- [ ] **Step 1: Create `src/menu/CarouselMenuScreen.h`**

```cpp
/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * A Screen that displays menu items as a horizontally scrolling carousel.
 * The selected item is shown as a large central tile; adjacent items appear
 * as smaller dimmed tiles to each side. Animated with a delta-time lerp.
 */

#ifndef CAROUSEL_MENU_SCREEN_H
#define CAROUSEL_MENU_SCREEN_H

#include <Adafruit_ST7789.h>
#include "menu/Screen.h"
#include "menu/MenuItem.h"
#include "menu/ScreenManager.h"

class CarouselMenuScreen : public Screen {
public:
    /**
     * @brief Construct a CarouselMenuScreen.
     * @param tft     Reference to the shared TFT display.
     * @param manager Reference to the ScreenManager (passed to item actions).
     * @param items   Pointer to an array of MenuItem structs. Array must outlive this object.
     * @param count   Number of items in the array.
     * @param title   Text displayed in the title bar.
     * @param wrap    Whether scrolling wraps at the ends (default true).
     */
    CarouselMenuScreen(Adafruit_ST7789& tft, ScreenManager& manager,
                       const MenuItem* items, uint8_t count,
                       const char* title, bool wrap = true);

    void onEnter() override;
    void onExit() override;
    void onResume() override;
    void update() override;
    void render() override;
    void onEncoderChange(int delta) override;
    void onButtonPress() override;

private:
    Adafruit_ST7789& _tft;
    ScreenManager&   _manager;
    const MenuItem*  _items;
    uint8_t          _count;
    const char*      _title;
    bool             _wrap;

    float         _animOffset;   ///< Current visual position (0.0 = item 0 centred)
    float         _targetOffset; ///< Destination position (always an integer)
    unsigned long _lastMs;       ///< Timestamp of last update(), for delta-time

    int _selectedIndex;          ///< Integer index of the currently selected item

    // ---- Layout (all in pixels, landscape 280×240) ----
    static constexpr int SCREEN_W     = 280;
    static constexpr int SCREEN_H     = 240;
    static constexpr int TITLE_H      = 30;
    static constexpr int LABEL_H      = 50;
    static constexpr int TILE_AREA_Y  = TITLE_H;
    static constexpr int TILE_AREA_H  = SCREEN_H - TITLE_H - LABEL_H;
    static constexpr int TILE_AREA_CY = TILE_AREA_Y + TILE_AREA_H / 2;
    static constexpr int CENTER_X     = SCREEN_W / 2;
    static constexpr int CENTER_SIZE  = 90;  ///< Tile side length when centred
    static constexpr int SIDE_SIZE    = 58;  ///< Tile side length when at ±1
    static constexpr int TILE_STRIDE  = 110; ///< Horizontal distance between tile centres

    // ---- Colour palette (RGB565) ----
    static constexpr uint16_t COL_BG         = 0x0000; ///< Background — black
    static constexpr uint16_t COL_ACCENT     = 0x07FF; ///< Selected tile — cyan
    static constexpr uint16_t COL_MUTED      = 0x2965; ///< Unselected tile — dark blue-grey
    static constexpr uint16_t COL_DASH       = 0xFFFF; ///< Dashed selection border — white
    static constexpr uint16_t COL_LABEL      = 0xFFFF; ///< Label bar text — white
    static constexpr uint16_t COL_TITLE      = 0xAD75; ///< Title bar text — light grey
    static constexpr uint16_t COL_ICON_SEL   = 0x0000; ///< Icon on selected tile — black
    static constexpr uint16_t COL_ICON_UNSEL = 0xAD75; ///< Icon on unselected tile — light grey

    void drawTitleBar();
    void drawLabelBar();
    void drawTileArea();

    /**
     * @brief Draw a single tile centred at (cx, cy).
     * @param cx       Horizontal centre of the tile.
     * @param cy       Vertical centre of the tile.
     * @param size     Tile side length in pixels.
     * @param selected Whether this is the selected (centre) tile.
     * @param item     The MenuItem to render.
     */
    void drawTile(int cx, int cy, int size, bool selected, const MenuItem& item);

    /**
     * @brief Draw a dashed rectangular border at the given position.
     * @param x      Top-left x.
     * @param y      Top-left y.
     * @param w      Width.
     * @param h      Height.
     * @param colour Border pixel colour.
     */
    void drawDashedBorder(int x, int y, int w, int h, uint16_t colour);
};

#endif // CAROUSEL_MENU_SCREEN_H
```

- [ ] **Step 2: Create stubbed `src/menu/CarouselMenuScreen.cpp`**

```cpp
/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Carousel menu screen — animated horizontal tile selector.
 */

#include "menu/CarouselMenuScreen.h"

CarouselMenuScreen::CarouselMenuScreen(Adafruit_ST7789& tft, ScreenManager& manager,
                                       const MenuItem* items, uint8_t count,
                                       const char* title, bool wrap)
    : _tft(tft), _manager(manager), _items(items), _count(count),
      _title(title), _wrap(wrap),
      _animOffset(0.0f), _targetOffset(0.0f), _lastMs(0),
      _selectedIndex(0) {}

void CarouselMenuScreen::onEnter()           {}
void CarouselMenuScreen::onExit()            {}
void CarouselMenuScreen::onResume()          {}
void CarouselMenuScreen::update()            {}
void CarouselMenuScreen::render()            {}
void CarouselMenuScreen::onEncoderChange(int) {}
void CarouselMenuScreen::onButtonPress()     {}

void CarouselMenuScreen::drawTitleBar()                                    {}
void CarouselMenuScreen::drawLabelBar()                                    {}
void CarouselMenuScreen::drawTileArea()                                    {}
void CarouselMenuScreen::drawTile(int,int,int,bool,const MenuItem&)        {}
void CarouselMenuScreen::drawDashedBorder(int,int,int,int,uint16_t)        {}
```

- [ ] **Step 3: Verify build compiles**

```bash
pio run
```

Expected: Build succeeds with no errors or warnings about missing overrides.

---

## Task 5: CarouselMenuScreen — static rendering

Fill in the drawing methods so the carousel renders a still frame (no animation yet).

**Files:**
- Modify: `src/menu/CarouselMenuScreen.cpp`

- [ ] **Step 1: Implement `drawTitleBar()`**

Replace the stub `void CarouselMenuScreen::drawTitleBar() {}` with:

```cpp
void CarouselMenuScreen::drawTitleBar() {
    _tft.fillRect(0, 0, SCREEN_W, TITLE_H, COL_BG);
    _tft.setTextSize(2);
    _tft.setTextColor(COL_TITLE);
    // Each char at size 2 is 12px wide, 16px tall
    int textW = strlen(_title) * 12;
    _tft.setCursor((SCREEN_W - textW) / 2, (TITLE_H - 16) / 2);
    _tft.print(_title);
}
```

- [ ] **Step 2: Implement `drawLabelBar()`**

Replace the stub `void CarouselMenuScreen::drawLabelBar() {}` with:

```cpp
void CarouselMenuScreen::drawLabelBar() {
    _tft.fillRect(0, SCREEN_H - LABEL_H, SCREEN_W, LABEL_H, COL_BG);
    if (_count == 0) return;
    const char* label = _items[_selectedIndex].label;
    _tft.setTextSize(2);
    _tft.setTextColor(COL_LABEL);
    int textW = strlen(label) * 12;
    _tft.setCursor((SCREEN_W - textW) / 2, SCREEN_H - LABEL_H + (LABEL_H - 16) / 2);
    _tft.print(label);
}
```

- [ ] **Step 3: Implement `drawDashedBorder()`**

Replace the stub with:

```cpp
void CarouselMenuScreen::drawDashedBorder(int x, int y, int w, int h, uint16_t colour) {
    // Top and bottom edges — draw every other pixel at step 4
    for (int px = x; px < x + w; px += 4) {
        _tft.drawPixel(px, y,         colour);
        _tft.drawPixel(px, y + h - 1, colour);
    }
    // Left and right edges
    for (int py = y; py < y + h; py += 4) {
        _tft.drawPixel(x,         py, colour);
        _tft.drawPixel(x + w - 1, py, colour);
    }
}
```

- [ ] **Step 4: Implement `drawTile()`**

Replace the stub with:

```cpp
void CarouselMenuScreen::drawTile(int cx, int cy, int size, bool selected, const MenuItem& item) {
    int x = cx - size / 2;
    int y = cy - size / 2;
    uint16_t tileColour = selected ? COL_ACCENT     : COL_MUTED;
    uint16_t iconColour = selected ? COL_ICON_SEL   : COL_ICON_UNSEL;

    _tft.fillRoundRect(x, y, size, size, 8, tileColour);

    if (selected) {
        drawDashedBorder(x - 3, y - 3, size + 6, size + 6, COL_DASH);
    }

    if (item.bitmap != nullptr) {
        int bx = cx - item.bitmapW / 2;
        int by = cy - item.bitmapH / 2;
        _tft.drawBitmap(bx, by, item.bitmap, item.bitmapW, item.bitmapH, iconColour);
    } else {
        // Placeholder char — at textSize 3, each char is 18×24 px
        _tft.setTextSize(3);
        _tft.setTextColor(iconColour);
        _tft.setCursor(cx - 9, cy - 12);
        _tft.print(item.placeholderChar);
    }
}
```

- [ ] **Step 5: Implement `drawTileArea()`**

Replace the stub with:

```cpp
void CarouselMenuScreen::drawTileArea() {
    _tft.fillRect(0, TILE_AREA_Y, SCREEN_W, TILE_AREA_H, COL_BG);

    for (int i = 0; i < _count; i++) {
        float dist = (float)i - _animOffset;

        // Wrap distance to the range [-count/2, count/2] for circular navigation
        if (_wrap && _count > 1) {
            float half = _count / 2.0f;
            while (dist >  half) dist -= _count;
            while (dist < -half) dist += _count;
        }

        if (abs(dist) > 1.5f) continue;

        // Tile size lerps between CENTER_SIZE (dist=0) and SIDE_SIZE (dist=±1)
        float t    = min(abs(dist), 1.0f);
        int   size = (int)(CENTER_SIZE + (SIDE_SIZE - CENTER_SIZE) * t);
        int   tileX = CENTER_X + (int)(dist * TILE_STRIDE);

        drawTile(tileX, TILE_AREA_CY, size, (i == _selectedIndex), _items[i]);
    }
}
```

- [ ] **Step 6: Implement `onEnter()` and `onResume()`**

Replace the stubs with:

```cpp
void CarouselMenuScreen::onEnter() {
    _animOffset   = (float)_selectedIndex;
    _targetOffset = (float)_selectedIndex;
    _lastMs       = millis();
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawLabelBar();
}

void CarouselMenuScreen::onResume() {
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawLabelBar();
}
```

- [ ] **Step 7: Implement `render()`**

Replace the stub with:

```cpp
void CarouselMenuScreen::render() {
    drawTileArea();
}
```

- [ ] **Step 8: Verify build compiles**

```bash
pio run
```

Expected: Build succeeds with no errors.

---

## Task 6: CarouselMenuScreen — animation and input

**Files:**
- Modify: `src/menu/CarouselMenuScreen.cpp`

- [ ] **Step 1: Implement `update()` with delta-time lerp**

Replace the stub with:

```cpp
void CarouselMenuScreen::update() {
    unsigned long now = millis();
    float dt = (now - _lastMs) / 1000.0f;
    _lastMs = now;

    float gap = _targetOffset - _animOffset;

    // For wrap mode, take the shortest path around the circle
    if (_wrap && _count > 1) {
        float half = _count / 2.0f;
        if (gap >  half) gap -= _count;
        if (gap < -half) gap += _count;
    }

    _animOffset += gap * min(dt * 12.0f, 1.0f);

    // Snap once close enough to avoid floating-point drift
    if (abs(gap) < 0.01f) {
        _animOffset = _targetOffset;
    }

    // Keep _animOffset in [0, count) when wrapping
    if (_wrap && _count > 1) {
        if (_animOffset >= _count) _animOffset -= _count;
        if (_animOffset <  0)      _animOffset += _count;
    }
}
```

- [ ] **Step 2: Implement `onEncoderChange()`**

Replace the stub with:

```cpp
void CarouselMenuScreen::onEncoderChange(int delta) {
    if (_count == 0) return;

    if (_wrap) {
        _selectedIndex = ((_selectedIndex + delta) % _count + _count) % _count;
    } else {
        _selectedIndex = max(0, min((int)_count - 1, _selectedIndex + delta));
    }

    _targetOffset = (float)_selectedIndex;
    drawLabelBar(); // update label immediately — no need to wait for render()
}
```

- [ ] **Step 3: Implement `onButtonPress()`**

Replace the stub with:

```cpp
void CarouselMenuScreen::onButtonPress() {
    if (_count == 0) return;
    if (_items[_selectedIndex].action != nullptr) {
        _items[_selectedIndex].action(_manager);
    }
}
```

- [ ] **Step 4: Implement `onExit()` (no-op for now)**

```cpp
void CarouselMenuScreen::onExit() {
    // Nothing to release — TFT is shared and owned by main.cpp
}
```

- [ ] **Step 5: Verify build compiles**

```bash
pio run
```

Expected: Build succeeds with no errors.

---

## Task 7: Wire main.cpp and verify on hardware

Replace the grid-scan prototype in `src/main.cpp` with the menu system. Upload and manually verify the carousel.

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Replace `src/main.cpp` entirely**

```cpp
/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Application entry point. Initialises hardware and launches the main menu.
 */

#include <Arduino.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#include "RotaryWrapper.h"
#include "input/ButtonWrapper.h"
#include "menu/ScreenManager.h"
#include "menu/MenuItem.h"
#include "menu/CarouselMenuScreen.h"

// ---- Hardware pins ----
#define TFT_MOSI  5
#define TFT_SCLK  4
#define TFT_CS   11
#define TFT_DC   12
#define TFT_RST  13
#define ROT_A    17
#define ROT_B    18
#define BTN_PIN  16

// ---- Hardware objects ----
SPIClass* hspi = new SPIClass(FSPI);
Adafruit_ST7789 tft = Adafruit_ST7789(hspi, TFT_CS, TFT_DC, TFT_RST);
RotaryWrapper rot(ROT_A, ROT_B);

// ---- Application ----
ScreenManager screenManager;

// Forward declarations for menu item actions
void onSolverSelected(ScreenManager& mgr);
void onPracticeSelected(ScreenManager& mgr);
void onMultiplayerSelected(ScreenManager& mgr);

MenuItem mainMenuItems[] = {
    { nullptr, 0, 0, 'S', "Solver",      onSolverSelected      },
    { nullptr, 0, 0, 'P', "Practice",    onPracticeSelected    },
    { nullptr, 0, 0, 'M', "Multiplayer", onMultiplayerSelected },
};

CarouselMenuScreen mainMenu(tft, screenManager, mainMenuItems, 3, "Genius Square");

ButtonWrapper button(BTN_PIN, []() { screenManager.onButtonPress(); });

void onSolverSelected(ScreenManager&)      { /* push SolverScreen when implemented */ }
void onPracticeSelected(ScreenManager&)    { /* push PracticeScreen when implemented */ }
void onMultiplayerSelected(ScreenManager&) { /* push MultiplayerScreen when implemented */ }

void setup() {
    Serial.begin(115200);

    hspi->begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
    tft.init(240, 280);
    tft.setRotation(1); // landscape: 280×240
    tft.fillScreen(0x0000);

    analogSetAttenuation(ADC_11db);

    rot.setCallbackFunc([](long delta) {
        screenManager.onEncoderChange((int)delta);
    });

    screenManager.push(&mainMenu);
}

void loop() {
    rot.poll();
    button.poll();
    screenManager.update();
    screenManager.render();
}
```

- [ ] **Step 2: Build**

```bash
pio run
```

Expected: Build succeeds with no errors.

- [ ] **Step 3: Upload and open monitor**

```bash
pio run --target upload && pio device monitor
```

- [ ] **Step 4: Verify on device**

Check the following manually:

| Behaviour | Expected |
|---|---|
| Screen on boot | Black background, "Genius Square" title, 3 tiles with S / P / M |
| Centre tile | Cyan, larger (~90px), dashed white border |
| Side tiles | Dark grey, smaller (~58px), no border |
| Label bar | Shows "Solver" (first item) on boot |
| Rotate encoder clockwise | Carousel slides left, "Practice" becomes centre, label updates |
| Rotate encoder anti-clockwise | Carousel slides right, wraps from "Solver" to "Multiplayer" |
| Animation | Smooth slide — no flickering, no tearing, responsive to encoder speed |
| Button press | No crash (action stubs are no-ops) |

- [ ] **Step 5: Commit**

```bash
git add src/input/ButtonWrapper.h src/input/ButtonWrapper.cpp \
        src/menu/Screen.h src/menu/MenuItem.h \
        src/menu/ScreenManager.h src/menu/ScreenManager.cpp \
        src/menu/CarouselMenuScreen.h src/menu/CarouselMenuScreen.cpp \
        src/main.cpp \
        docs/superpowers/specs/2026-04-11-menu-system-design.md \
        docs/superpowers/plans/2026-04-11-menu-system.md
git commit -m "feat: add screen stack and animated carousel main menu"
```
