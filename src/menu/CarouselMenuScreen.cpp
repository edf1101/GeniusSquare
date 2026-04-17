/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Carousel menu screen — animated horizontal tile selector.
 */

#include "menu/CarouselMenuScreen.h"
#include <math.h>
#include <string.h>

CarouselMenuScreen::CarouselMenuScreen(TFT_eSPI& tft, ScreenManager& manager,
                                       const MenuItem* items, uint8_t count,
                                       const char* title, bool wrap)
    : _tft(tft), _manager(manager), _items(items), _count(count),
      _title(title), _wrap(wrap),
      _animOffset(0.0f), _prevAnimOffset(-1.0f), _targetOffset(0.0f),
      _lastMs(0), _selectedIndex(0), _dirty(true),
      _borderPhase(0.0f), _lastBorderThickness(-1.0f), _lastBorderColor(0),
      _borderEnvelope(0.0f), _borderPulseActive(false)
{
}

void CarouselMenuScreen::onEnter() {
    _animOffset      = (float)_selectedIndex;
    _prevAnimOffset  = -1.0f; // forces a full tile-area redraw on first render
    _targetOffset    = (float)_selectedIndex;
    _borderPhase     = 0.0f;
    _lastBorderThickness = -1.0f;
    _lastBorderColor = 0;
    _borderEnvelope    = 0.0f;
    _borderPulseActive = false;
    _lastMs          = millis();
    _dirty           = true;
    _tft.setTextWrap(false);
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawLabelBar();
}

void CarouselMenuScreen::onExit() {
    // Nothing to release — TFT is shared and owned by main.cpp
}

void CarouselMenuScreen::onResume() {
    _prevAnimOffset = -1.0f;
    _lastBorderThickness = -1.0f;
    _lastBorderColor = 0;
    _borderEnvelope    = 0.0f;
    _borderPulseActive = false;
    _dirty          = true;
    _tft.setTextWrap(false);
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawLabelBar();
}

/**
 * @brief Update animation state for one frame.
 *
 * 1. Advance border phase for sine-wave animation
 * 2. Interpolate carousel position towards _targetOffset (lerp 8 units/sec)
 *    - Handle wrapping if carousel is looping
 *    - Snap to target when within 0.01f (settles animation)
 * 3. Fade border in/out based on carousel settling:
 *    - Fade in when settled (125ms)
 *    - Fade out when moving (67ms fade-out, faster decay)
 *
 * Set _dirty = true to trigger render() on this frame.
 */
void CarouselMenuScreen::update() {
    unsigned long now = millis();
    float dt = (now - _lastMs) / 1000.0f;  // Delta time in seconds
    _lastMs = now;

    // Advance wave-border phase — always animating while this screen is active
    // This drives the sine-wave thickness oscillation of the center tile's border.
    _borderPhase += BORDER_SPEED * dt;
    if (_borderPhase >= (float)(2.0 * M_PI)) _borderPhase -= (float)(2.0 * M_PI);
    _dirty = true;

    // Advance carousel position towards _targetOffset using exponential lerp.
    // Compute shortest path considering wrapping.
    float gap = _targetOffset - _animOffset;
    if (_wrap && _count > 1) {
        float half = _count / 2.0f;
        if (gap >  half) gap -= _count;  // Wrap long jumps the short way around
        if (gap < -half) gap += _count;
    }

    if (fabsf(gap) >= 0.01f) {
        // Lerp towards target at CAROUSEL_LERP_SPEED (8.0 units/sec).
        // Use min(..., 1.0f) to clamp dt * speed so we don't overshoot in one frame.
        _animOffset += gap * min(dt * CAROUSEL_LERP_SPEED, 1.0f);

        // Rewrap position after lerp to keep in [0, _count) range.
        if (_wrap && _count > 1) {
            if (_animOffset >= _count) _animOffset -= _count;
            if (_animOffset <  0)      _animOffset += _count;
        }
    } else if (_animOffset != _targetOffset) {
        _animOffset = _targetOffset;  // Snap to exact target when very close
    }

    // Fade border in/out based on carousel settling.
    // Mirror the 0.01f threshold from the lerp to detect "settled" state.
    bool settled = fabsf(_targetOffset - _animOffset) < 0.01f;
    if (settled) {
        // Carousel is still; fade border in (125ms to full opacity).
        _borderEnvelope = min(_borderEnvelope + dt * BORDER_INTRO_SPEED, 1.0f);
    } else {
        // Carousel is moving; quickly fade border out (67ms to zero).
        _borderEnvelope = max(_borderEnvelope - dt * BORDER_OUTRO_SPEED, 0.0f);
    }
}

void CarouselMenuScreen::render() {
    if (!_dirty) return;
    _dirty = false;

    bool carouselMoved = (fabsf(_animOffset - _prevAnimOffset) > 0.0001f);
    _prevAnimOffset = _animOffset;

    if (carouselMoved) {
        // Full tile-area redraw during and immediately after the carousel slide.
        drawTileArea();

        // Force the border to redraw completely next frame to fix the black phantom outline glitch
        _lastBorderThickness = -1.0f;
        // Reset envelope and phase so the border always fades in from zero at a consistent
        // wave starting-point each time the carousel settles on a new tile.
        _borderEnvelope = 0.0f;
        _borderPhase = 0.0f;
    } else {
        // Carousel settled — update the animated border around the center tile.
        updateBorder();
    }
}

void CarouselMenuScreen::onEncoderChange(int delta) {
    if (_count == 0) return;

    if (_wrap) {
        _selectedIndex = ((_selectedIndex + delta) % _count + _count) % _count;
    } else {
        _selectedIndex = max(0, min((int)_count - 1, _selectedIndex + delta));
    }

    _targetOffset = (float)_selectedIndex;
    _dirty        = true;
    drawLabelBar();
}

void CarouselMenuScreen::onButtonPress() {
    if (_count == 0) return;
    if (_items[_selectedIndex].action != nullptr) {
        _items[_selectedIndex].action(_manager);
    }
}

// ---------------------------------------------------------------------------
// Static regions
// ---------------------------------------------------------------------------

void CarouselMenuScreen::drawTitleBar() {
    _tft.fillRect(0, 0, SCR_W, TITLE_H, COL_BG);
    _tft.setTextFont(1);
    _tft.setTextSize(2);
    _tft.setTextColor(COL_TITLE, COL_BG);
    int textW = strlen(_title) * 12;
    _tft.setCursor((SCR_W - textW) / 2, (TITLE_H - 16) / 2);
    _tft.print(_title);
}

void CarouselMenuScreen::drawLabelBar() {
    _tft.fillRect(0, SCR_H - LABEL_H, SCR_W, LABEL_H, COL_BG);
    if (_count == 0) return;
    const char* label = _items[_selectedIndex].label;
    _tft.setTextFont(1);
    _tft.setTextSize(2);
    _tft.setTextColor(COL_LABEL, COL_BG);
    int textW = strlen(label) * 12;
    _tft.setCursor((SCR_W - textW) / 2, SCR_H - LABEL_H + (LABEL_H - 16) / 2);
    _tft.print(label);
}

// ---------------------------------------------------------------------------
// Tile drawing (carousel slide frames only)
// ---------------------------------------------------------------------------

void CarouselMenuScreen::drawTileArea() {
    // 3x5 glyphs for placeholder chars; rendered at 6x scale into a stable 18x30 area.
    auto glyphRows3x5 = [](char c) {
        struct Glyph { char ch; uint8_t rows[5]; };
        static const Glyph glyphs[] = {
            {'0', {0b111,0b101,0b101,0b101,0b111}},
            {'1', {0b010,0b110,0b010,0b010,0b111}},
            {'2', {0b111,0b001,0b111,0b100,0b111}},
            {'3', {0b111,0b001,0b111,0b001,0b111}},
            {'4', {0b101,0b101,0b111,0b001,0b001}},
            {'5', {0b111,0b100,0b111,0b001,0b111}},
            {'6', {0b111,0b100,0b111,0b101,0b111}},
            {'7', {0b111,0b001,0b010,0b010,0b010}},
            {'8', {0b111,0b101,0b111,0b101,0b111}},
            {'9', {0b111,0b101,0b111,0b001,0b111}},
            {'A', {0b111,0b101,0b111,0b101,0b101}},
            {'B', {0b110,0b101,0b110,0b101,0b110}},
            {'C', {0b111,0b100,0b100,0b100,0b111}},
            {'D', {0b110,0b101,0b101,0b101,0b110}},
            {'E', {0b111,0b100,0b110,0b100,0b111}},
            {'F', {0b111,0b100,0b110,0b100,0b100}},
            {'G', {0b111,0b100,0b101,0b101,0b111}},
            {'H', {0b101,0b101,0b111,0b101,0b101}},
            {'I', {0b111,0b010,0b010,0b010,0b111}},
            {'J', {0b111,0b001,0b001,0b101,0b111}},
            {'K', {0b101,0b101,0b110,0b101,0b101}},
            {'L', {0b100,0b100,0b100,0b100,0b111}},
            {'M', {0b101,0b111,0b111,0b101,0b101}},
            {'N', {0b101,0b111,0b111,0b111,0b101}},
            {'O', {0b111,0b101,0b101,0b101,0b111}},
            {'P', {0b111,0b101,0b111,0b100,0b100}},
            {'Q', {0b111,0b101,0b101,0b111,0b011}},
            {'R', {0b111,0b101,0b111,0b110,0b101}},
            {'S', {0b111,0b100,0b111,0b001,0b111}},
            {'T', {0b111,0b010,0b010,0b010,0b010}},
            {'U', {0b101,0b101,0b101,0b101,0b111}},
            {'V', {0b101,0b101,0b101,0b101,0b010}},
            {'W', {0b101,0b101,0b111,0b111,0b101}},
            {'X', {0b101,0b101,0b010,0b101,0b101}},
            {'Y', {0b101,0b101,0b010,0b010,0b010}},
            {'Z', {0b111,0b001,0b010,0b100,0b111}},
            {'?', {0b111,0b001,0b011,0b000,0b010}},
            {' ', {0b000,0b000,0b000,0b000,0b000}},
        };

        char uc = c;
        if (uc >= 'a' && uc <= 'z') uc = (char)(uc - ('a' - 'A'));
        for (unsigned int i = 0; i < sizeof(glyphs) / sizeof(glyphs[0]); i++) {
            if (glyphs[i].ch == uc) return glyphs[i].rows;
        }
        for (unsigned int i = 0; i < sizeof(glyphs) / sizeof(glyphs[0]); i++) {
            if (glyphs[i].ch == '?') return glyphs[i].rows;
        }
        return glyphs[0].rows;
    };

    // Collect visible tile data for the scanline pass
    struct TileInfo {
        int cx, cy;
        int x1, y1, x2, y2;
        uint16_t fillColour;
        uint16_t iconColour;
        const MenuItem* item;
    };

    TileInfo tiles[8];
    int nTiles = 0;

    for (int i = 0; i < _count && nTiles < 8; i++) {
        float dist = (float)i - _animOffset;
        if (_wrap && _count > 1) {
            float half = _count / 2.0f;
            while (dist >  half) dist -= _count;
            while (dist < -half) dist += _count;
        }
        if (fabsf(dist) > 1.5f) continue;

        float t    = min(fabsf(dist), 1.0f);
        int   size = (int)(CENTER_SIZE + (SIDE_SIZE - CENTER_SIZE) * t);
        int   tileX = CENTER_X + (int)(dist * TILE_STRIDE);

        TileInfo& ti  = tiles[nTiles++];
        ti.cx         = tileX;
        ti.cy         = TILE_AREA_CY;
        ti.x1         = tileX - size / 2;
        ti.y1         = TILE_AREA_CY - size / 2;
        ti.x2         = ti.x1 + size - 1;
        ti.y2         = ti.y1 + size - 1;
        ti.fillColour = (i == _selectedIndex) ? COL_ACCENT   : COL_MUTED;
        ti.iconColour = (i == _selectedIndex) ? COL_ICON_SEL : COL_ICON_UNSEL;
        ti.item       = &_items[i];
    }

    // Sort left-to-right so the scanline sweep is sequential
    for (int a = 0; a < nTiles - 1; a++)
        for (int b = a + 1; b < nTiles; b++)
            if (tiles[b].x1 < tiles[a].x1) {
                TileInfo tmp = tiles[a]; tiles[a] = tiles[b]; tiles[b] = tmp;
            }

    // Stream the whole tile area in one SPI transaction, including placeholder-char pixels.
    _tft.startWrite();
    _tft.setAddrWindow(0, TILE_AREA_Y, SCR_W, TILE_AREA_H);

    for (int y = TILE_AREA_Y; y < TILE_AREA_Y + TILE_AREA_H; y++) {
        int x = 0;
        for (int t = 0; t < nTiles; t++) {
            int dx1 = max(tiles[t].x1, x);
            int dx2 = min(tiles[t].x2, SCR_W - 1);
            if (dx2 < x || dx1 >= SCR_W) continue;

            if (dx1 > x) {
                _tft.writeColor(COL_BG, dx1 - x);
                x = dx1;
            }

            bool inY = (y >= tiles[t].y1 && y <= tiles[t].y2);
            if (!inY) {
                _tft.writeColor(COL_BG, dx2 - x + 1);
                x = dx2 + 1;
                continue;
            }

            const TileInfo& ti = tiles[t];
            bool isPlaceholder = (ti.item->bitmap == nullptr);
            if (!isPlaceholder) {
                _tft.writeColor(ti.fillColour, dx2 - x + 1);
                x = dx2 + 1;
                continue;
            }

            // Placeholder char bounds (matching previous 18x30 placement).
            const int charX = ti.cx - 9;
            const int charY = ti.cy - 12;
            const int charW = 18;
            const int charH = 30;

            int sx = x;
            int ex = dx2;
            if (y < charY || y >= charY + charH) {
                _tft.writeColor(ti.fillColour, ex - sx + 1);
                x = ex + 1;
                continue;
            }

            const uint8_t* rows = glyphRows3x5(ti.item->placeholderChar);
            int gy = (y - charY) / 6; // 0..4
            uint8_t rowMask = rows[gy];

            while (sx <= ex) {
                int gx = (sx - charX) / 6;
                bool inCharX = (sx >= charX && sx < charX + charW);
                bool on = inCharX && gx >= 0 && gx < 3 && ((rowMask >> (2 - gx)) & 0x1);
                uint16_t col = on ? ti.iconColour : ti.fillColour;

                int runStart = sx;
                sx++;
                while (sx <= ex) {
                    int ng = (sx - charX) / 6;
                    bool ninCharX = (sx >= charX && sx < charX + charW);
                    bool non = ninCharX && ng >= 0 && ng < 3 && ((rowMask >> (2 - ng)) & 0x1);
                    uint16_t ncol = non ? ti.iconColour : ti.fillColour;
                    if (ncol != col) break;
                    sx++;
                }
                _tft.writeColor(col, sx - runStart);
            }

            x = ex + 1;
        }
        if (x < SCR_W) _tft.writeColor(COL_BG, SCR_W - x);
    }

    _tft.endWrite();

    // Draw bitmap icons on top; placeholder chars are already baked into the tile stream.
    for (int t = 0; t < nTiles; t++) {
        const TileInfo& ti = tiles[t];
        if (ti.x1 >= SCR_W || ti.x2 < 0) continue;
        if (ti.item->bitmap != nullptr) {
            _tft.drawBitmap(ti.cx - ti.item->bitmapW / 2,
                            ti.cy - ti.item->bitmapH / 2,
                            ti.item->bitmap, ti.item->bitmapW, ti.item->bitmapH,
                            ti.iconColour);
        }
    }
}

// ---------------------------------------------------------------------------
// Animated wave border — delta drawing, no tile redraw
// ---------------------------------------------------------------------------

void CarouselMenuScreen::drawQuarterDisc(int cx, int cy, int quadrant, int r, uint16_t colour) {
    // Fill a quarter-disc outside the tile corner (cx, cy) with horizontal line spans.
    // The disc does not overlap with any straight-edge draw region:
    //   TL (q=0): draws at x < cx, y < cy
    //   TR (q=1): draws at x >= cx, y < cy
    //   BR (q=2): draws at x >= cx, y >= cy
    //   BL (q=3): draws at x < cx, y >= cy
    for (int d = 0; d < r; d++) {
        int span = (int)sqrtf((float)(r * r - d * d));
        if (span <= 0) continue;
        switch (quadrant) {
            case 0: _tft.drawFastHLine(cx - span, cy - 1 - d, span, colour); break;
            case 1: _tft.drawFastHLine(cx,         cy - 1 - d, span, colour); break;
            case 2: _tft.drawFastHLine(cx,         cy + d,     span, colour); break;
            case 3: _tft.drawFastHLine(cx - span,  cy + d,     span, colour); break;
        }
    }
}

uint16_t CarouselMenuScreen::blendColor(uint16_t bg, uint16_t fg, float alpha) {
    // Clamp alpha to [0, 1]
    alpha = max(0.0f, min(alpha, 1.0f));

    // Extract RGB565 components
    uint8_t bgR = (bg >> 11) & 0x1F;
    uint8_t bgG = (bg >> 5)  & 0x3F;
    uint8_t bgB = bg & 0x1F;
    uint8_t fgR = (fg >> 11) & 0x1F;
    uint8_t fgG = (fg >> 5)  & 0x3F;
    uint8_t fgB = fg & 0x1F;

    // Linear interpolation
    uint8_t r = (uint8_t)(bgR + (fgR - bgR) * alpha + 0.5f);
    uint8_t g = (uint8_t)(bgG + (fgG - bgG) * alpha + 0.5f);
    uint8_t b = (uint8_t)(bgB + (fgB - bgB) * alpha + 0.5f);

    return (r << 11) | (g << 5) | b;
}

void CarouselMenuScreen::drawBorderWithRoundedCorners(int x, int y, int w, int h, float rExact, uint16_t colour) {
    int r = (int)rExact;
    float frac = rExact - r;

    // AA fringe blended based on the exact float remainder
    uint16_t aaColour = blendColor(COL_BG, colour, frac);

    if (r > 0) {
        // Main solid border strokes (r pixels thick on each edge)
        _tft.fillRect(x,     y - r, w,     r, colour);
        _tft.fillRect(x + w, y,     r,     h, colour);
        _tft.fillRect(x,     y + h, w,     r, colour);
        _tft.fillRect(x - r, y,     r,     h, colour);

        // Solid corner quarter-discs
        drawQuarterDisc(x,     y,     0, r, colour);
        drawQuarterDisc(x + w, y,     1, r, colour);
        drawQuarterDisc(x + w, y + h, 2, r, colour);
        drawQuarterDisc(x,     y + h, 3, r, colour);
    }

    if (frac > 0.01f) {
        // AA fringe on straight edges — one pixel outside the solid border
        _tft.drawFastHLine(x, y - r - 1, w, aaColour);
        _tft.drawFastHLine(x, y + h + r, w, aaColour);
        _tft.drawFastVLine(x - r - 1, y, h, aaColour);
        _tft.drawFastVLine(x + w + r, y, h, aaColour);

        // AA fringe on corners — the sub-pixel ring between radius r and r+1
        int ro = r + 1;
        for (int d = 0; d <= r; d++) {
            int spanO = (int)sqrtf((float)(ro * ro - d * d));
            int spanI = (d < r) ? (int)sqrtf((float)(r * r - d * d)) : 0;
            int diff = spanO - spanI;
            if (diff <= 0) continue;
            _tft.drawFastHLine(x     - spanO, y     - 1 - d, diff, aaColour); // TL
            _tft.drawFastHLine(x + w + spanI, y     - 1 - d, diff, aaColour); // TR
            _tft.drawFastHLine(x + w + spanI, y + h + d,     diff, aaColour); // BR
            _tft.drawFastHLine(x     - spanO, y + h + d,     diff, aaColour); // BL
        }
    }
}

void CarouselMenuScreen::updateBorder() {
    // Tile bounding box of the settled center tile
    const int x = CENTER_X    - CENTER_SIZE / 2;
    const int y = TILE_AREA_CY - CENTER_SIZE / 2;
    const int w = CENTER_SIZE;
    const int h = CENTER_SIZE;

    // Compute fade alpha based on carousel movement (fades when moving, full opacity when settled)
    float gap = fabsf(_animOffset - _targetOffset);
    float fadeAlpha = (1.0f - min(gap * 2.0f, 1.0f)) * _borderEnvelope;

    // Don't draw border if it's too faded out; clear only the previously drawn ring
    if (fadeAlpha < 0.15f) {
        if (_lastBorderThickness >= 0.0f) {
            drawBorderWithRoundedCorners(x, y, w, h, _lastBorderThickness, COL_BG);
        }
        _lastBorderThickness = -1.0f;
        _lastBorderColor = 0;
        return;
    }

    // Compute exact float thickness with time-based sine wave
    float t = 0.5f + 0.5f * sinf(_borderPhase);
    float exactThickness = BORDER_T_MIN + (BORDER_T_MAX - BORDER_T_MIN) * t;

    // Blend border colour with background based on fade alpha
    uint16_t borderColour = fadeAlpha >= 0.99f ? COL_BORDER : blendColor(COL_BG, COL_BORDER, fadeAlpha);

    // Only redraw if thickness or colour changed noticeably
    if (fabsf(exactThickness - _lastBorderThickness) < 0.01f && borderColour == _lastBorderColor) {
        return;
    }

    // First visible border frame: draw once, no erase pass needed.
    if (_lastBorderThickness < 0.0f) {
        drawBorderWithRoundedCorners(x, y, w, h, exactThickness, borderColour);
    } else if (borderColour != _lastBorderColor) {
        // Colour changed: repaint current ring in-place to avoid any clear flash.
        drawBorderWithRoundedCorners(x, y, w, h, exactThickness, borderColour);
    } else {
        // Thickness-only update: use draw-only repaint for both growth and shrink to avoid erase flash.
        drawBorderWithRoundedCorners(x, y, w, h, exactThickness, borderColour);
    }

    _lastBorderThickness = exactThickness;
    _lastBorderColor = borderColour;
}
