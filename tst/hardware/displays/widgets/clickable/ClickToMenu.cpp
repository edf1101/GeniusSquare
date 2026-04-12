/*
 * Created by Ed Fillingham on 27/07/2025.
 *
 * This file is a concrete decorator for a ClickableWidget that allows it to
 * move to a menu when clicked.
*/

#include "ClickToMenu.h"
#include <LaserTag.h>

/**
 * Constructor for ClickToMenu.
 *
 * @param widget the ClickableWidget to decorate
 * @param menuId  the ID of the menu to navigate to when clicked
 */
ClickToMenu::ClickToMenu(ClickableWidget *widget, Menu* menuTarget) :
        ClickableWidgetDecorator(widget), menu(menuTarget) {
}

/**
 * Set the menu ID to navigate to when clicked.
 *
 * @param menuId the ID of the menu to navigate to
 */
void ClickToMenu::setMenu(Menu* menuTarget) {
  this->menu = menuTarget;
}

/**
 * This gets called when the onClick event is triggered.
 */
void ClickToMenu::onClick() {
  LaserTag::getInstance()->getMenuManager().swapMenu(menu);
}
