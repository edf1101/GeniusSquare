/*
 * Created by Ed Fillingham on 26/07/2025.
 *
 * This class represents a clickable widget on the display. Will be used for menus etc.
*/

#include "ClickableWidget.h"

/**
 * Constructor for the ClickableWidget class.
 *
 * @param x x position of the widget
 * @param y y position of the widget
 * @param width the width of the widget
 * @param height the height of the widget
 */
ClickableWidget::ClickableWidget(int x, int y, int width, int height) :
        Widget(x, y, width, height) {
  registerHashField(highlighted);
}

/**
 * Set the highlight state of the widget.
 *
 * @param highlight true to highlight the widget, false to remove highlight
 */
void ClickableWidget::setHighlightState(bool highlight) {
  this->highlighted = highlight;
}

/**
 * Check if the widget is highlighted.
 *
 * @return true if the widget is highlighted, false otherwise
 */
bool ClickableWidget::isHighlighted() const {
  return highlighted;
}
