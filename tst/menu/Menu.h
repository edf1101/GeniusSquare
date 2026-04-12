/*
 * Created by Ed Fillingham on 28/07/2025.
 *
 * This class represents a base class for menus in the laser tag system.
 * a menu is simply a page with a collection of widgets that can be interacted with
 * and swapped by the MenuManager.
*/

#ifndef LASERTAG25_MENU_H
#define LASERTAG25_MENU_H

#include <Arduino.h>
#include <vector>
#include <hardware/displays/widgets/Widget.h>
#include <hardware/displays/widgets/clickable/ClickableWidget.h>

class Display;


class Menu {
public:
    Menu() = default; // Default constructor
    virtual ~Menu() = default; // Default destructor

    virtual void setupWidgets(Display *display) = 0; // Pure virtual function to set up widgets on the display
    virtual void removeWidgets(Display *display); // Pure virtual function to remove widgets from the display

    void setParentMenu(Menu *parent);

    Menu *getParentMenu() const;

    void setRequiresAdmin(bool requiresAdmin);

    bool getRequiresAdmin() const;
    bool getRequiresRefresh() const;

    virtual void resetMenu();
    virtual void onRotaryTurned(int change); // called when the rotary encoder is turned
    virtual void onRotaryPressed(); // called when the button is pressed


protected:
    std::vector<Widget *> widgets; // Vector to hold pointers to widgets associated with this menu
    std::vector<ClickableWidget *> clickableWidgets; // Vector to hold pointers to clickable widgets associated with this menu

    Menu *parentMenu = nullptr; // Pointer to the parent menu, if any

    bool requiresAdmin = false; // Flag to indicate if the menu requires admin privileges
    bool requiresRefresh = false; // Flag to indicate if the menu requires refreshing every so often ie leaderboard

    int rotaryCounter = 0; // The current counter on the rotary encoder
};


#endif //LASERTAG25_MENU_H
