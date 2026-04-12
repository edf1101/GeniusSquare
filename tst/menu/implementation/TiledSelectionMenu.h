/*
 * Created by Ed Fillingham on 28/07/2025.
 *
 * This class represents a tiled selection menu.
 * This is a menu where the user can select options from a grid of tiles.
*/

#ifndef LASERTAG25_TILEDSELECTIONMENU_H
#define LASERTAG25_TILEDSELECTIONMENU_H

#include <menu/Menu.h>
#include <string>
#include <utility>
#include <vector>
#include "hardware/image/ImageLoader.h"

/**
 * This class holds data for tile name, image then function OR menu to go to.
 * just wraps it nicely for the TiledSelectionMenu class to work with.
 */
class MenuTile {
public:
    MenuTile(std::string name, std::string _imgPath,int _imgW,int _imgH, Menu* nextMenu)
            : name(std::move(name)), imgPath(std::move(_imgPath)), imgH(_imgH), imgW(_imgW), nextMenu(nextMenu),
              nextFunction(nullptr), isClickToMenu(true) {}

    MenuTile(std::string name,  std::string _imgPath,int _imgW,int _imgH, const std::function<void(void)>& _nextFunction)
            : name(std::move(name)), imgPath(std::move(_imgPath)), imgH(_imgH), imgW(_imgW), nextMenu(nullptr),
              nextFunction(_nextFunction), isClickToMenu(false) {}

private:
    std::string name; // Name of the tile
    std::string imgPath; // Path to the image for the tile
    int imgW; // Width of the image
    int imgH; // Height of the image
    bool isClickToMenu;
   Menu* nextMenu; // Menu to go to when this tile is selected, if applicable
    std::function<void(void)> nextFunction; // Function to call when this tile is selected, if applicable

    friend class TiledSelectionMenu; // Allow TiledSelectionMenu to access private members
};


class TiledSelectionMenu : public Menu {
public:
    explicit TiledSelectionMenu(std::string title);
    void addTile(MenuTile tile); // Add a tile to the menu

    void setupWidgets(Display *display) override;


private:
    const int maxAllowedTiles = 6;
    std::string title;
    std::vector<MenuTile> tiles; // Vector to hold the tiles in the menu
};

#endif //LASERTAG25_TILEDSELECTIONMENU_H
