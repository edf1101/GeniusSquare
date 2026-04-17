/*
 * Created by Ed Fillingham on 17/04/2026.
 *
 * BorderAnimator — shared glow-border animation for menu screens.
 */

#include "menu/BorderAnimator.h"
#include <math.h>

void BorderAnimator::advance(float dt) {
    phase += BORDER_SPEED * dt;
    if (phase >= 2.0f * (float)M_PI) phase -= 2.0f * (float)M_PI;
    envelope = fminf(envelope + BORDER_INTRO_SPEED * dt, 1.0f);
}

void BorderAnimator::reset() {
    phase         = 0.f;
    envelope      = 0.f;
    lastThickness = -1.0f;
    lastColor     = 0;
}

uint16_t BorderAnimator::blendColor(uint16_t bg, uint16_t fg, float alpha) {
    uint8_t r = ((bg >> 11) & 0x1F) + (int)(((int)((fg >> 11) & 0x1F) - (int)((bg >> 11) & 0x1F)) * alpha);
    uint8_t g = ((bg >>  5) & 0x3F) + (int)(((int)((fg >>  5) & 0x3F) - (int)((bg >>  5) & 0x3F)) * alpha);
    uint8_t b = ((bg      ) & 0x1F) + (int)(((int)( fg        & 0x1F) - (int)( bg        & 0x1F)) * alpha);
    return (uint16_t)((r << 11) | (g << 5) | b);
}

void BorderAnimator::drawQuarterDisc(TFT_eSPI& tft, int cx, int cy, int quadrant, int r, uint16_t colour) {
    for (int d = 0; d < r; d++) {
        int span = (int)sqrtf((float)(r * r - d * d));
        if (span <= 0) continue;
        switch (quadrant) {
            case 0: tft.drawFastHLine(cx - span, cy - 1 - d, span, colour); break;
            case 1: tft.drawFastHLine(cx,         cy - 1 - d, span, colour); break;
            case 2: tft.drawFastHLine(cx,         cy + d,     span, colour); break;
            case 3: tft.drawFastHLine(cx - span,  cy + d,     span, colour); break;
        }
    }
}

void BorderAnimator::drawBorderWithRoundedCorners(TFT_eSPI& tft,
                                                   int x, int y, int w, int h,
                                                   float rExact, uint16_t colour,
                                                   uint16_t bgColour) {
    int r = (int)rExact;
    float frac = rExact - (float)r;
    uint16_t aaColour = blendColor(bgColour, colour, frac);

    if (r > 0) {
        tft.fillRect(x,     y - r, w,     r, colour);  // Top
        tft.fillRect(x + w, y,     r,     h, colour);  // Right
        tft.fillRect(x,     y + h, w,     r, colour);  // Bottom
        tft.fillRect(x - r, y,     r,     h, colour);  // Left
        drawQuarterDisc(tft, x,     y,     0, r, colour);  // TL
        drawQuarterDisc(tft, x + w, y,     1, r, colour);  // TR
        drawQuarterDisc(tft, x + w, y + h, 2, r, colour);  // BR
        drawQuarterDisc(tft, x,     y + h, 3, r, colour);  // BL
    }

    if (frac > 0.01f) {
        tft.drawFastHLine(x, y - r - 1, w, aaColour);
        tft.drawFastHLine(x, y + h + r, w, aaColour);
        tft.drawFastVLine(x - r - 1, y, h, aaColour);
        tft.drawFastVLine(x + w + r, y, h, aaColour);

        int ro = r + 1;
        for (int d = 0; d <= r; d++) {
            int spanO = (int)sqrtf((float)(ro * ro - d * d));
            int spanI = (d < r) ? (int)sqrtf((float)(r * r - d * d)) : 0;
            int diff  = spanO - spanI;
            if (diff <= 0) continue;
            tft.drawFastHLine(x     - spanO, y     - 1 - d, diff, aaColour);
            tft.drawFastHLine(x + w + spanI, y     - 1 - d, diff, aaColour);
            tft.drawFastHLine(x + w + spanI, y + h + d,     diff, aaColour);
            tft.drawFastHLine(x     - spanO, y + h + d,     diff, aaColour);
        }
    }
}
