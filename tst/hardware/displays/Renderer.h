/*
 * Created by Ed Fillingham on 25/07/2025.
 *
 * This file is for a virtual renderer class that all Displays must inherit from and
 * implement the rendering functions. This is here so that the HUD and side display can potentially
 * use different libs if they want to render to a different display type, yet all widgets
 * can be used on either.
*/

#ifndef LASERTAG25_RENDERER_H
#define LASERTAG25_RENDERER_H

#include <Arduino.h>

class Renderer {
public:
    virtual ~Renderer() = default;

    /**
     * Draws a line from (x1, y1) to (x2, y2) with the specified colour.
     * @param x1 Start x coordinate
     * @param y1 Start y coordinate
     * @param x2 End x coordinate
     * @param y2 End y coordinate
     * @param c Colour of the line
     */
    virtual void drawLine(int x1, int y1, int x2, int y2, uint16_t c) = 0;
    /**
     * Draws a circle with center (x, y) and radius r with the specified colour.
     * @param x Center x coordinate
     * @param y Center y coordinate
     * @param r Radius of the circle
     * @param c Colour of the circle
     */
    virtual void drawCircle(int x, int y, int r, uint16_t c) = 0;
    /**
     * Draws a filled circle with center (x, y) and radius r with the specified colour.
     * @param x Center x coordinate
     * @param y Center y coordinate
     * @param r Radius of the circle
     * @param c Colour of the filled circle
     */
    virtual void drawFilledCircle(int x, int y, int r, uint16_t c) = 0;

    /**
     * Draws a rectangle with top-left corner (x, y), width w, height h with the specified colour.
     * @param x Top-left x coordinate
     * @param y Top-left y coordinate
     * @param w Width of the rectangle
     * @param h Height of the rectangle
     * @param c Colour of the rectangle
     */
    virtual void drawRect(int x, int y, int w, int h, uint16_t c) = 0;
    /**
     * Draws a filled rectangle with top-left corner (x, y), width w, height h with the specified colour.
     * @param x Top-left x coordinate
     * @param y Top-left y coordinate
     * @param w Width of the rectangle
     * @param h Height of the rectangle
     * @param c Colour of the filled rectangle
     */
    virtual void drawFilledRect(int x, int y, int w, int h, uint16_t c) = 0;
    /**
     * Draws text at position (x, y) top left with the specified colour.
     * @param x X coordinate for the text (top left of 1st char)
     * @param y Y coordinate for the text (top left of 1st char)
     * @param txt Pointer to the null-terminated string to draw
     * @param textSize Size of the text (1 for normal, 2 for double size, 3 for triple size)
     * @param c Colour of the text
     */
    virtual void drawText(int x, int y, const char *txt, int textSize, uint16_t c) = 0;

    /**
     * Draws an XBM image at position (x, y) with the specified colour.
     * @param x X coordinate for the image (top left)
     * @param y Y coordinate for the image (top left)
     * @param bmp Pointer to the XBM bitmap data
     * @param w Width of the bitmap
     * @param h Height of the bitmap
     * @param c Colour to draw the bitmap in
     */
    virtual void drawXBM(int x, int y, const uint8_t *bmp, int w, int h, uint16_t c) = 0;
    /**
     * Draws a single pixel at (x, y) with the specified colour.
     * @param x X coordinate of the pixel
     * @param y Y coordinate of the pixel
     * @param c Colour of the pixel
     */
    virtual void drawPixel(int x, int y, uint16_t c) = 0;
    /**
     * Gets the main colour used for drawing.
     * @return The main colour as a 16-bit value
     */
    virtual uint16_t getMainColour() const = 0;
    /**
     * Gets the highlight colour used for drawing.
     * @return The highlight colour as a 16-bit value
     */
    virtual uint16_t getHighlightColour() const = 0;

    /**
     * Gets the background colour used for drawing.
     * @return The background colour as a 16-bit value
     */
    virtual uint16_t getBackgroundColour() const = 0;

    /**
     * Gets the text size used for drawing text.
     * @param text The text to measure
     * @param textSize The size of the text (1 for normal, 2 for double size, 3 for triple size)
     * @return The size of the text as an integer (in pixels)
     */
    virtual int getTextWidth(const char *text, int textSize) = 0;
};

#endif //LASERTAG25_RENDERER_H
