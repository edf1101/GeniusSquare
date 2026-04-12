/*
 * Created by Ed Fillingham on 25/07/2025.
 *
 * All other displays (HUD and sideDisplay classes implement the main display class.
 * This provides a similar interface that widgets can use to render to the display.
*/

#ifndef DISPLAY_DISPLAY_H
#define DISPLAY_DISPLAY_H

#include <Arduino.h>
#include "Renderer.h"

#include <vector>

class Widget; // Forward declaration of Widget class

class Display : public Renderer {
public:
    virtual void init() = 0;

    virtual void updateDisplay() = 0; // updates the display with the current widgets
    virtual void clearDisplay() = 0; // clears the display

    virtual int getDisplayWidth() = 0;

    virtual int getDisplayHeight() = 0;

    void clearWidgets(); // clears all widgets from the display
    void addWidget(Widget *widget);
    void removeWidget(Widget *widget);


protected:
    void drawWidgets(); // draws all the stored widgets to the screen

    // "Primitive operations" that *must* be implemented by concrete displays
    virtual void drawLineImpl(int, int, int, int, uint16_t) = 0;

    virtual void drawCircleImpl(int, int, int, uint16_t) = 0;

    virtual void drawFilledCircleImpl(int, int, int, uint16_t) = 0;

    virtual void drawRectImpl(int, int, int, int, uint16_t) = 0;

    virtual void drawFilledRectImpl(int, int, int, int, uint16_t) = 0;

    virtual void drawTextImpl(int, int, const char *, int, uint16_t) = 0;

    virtual void drawXBMImpl(int, int, const uint8_t *, int, int, uint16_t) = 0;

    virtual void drawPixelImpl(int, int, uint16_t) = 0;

private:
    // Final wrappers that satisfy the public Renderer contract
    void drawLine(int x1, int y1, int x2, int y2, uint16_t c) final override { drawLineImpl(x1, y1, x2, y2, c); }

    void drawCircle(int x, int y, int r, uint16_t c) final override { drawCircleImpl(x, y, r, c); }

    void drawFilledCircle(int x, int y, int r, uint16_t c) final override { drawFilledCircleImpl(x, y, r, c); }

    void drawRect(int x, int y, int w, int h, uint16_t c) final override { drawRectImpl(x, y, w, h, c); }

    void drawFilledRect(int x, int y, int w, int h, uint16_t c) final override { drawFilledRectImpl(x, y, w, h, c); }

    void drawText(int x, int y, const char *t, int textSize, uint16_t c) final override {
      drawTextImpl(x, y, t, textSize, c);
    }

    void drawXBM(int x, int y, const uint8_t *b, int w, int h, uint16_t c) final override {
      drawXBMImpl(x, y, b, w, h, c);
    }

    void drawPixel(int x, int y, uint16_t c) final override { drawPixelImpl(x, y, c); }

    std::vector<Widget *> widgets; // List of widgets to be drawn on the display
};


#endif //DISPLAY_DISPLAY_H
