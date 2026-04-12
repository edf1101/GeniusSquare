/*
 * Created by Ed Fillingham on 28/07/2025.
 *
 * This class manages the Menu system
*/

#ifndef LASERTAG25_MENUMANAGER_H
#define LASERTAG25_MENUMANAGER_H

#include <hardware/displays/Display.h>
#include <Arduino.h>
#include "Menu.h"
#include "implementation/TiledSelectionMenu.h"


class MenuManager {
public:
    void init(Display *_display);

    Menu *getCurrentMenu();
    void swapMenu(Menu *newMenu); // swap the current menu with a new one

    void onRotaryTurned(int change); // called when the rotary encoder is turned
    void onRotaryPressed(); // called when the button is pressed

private:
    void setupMenus();

    Menu *currentMenu = nullptr;
    Display *display = nullptr; // Pointer to the display where menus are shown

    TiledSelectionMenu* mainMenu = new TiledSelectionMenu("Main Menu");

    void test1(){Serial.println("Test 1 function called!");};
};


#endif //LASERTAG25_MENUMANAGER_H
