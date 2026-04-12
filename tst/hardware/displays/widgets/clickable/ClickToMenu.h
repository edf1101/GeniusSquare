/*
 * Created by Ed Fillingham on 27/07/2025.
 *
 * This file is a concrete decorator for a ClickableWidget that allows it to
 * move to a menu when clicked.
*/

#ifndef LASERTAG25_CLICKTOMENU_H
#define LASERTAG25_CLICKTOMENU_H

#include "ClickableWidgetDecorator.h"
class Menu; // Forward declaration of Menu class
class ClickToMenu : public ClickableWidgetDecorator{
public:
    ClickToMenu(ClickableWidget *widget, Menu* menuTarget);
    void onClick() override;

    void setMenu(Menu* menuTarget);
private:
    Menu* menu;
};


#endif //LASERTAG25_CLICKTOMENU_H
