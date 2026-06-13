/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * CarouselMenuScreen — animated horizontal tile carousel menu.
 *
 * Displays menu items as horizontally scrolling tiles with morphing sizes:
 * - Center tile: large (90×90px), fully opaque, selected colour (cyan)
 * - Adjacent tiles (±1): smaller (58×58px), dimmed, side-by-side
 * - Edge tiles (±1.5+): nearly off-screen, invisible after 1.5 tile units away
 *
 * Animations include:
 * - Carousel position lerp: smooth slide to new selection (8 units/sec)
 * - Border wave: animated glow around center tile (4 rad/sec sine wave)
 * - Border fade: in/out envelope tied to carousel settling (125ms fade-in, 67ms fade-out)
 *
 * All rendering is SPI-transaction-aware; no partial TFT writes that could
 * flicker. The border uses delta rendering — only redraws when thickness
 * or colour changes, avoiding unnecessary SPI traffic.
 */

#ifndef CAROUSEL_MENU_SCREEN_H
#define CAROUSEL_MENU_SCREEN_H

#include <TFT_eSPI.h>
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
    CarouselMenuScreen(TFT_eSPI& tft, ScreenManager& manager,
                       const MenuItem* items, uint8_t count,
                       const char* title, bool wrap = false);

    void onEnter() override;
    void onExit() override;
    void onResume() override;
    void update() override;
    void render() override;
    void onEncoderChange(int delta) override;
    void onButtonPress() override;

private:
    TFT_eSPI&        _tft;
    ScreenManager&   _manager;
    const MenuItem* _items;
    uint8_t          _count;
    const char* _title;
    bool             _wrap;

    float         _animOffset;      ///< Current visual position (0.0 = item 0 centred)
    float         _prevAnimOffset;  ///< animOffset from previous render, detects carousel motion
    float         _targetOffset;    ///< Destination position (always an integer)
    unsigned long _lastMs;          ///< Timestamp of last update(), for delta-time

    int   _selectedIndex;   ///< Integer index of the currently selected item
    bool  _dirty;           ///< True when at least one redraw is pending
    float _borderPhase;     ///< Accumulated phase offset for the wave border animation

    // ---- Layout (all in pixels, landscape 280×240) ----
    static constexpr int SCR_W        = 280;
    static constexpr int SCR_H        = 240;
    static constexpr int TITLE_H      = 30;
    static constexpr int LABEL_H      = 50;
    static constexpr int TILE_AREA_Y  = TITLE_H;
    static constexpr int TILE_AREA_H  = SCR_H - TITLE_H - LABEL_H;
    static constexpr int TILE_AREA_CY = TILE_AREA_Y + TILE_AREA_H / 2;
    static constexpr int CENTER_X     = SCR_W / 2;
    static constexpr int CENTER_SIZE  = 90;  ///< Tile side length when centred
    static constexpr int SIDE_SIZE    = 58;  ///< Tile side length when at ±1
    static constexpr int TILE_STRIDE  = 110; ///< Horizontal distance between tile centres

    // ---- Tunable animation parameters ----
    static constexpr float CAROUSEL_LERP_SPEED = 8.0f;  ///< Carousel slide lerp factor (units/sec)
    static constexpr float BORDER_SPEED        = 4.0f;  ///< Border wave phase advance (rad/sec)
    static constexpr int   BORDER_T_MIN        = 3;     ///< Minimum border thickness (px)
    static constexpr int   BORDER_T_MAX        = 5;     ///< Maximum border thickness (px)
    static constexpr float BORDER_INTRO_SPEED  = 8.0f;  ///< Envelope rate for border intro (units/sec, ~125 ms)
    static constexpr float BORDER_OUTRO_SPEED  = 15.0f; ///< Envelope rate for border outro (units/sec, ~67 ms)

    // ---- Animated border state ----
    float    _lastBorderThickness;  ///< Thickness from previous frame (for delta detection, converted to float for AA)
    uint16_t _lastBorderColor;      ///< Colour from previous frame (for fade blending)

    // ---- Colour palette (RGB565) ----
    static constexpr uint16_t COL_BG         = 0x0000; ///< Background — black
    static constexpr uint16_t COL_ACCENT     = 0x07FF; ///< Selected tile — cyan
    static constexpr uint16_t COL_MUTED      = 0x2965; ///< Unselected tile — dark blue-grey
    static constexpr uint16_t COL_BORDER     = 0xFFFF; ///< Wave selection border — white
    static constexpr uint16_t COL_LABEL      = 0xFFFF; ///< Label bar text — white
    static constexpr uint16_t COL_TITLE      = 0xAD75; ///< Title bar text — light grey
    static constexpr uint16_t COL_ICON_SEL   = 0xFFFF; ///< Icon on selected tile — white
    static constexpr uint16_t COL_ICON_UNSEL = 0xFFFF; ///< Icon on unselected tile — white

    void drawTitleBar();
    void drawLabelBar();

    /**
     * @brief Stream the entire tile area to the display in one SPI transaction.
     * Each pixel's colour (tile or background) is computed on-the-fly, so the
     * display never sees an intermediate black frame between clear and redraw.
     */
    void drawTileArea();

    /**
     * @brief Update the animated border around the settled center tile.
     * Border has uniform thickness (animated via sine wave) and fades with carousel motion.
     * Only redraws when thickness or colour changes.
     */
    void updateBorder();

    /**
     * @brief Draw bitmap icons for all visible tiles (called only when carousel is settled).
     */
    void drawBitmaps();

    /**
     * @brief Draw a uniform-thickness border with rounded corners around a tile.
     * @param x      Tile left x.
     * @param y      Tile top y.
     * @param w      Tile width.
     * @param h      Tile height.
     * @param rExact Fractional uniform border thickness.
     * @param colour Border colour.
     */
    void drawBorderWithRoundedCorners(int x, int y, int w, int h, float rExact, uint16_t colour);

    /**
     * @brief Draw a filled quarter-disc at a tile corner.
     * @param cx       x coordinate of the tile corner.
     * @param cy       y coordinate of the tile corner.
     * @param quadrant 0=TL, 1=TR, 2=BR, 3=BL — determines which quadrant is filled.
     * @param r        Disc radius (= border thickness at this corner).
     * @param colour   Fill colour.
     */
    void drawQuarterDisc(int cx, int cy, int quadrant, int r, uint16_t colour);

    /**
     * @brief Blend two RGB565 colours by linear interpolation.
     * @param bg    Background colour.
     * @param fg    Foreground colour.
     * @param alpha Blend factor [0.0, 1.0], where 0.0 = bg, 1.0 = fg.
     * @return Blended colour.
     */
    uint16_t blendColor(uint16_t bg, uint16_t fg, float alpha);

    float _borderEnvelope;     // 0..1 intro/outro envelope for border thickness
    bool  _borderPulseActive;  // pulse starts only after intro reaches BORDER_T_

    float _bitmapFade;         // 0..1 fade envelope for bitmap icons
};

#endif // CAROUSEL_MENU_SCREEN_H