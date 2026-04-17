/*
 * Created by Ed Fillingham on 17/04/2026.
 *
 * Shared border animation state and drawing helpers for menu screens.
 * Extracts the animated glow-border logic from ListMenuScreen so multiple
 * screens can reuse it without duplication.
 */

#ifndef BORDER_ANIMATOR_H
#define BORDER_ANIMATOR_H

#include <TFT_eSPI.h>

/**
 * @brief Manages animated glow-border state for a selected menu row.
 *
 * Holds phase/envelope state. Call advance() each update tick.
 * Use the static draw helpers to render the border to the TFT.
 */
struct BorderAnimator {
    float    phase         = 0.f;   ///< Sine-wave phase (radians)
    float    envelope      = 0.f;   ///< Fade-in envelope [0.0, 1.0]
    float    lastThickness = -1.0f; ///< Thickness drawn last frame (-1 = nothing drawn)
    uint16_t lastColor     = 0;     ///< Colour drawn last frame

    /**
     * @brief Advance phase and fade envelope toward 1.
     * @param dt Delta time in seconds since last call.
     */
    void advance(float dt);

    /**
     * @brief Reset all animation state. Call when the selected row changes.
     *        Zeroes phase and envelope; marks no border drawn (lastThickness = -1).
     */
    void reset();

    /**
     * @brief Linear interpolation between two RGB565 colours.
     * @param bg    Background colour.
     * @param fg    Foreground colour.
     * @param alpha Blend factor [0.0, 1.0].
     * @return Blended RGB565 colour.
     */
    static uint16_t blendColor(uint16_t bg, uint16_t fg, float alpha);

    /**
     * @brief Draw a uniform-thickness border with rounded corners around a box.
     *        Integer part of rExact = solid fill; fractional part = 1px AA fringe.
     *        The AA fringe cleans up ghost pixels on shrink — no erase pass needed.
     * @param tft      TFT display reference.
     * @param x        Box left edge.
     * @param y        Box top edge.
     * @param w        Box width.
     * @param h        Box height.
     * @param rExact   Fractional border thickness.
     * @param colour   Border colour (pass bgColour to erase).
     * @param bgColour Background colour used for AA fringe blending.
     */
    static void drawBorderWithRoundedCorners(TFT_eSPI& tft,
                                              int x, int y, int w, int h,
                                              float rExact, uint16_t colour,
                                              uint16_t bgColour);

    /**
     * @brief Fill a quarter-disc at a box corner (used by drawBorderWithRoundedCorners).
     * @param tft      TFT display reference.
     * @param cx       Corner x coordinate.
     * @param cy       Corner y coordinate.
     * @param quadrant 0=TL, 1=TR, 2=BR, 3=BL.
     * @param r        Disc radius (integer border thickness).
     * @param colour   Fill colour.
     */
    static void drawQuarterDisc(TFT_eSPI& tft,
                                 int cx, int cy, int quadrant,
                                 int r, uint16_t colour);

    // Animation constants — shared by all screens
    static constexpr float BORDER_T_MIN       = 1.0f;
    static constexpr float BORDER_T_MAX       = 3.0f;
    static constexpr float BORDER_SPEED       = 4.0f;   ///< rad/sec
    static constexpr float BORDER_INTRO_SPEED = 8.0f;   ///< envelope fade-in rate
    static constexpr float BORDER_CHANGE_TOL  = 0.01f;  ///< Callers: use this tolerance when comparing lastThickness to new thickness to decide whether to redraw
};

#endif // BORDER_ANIMATOR_H
