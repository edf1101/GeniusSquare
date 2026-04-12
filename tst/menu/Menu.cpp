/*
 * Created by Ed Fillingham on 28/07/2025.
 *
 * This class represents a base class for menus in the laser tag system.
 * a menu is simply a page with a collection of widgets that can be interacted with
 * and swapped by the MenuManager.
*/

#include "Menu.h"
#include <hardware/displays/Display.h>

/**
 * Called when the rotary encoder is turned.
 *
 * @param change The change in the rotary encoder position.
 */
void Menu::onRotaryTurned(int change) {
  int maxCounter = (int) clickableWidgets.size();
  rotaryCounter = (rotaryCounter + change + maxCounter) % maxCounter; // add the change to the rotary counter

  // make sure the selected menu is now highlighted
  for (int i = 0; i < clickableWidgets.size(); i++) {
    if (i == rotaryCounter) {
      clickableWidgets[i]->setHighlightState(true); // Set the current widget as selected
    } else {
      clickableWidgets[i]->setHighlightState(false); // Deselect other widgets
    }
  }
}

/**
 * Called when the rotary encoder button is pressed.
 */
void Menu::onRotaryPressed() {
  // call the click function for the currently selected widget
  if (rotaryCounter < clickableWidgets.size()) {
    clickableWidgets[rotaryCounter]->onClick(); // Call the click function of the currently selected widget
  }
}

/**
 * Check if the menu requires refreshing.
 *
 * @return True if the menu requires refreshing, false otherwise.
 */
bool Menu::getRequiresRefresh() const {
  return requiresRefresh;
}


/**
 * Set the parent menu
 *
 * @param parent Pointer to the parent menu. If nullptr, this menu will be a top-level menu.
 */
void Menu::setParentMenu(Menu *parent) {
  this->parentMenu = parent; // Set the parent menu pointer to the provided parent
}

/**
 * Function to check if the menu requires admin privileges.
 */
bool Menu::getRequiresAdmin() const {
  return requiresAdmin; // Return the requiresAdmin flag
}

/**
 * Get the parent menu of this menu.
 *
 * @return Pointer to the parent menu, or nullptr if this is a top-level menu.
 */
Menu *Menu::getParentMenu() const {
  return parentMenu;
}

/**
 * Set whether this menu requires admin privileges.
 *
 * @param requiresAdmin Boolean flag indicating if admin privileges are required.
 */
void Menu::setRequiresAdmin(bool requiresAdmin) {
  this->requiresAdmin = requiresAdmin; // Set the requiresAdmin flag
}

/**
 * Reset the menu
 */
void Menu::resetMenu() {
  rotaryCounter = 0;
  onRotaryTurned(0); // Reset the rotary counter and highlight the first widget
}

/**
 * Remove all widgets from the display.
 *
 * @param display Pointer to the display from which widgets will be removed.
 */
void Menu::removeWidgets(Display *display) {
  // clear the clickable widgets vector but don't remove from display or delete them

  clickableWidgets.clear();
  // remove the clickable widgets from the widgets list and delete them
  for (Widget *widget: widgets) {
    display->removeWidget(widget); // Remove the widget from the display
    delete widget; // Delete the widget to free memory
  }
  widgets.clear(); // Clear the widgets vector
}