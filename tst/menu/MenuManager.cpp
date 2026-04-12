/*
 * Created by Ed Fillingham on 28/07/2025.
*/

#include "MenuManager.h"
#include "hardware/displays/widgets/clickable/ClickableTile.h"
#include <hardware/image/ImageLoader.h>

/** Initialize the MenuManager with a display.
 *
 * @param _display Pointer to the display where menus will be shown.
 */
void MenuManager::init(Display *_display) {
  display = _display;
  setupMenus();

  // load the initial menu
  currentMenu = mainMenu; // Set the current menu to the new menu
  currentMenu->setupWidgets(display); // Set up widgets for the new menu
  currentMenu->resetMenu(); // Reset the new menu to its initial state
  display->updateDisplay();
}

/**
 * This function sets up all the menus
 */
void MenuManager::setupMenus() {

  // declare other menus here (not mainMenu)
  auto *menu2 = new TiledSelectionMenu("Menu 2");

  // add tiles to the menus here
  mainMenu->setParentMenu(nullptr); // Set the parent menu to nullptr for the main menu
  mainMenu->addTile(MenuTile{"Menu 2", "/img/hudAmmoIcon.bin",13,20, menu2});
  mainMenu->addTile(MenuTile{"Test2", "/img/hudAmmoIcon.bin",13,20, nullptr});
  mainMenu->addTile(MenuTile{"Test3", "/img/hudAmmoIcon.bin",13,20, nullptr});
  mainMenu->addTile(MenuTile{"Test4", "/img/hudAmmoIcon.bin",13,20, nullptr});
  mainMenu->addTile(MenuTile{"Test5", "/img/hudAmmoIcon.bin",13,20, [&]() { test1(); }}); // Print a message when the tile is clicked

  menu2->setParentMenu(mainMenu); // Set the parent menu for menu2
  menu2->addTile(MenuTile{"MT1", "/img/hudAmmoIcon.bin",13,20, [&]() {
      Serial.println("Test1 function called!");
  }}); // Print a message when the tile is clicked
  menu2->addTile(MenuTile{"MT2", "/img/hudAmmoIcon.bin",13,20, [&]() {
      Serial.println("Test2 function called!");
  }}); // Print a message when the tile is clicked
//    menu2->addTile(MenuTile{"Return", ammoImage, mainMenu}); // Print a message when the tile is clicked
}


/**
 * Get the current menu.
 *
 * @return Pointer to the current menu, or nullptr if no menu is set.
 */
Menu *MenuManager::getCurrentMenu() {
  return currentMenu;
}

/**
 * Called when the rotary encoder is turned.
 *
 * @param change how many increments the rotary encoder has been turned. (positive or negative)
 */
void MenuManager::onRotaryTurned(int change) {
  if (currentMenu != nullptr) {
    currentMenu->onRotaryTurned(change); // Call the onRotaryTurned method of the current menu
    display->updateDisplay();
  }
}

/**
 * Called when the rotary encoder button is pressed.
 */
void MenuManager::onRotaryPressed() {
  if (currentMenu != nullptr) {
    currentMenu->onRotaryPressed(); // Call the onRotaryTurned method of the current menu
    display->updateDisplay();
  }
}

/**
 * If possible it swaps into a new Menu.
 *
 * @param newMenu pointer to the new Menu to swap into.
 */
void MenuManager::swapMenu(Menu *newMenu) {
  if (newMenu == nullptr || currentMenu == newMenu)
    return;

  bool amAdmin = true; // temporary variable to check if the user is an admin
  if (newMenu->getRequiresAdmin() && !amAdmin) {
    return; // If the new menu requires admin privileges and the user is not an admin, do not swap
  }
  currentMenu->removeWidgets(display); // Remove widgets from the current menu

  currentMenu = newMenu; // Set the current menu to the new menu

  currentMenu->setupWidgets(display); // Set up widgets for the new menu
  currentMenu->resetMenu(); // Reset the new menu to its initial state
  display->updateDisplay(); // Update the display to reflect the new menu

}



