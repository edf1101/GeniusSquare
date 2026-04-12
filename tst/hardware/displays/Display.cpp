/*
 * Created by Ed Fillingham on 25/07/2025.
 *
 * All other displays (HUD and sideDisplay classes implement the main display class.
 * This provides a similar interface that widgets can use to render to the display.
*/

#include "Display.h"
#include "hardware/displays/widgets/Widget.h"


/**
 * Clear the widgets from the display.
 */
void Display::clearWidgets() {
// logic to erase each widgets content here

  widgets.clear();

}

/**
 * Draw all the widgets to the display.
 */
void Display::drawWidgets() {
  for (Widget *widget: widgets) {
    if (widget) {
      widget->updateWidget(*this);
    }
  }
}

/**
 * Add a widget to the display.
 *
 * @param widget Pointer to the widget to be added.
 */
void Display::addWidget(Widget *widget) {
  if (widget) {
    widgets.push_back(widget); // Add widget to the list
  }
}

void Display::removeWidget(Widget *widget) {
  if (widget) {
    widget->erase(*this); // Erase the widget from the display first
    auto it = std::find(widgets.begin(), widgets.end(), widget);
    if (it != widgets.end()) {
      widgets.erase(it); // Remove the widget from the list
    }
  }
  this->updateDisplay();

}
