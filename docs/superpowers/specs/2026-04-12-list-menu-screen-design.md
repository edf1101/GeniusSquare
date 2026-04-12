# ListMenuScreen Design

**Date:** 2026-04-12
**Project:** GeniusSquare ESP32-S3 Firmware

---

## Overview

A scrolling vertical list screen for navigating large option sets (50+ items). Complements `CarouselMenuScreen` for use cases like Practice arrangement selection and Multiplayer lobby joining, where a carousel is impractical. The first item is always an auto-injected "Back" entry that calls `manager.pop()`.

---

## Architecture

### `ListItem` — plain data struct (`src/menu/ListItem.h`)

```cpp
struct ListItem {
    const char*           label;
    void (*action)(ScreenManager&);
};
```

No bitmaps or placeholder chars. Mirrors the simplicity of `MenuItem` without icon fields. Raw function pointer to avoid heap allocation.

### `ListMenuScreen` — implements `Screen` (`src/menu/ListMenuScreen.h/.cpp`)

Constructor:

```cpp
ListMenuScreen(TFT_eSPI& tft, ScreenManager& manager,
               const ListItem* items, uint8_t count,
               const char* title);
```

`items` contains only the real options (no Back entry). The screen synthesises "Back" → `manager.pop()` internally at index 0. `count` is the number of real items; total navigable items = `count + 1`.

---

## Layout (280×240 landscape)

| Region   | Y range  | Behaviour                                      |
|----------|----------|------------------------------------------------|
| Title bar | 0–29px  | Drawn once on `onEnter()` / `onResume()`       |
| Row area  | 30–241px | 4 rows × 52px, redrawn fully on every snap     |

### Row anatomy (52px tall)

```
|  num box  |  label text                          |
|  30px     |  250px                               |
```

- **Number box**: 24×24px, centered vertically in the row. Shows the 1-based on-screen position (always 1–4, never the absolute index). White outlined box when the row is selected; number only (no box) when unselected.
- **Label text**: left-aligned, vertically centered within the row.
- **Row colours**: black background (`0x0000`), grey text (`0xAD75`) unselected, white text (`0xFFFF`) selected.
- **Dividers**: 1px faint horizontal line (`0x2965`) between rows — 3 lines total (after on-screen rows 1, 2, 3). Not drawn after row 4.

### Configurable constant

```cpp
static constexpr uint8_t VISIBLE_ROWS = 4;  // adjustable in ListMenuScreen.h
```

---

## Scroll & Selection Logic

Two state variables:

- `_selectedIndex` — absolute index of the highlighted item (0 = Back, 1..count = real items)
- `_viewStart` — absolute index of the first visible row

**On encoder change (delta = ±1):**

1. Clamp `_selectedIndex` to `[0, count]` — no wrap-around.
2. Bring viewport to follow:
   - If `_selectedIndex < _viewStart`: `_viewStart = _selectedIndex`
   - If `_selectedIndex >= _viewStart + VISIBLE_ROWS`: `_viewStart = _selectedIndex - VISIBLE_ROWS + 1`
3. Snap redraw the row area.

**Highlight-then-scroll behaviour:**
- While the cursor is within the current viewport the viewport stays fixed and the highlight moves down the visible rows.
- Only when the cursor reaches the bottom edge of the viewport does the viewport scroll by one row.

**No animation** — instant snap on every input event.

**On button press:** call `item.action(manager)` for the selected item (or `manager.pop()` for Back).

---

## Edge Cases

| Scenario | Behaviour |
|---|---|
| Only Back (count = 0) | Row 1 shows "Back" selected, rows 2–4 black. Dividers still drawn. |
| Fewer items than VISIBLE_ROWS | Empty rows below last item are black. Dividers still drawn up to `VISIBLE_ROWS - 1`. |
| Scrolled to end of list | `_selectedIndex` clamps at `count`; viewport does not scroll past last item. |

---

## File Structure

```
src/menu/
  ListItem.h            # Plain data struct — header only, no .cpp
  ListMenuScreen.h/.cpp # Scrolling list implementation
```

No changes to `Screen.h`, `ScreenManager`, `MenuItem`, or `CarouselMenuScreen`.

---

## Out of Scope (this spec)

- Smooth scroll animation — snap only; animation deferred to a future iteration.
- Custom "Back" label — always "Back", always calls `manager.pop()`.
- Multi-column layouts.
- Icons or bitmaps in list rows.
