/*
 * Created by Ed Fillingham on 29/07/2025.
*/

#include "TextWidget.h"

/**
 * Constructor for the TextWidget class.
 *
 * @param x The x coordinate of the widget's position.
 * @param y The y coordinate of the widget's position.
 * @param width The width of the widget.
 * @param height The height of the widget.
 * @param text The text to be displayed by this widget.
 * @param textSize The size of the text to be displayed.
 */
TextWidget::TextWidget(int x, int y, int width, int height, std::string text, int textSize, uint16_t col)
        : Widget(x, y, width, height), text(std::move(text)), textSize(textSize), col(col) {

  registerHashField(this->text); // Register the text for hashing
  registerHashField(this->textSize); // Register the text size for hashing
  registerHashField(this->col); // Register the colour for hashing
}

/**
 * Draws the text widget on the specified render target.
 *
 * @param newText the text to write to the widget
 */
void TextWidget::setText(const std::string &newText) {
  text = newText;
}

/**
 * Draws the text widget on the specified render target.
 *
 * @param renderTarget The Renderer object where the widget will be drawn.
 */
void TextWidget::draw(Renderer &renderTarget) {
  renderTarget.drawText(x, y, text.c_str(), textSize, col);
}

/**
 * Sets the colour of the text widget.
 *
 * @param newCol The new colour to set for the text widget.
 */
void TextWidget::setColour(uint16_t newCol) {
  col = newCol;
}
