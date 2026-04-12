/*
 * Created by Ed Fillingham on 25/07/2025.
*/

#ifndef SIDEDISPLAY_SIDEDISPLAY_H
#define SIDEDISPLAY_SIDEDISPLAY_H

#include "Display.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <config.h>

class SideDisplay : public Display {
public:
    SideDisplay(SPIClass &spi);
    void init() override;

    int getDisplayWidth() override;

    int getDisplayHeight() override;

    void updateDisplay() override;

    void clearDisplay() override;

    void setMainColour(uint16_t colour);

private:
    Adafruit_ST7789 rawTftDisplay;
    uint16_t mainColour = ST77XX_RED; // Main colour for the display

    // Implementations for the "primitive operations" that must be defined by concrete displays
    void drawLineImpl(int x1, int y1, int x2, int y2, uint16_t colour) override;

    void drawCircleImpl(int, int, int, uint16_t) override;

    void drawFilledCircleImpl(int, int, int, uint16_t) override;

    void drawRectImpl(int, int, int, int, uint16_t) override;

    void drawFilledRectImpl(int, int, int, int, uint16_t) override;

    void drawTextImpl(int, int, const char *, int, uint16_t) override;

    void drawXBMImpl(int, int, const uint8_t *, int, int, uint16_t) override;

    void drawPixelImpl(int, int, uint16_t) override;

    uint16_t getMainColour() const override { return mainColour; }

    uint16_t getHighlightColour() const override { return ST77XX_WHITE; }

    uint16_t getBackgroundColour() const override { return ST77XX_BLACK; }

    int getTextWidth(const char *text, int textSize)  override;
};


#endif //LASERTAG25_SIDEDISPLAY_H
