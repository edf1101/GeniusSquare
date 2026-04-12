/*
 * Created by Ed Fillingham on 25/07/2025.
*/

#include "SideDisplay.h"
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>

/**
 * This constructor is needed to init the display
 */
SideDisplay::SideDisplay(SPIClass &spi) : rawTftDisplay(
        Adafruit_ST7789(&spi,DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN)) {

}

/**
 * This function initialises the side display.
 */
void SideDisplay::init() {

  rawTftDisplay.init(240, 280);
  rawTftDisplay.setRotation(1); // Usually makes it 280x240 landscape
  rawTftDisplay.fillScreen(ST77XX_BLACK);
}

/**
 * This function updates the display by drawing the widgets on it
 */
void SideDisplay::updateDisplay() {
  drawWidgets();

}

/**
 * This function clears the display
 */
void SideDisplay::clearDisplay() {
  clearWidgets();

}

/**
 * Gets the width of the display
 *
 * @return The width of the display in pixels
 */
int SideDisplay::getDisplayWidth() {
  return rawTftDisplay.width();
}

/**
 * Gets the height of the display
 *
 * @return The height of the display in pixels
 */
int SideDisplay::getDisplayHeight() {
  return rawTftDisplay.height();
}

/**
 * Sets the main colour for the display
 *
 * @param colour The colour to set as the main colour
 */
void SideDisplay::setMainColour(uint16_t colour) {
  mainColour = colour; // Set the main colour for the display

}


// Functions below are implementations for the Renderer primitives

/**
 * Draw a line
 *
 * @param x1 Starting x point
 * @param y1 Starting y point
 * @param x2 Ending x point
 * @param y2 Ending y point
 * @param colour Colour to draw in
 */
void SideDisplay::drawLineImpl(int x1, int y1, int x2, int y2, uint16_t c) {
  rawTftDisplay.drawLine(x1, y1, x2, y2, c);
}

/**
 * Draw a circle
 *
 * @param x center x
 * @param y center y
 * @param r radius
 * @param c colour
 */
void SideDisplay::drawCircleImpl(int x, int y, int r, uint16_t c) { rawTftDisplay.drawCircle(x, y, r, c); }

/**
 * Draw a filled circle
 *
 * @param x center x
 * @param y center y
 * @param r radius
 * @param c colour
 */
void SideDisplay::drawFilledCircleImpl(int x, int y, int r, uint16_t c) { rawTftDisplay.fillCircle(x, y, r, c); }

/**
 * Draw a rectangle
 *
 * @param x starting x (top left)
 * @param y starting y (top left)
 * @param w width
 * @param h height
 * @param c colour
 */
void SideDisplay::drawRectImpl(int x, int y, int w, int h, uint16_t c) { rawTftDisplay.drawRect(x, y, w, h, c); }

/**
 * Draw a filled rectangle
 *
 * @param x starting x (top left)
 * @param y starting y (top left)
 * @param w width
 * @param h height
 * @param c colour
 */
void SideDisplay::drawFilledRectImpl(int x, int y, int w, int h, uint16_t c) { rawTftDisplay.fillRect(x, y, w, h, c); }

/**
 * Draw a pixel
 *
 * @param x x coordinate
 * @param y y coordinate
 * @param c colour
 */
void SideDisplay::drawPixelImpl(int x, int y, uint16_t c) { rawTftDisplay.drawPixel(x, y, c); }

/** Draw text
 *
 * @param x starting x
 * @param y starting y
 * @param txt text to draw
 * @param textSize size of the text
 * @param c colour
 */
void SideDisplay::drawTextImpl(int x, int y, const char *txt, int textSize, uint16_t c) {
  rawTftDisplay.setTextColor(c); // set colour

  // set text size
  textSize = constrain(textSize, 1, 3);
  int textHeight = 0;
  switch (textSize) {
    case 1:
      rawTftDisplay.setFont(NULL); // Use 9pt font for size 1
      rawTftDisplay.setTextSize(1);
      textHeight = 0; // Set text height for size 1
      break;
    case 2:
      rawTftDisplay.setFont(&FreeSansBold9pt7b); // Use 9pt font for size 1 (New)
      textHeight = 15; // Set text height for size 2
      rawTftDisplay.setTextSize(1);
      break;
    case 3:
      rawTftDisplay.setFont(&FreeMonoBold18pt7b); // Use 24pt font for size 3
      rawTftDisplay.setTextSize(1);
      textHeight = 24; // Set text height for size 3
      break;
    default:
      rawTftDisplay.setFont(NULL); // default to 9pt font if size is out of range
      textHeight = 0; // Set default text height for size 1
      rawTftDisplay.setTextSize(1);
      break;
  }

  rawTftDisplay.setCursor(x, y + textHeight);
  rawTftDisplay.print(txt);
}

/**
 * Draw a bitmap (XBM format)
 *
 * @param x top left x coordinate
 * @param y top left y coordinate
 * @param bmp bitmap image
 * @param w width of the image
 * @param h height of the image
 * @param c colour
 */
void SideDisplay::drawXBMImpl(int x, int y, const uint8_t *bmp, int w, int h, uint16_t c) {
  rawTftDisplay.drawXBitmap(x, y, bmp, w, h, c);
}

/**
 * calculate the width of a text string
 *
 * @param text The text to measure
 * @param textSize The size of the text (1 for normal, 2 for double size, 3 for triple size)
 * @return The width of the text in pixels
 */
int SideDisplay::getTextWidth(const char *text, int textSize) {
  // set the text size first
  // set text size
  textSize = constrain(textSize, 1, 3);
  switch (textSize) {
    case 1:
      rawTftDisplay.setFont(NULL); // Use 9pt font for size 1
      rawTftDisplay.setTextSize(1);
      break;
    case 2:
      rawTftDisplay.setFont(&FreeSansBold9pt7b); // Use 9pt font for size 1 (New)
      rawTftDisplay.setTextSize(1);
      break;
    case 3:
      rawTftDisplay.setFont(&FreeMonoBold18pt7b); // Use 24pt font for size 3
      rawTftDisplay.setTextSize(1);
      break;
    default:
      rawTftDisplay.setFont(NULL); // default to 9pt font if size is out of range
      rawTftDisplay.setTextSize(1);
      break;
  }
  int16_t x, y;
  uint16_t w, h;
  rawTftDisplay.getTextBounds(text, 0, 0, &x, &y, &w, &h);
  return w;
}


