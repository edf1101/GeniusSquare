/*
 * Created by Ed Fillingham on 28/07/2025.
 *
 * This class represents a tiled selection menu.
 * This is a menu where the user can select options from a grid of tiles.
*/

#include "TiledSelectionMenu.h"
#include <hardware/displays/widgets/implementations/TextWidget.h>
#include <hardware/displays/Display.h>

#include <hardware/displays/widgets/clickable/ClickableWidget.h>
#include <hardware/displays/widgets/clickable/ClickableTile.h>
#include <hardware/displays/widgets/clickable/ClickToFunction.h>
#include <hardware/displays/widgets/clickable/ClickToMenu.h>


/**
 * Constructor for the TiledSelectionMenu class.
 *
 * @param title The title of the menu.
 */
TiledSelectionMenu::TiledSelectionMenu(std::string title)
        : Menu(), title(std::move(title)) {

}

/**
 * Add a tile to the menu.
 *
 * @param tile The tile to be added to the menu.
 */
void TiledSelectionMenu::addTile(MenuTile tile) {
  int currentTileCount = tiles.size();
  bool haveParent = parentMenu != nullptr;

  if (currentTileCount + (haveParent ? 1 : 0) >= maxAllowedTiles) {
    return; // Prevent adding more tiles than allowed
  }

  tiles.push_back(std::move(tile));
}

/**
 * Set up the widgets for the tiled selection menu.
 *
 * @param display pointer to the display where the widgets will be added
 */
void TiledSelectionMenu::setupWidgets(Display *display) {

  int tileWidth = 48; // Width of each tile
  int tileHeight = 45; // Height of each tile
  int tileSpacingX = 3; // Spacing between tiles in X
  int tileSpacingY = 5; // Spacing between tiles in Y
  int tileOffsetX = 5; // how far from the left edge of the screen the first tile is
  int tileOffsetY = 30; // how far from the top edge of the screen the first tile is

  // get tile count
  int currentTileCount = tiles.size();
  bool haveParent = parentMenu != nullptr;
  currentTileCount = currentTileCount + (haveParent ? 1 : 0);

  for (int idx = 0; idx < currentTileCount - (haveParent ? 1 : 0); idx++) {
    // get the tile
    MenuTile &tileData = tiles[idx];

    int row = ((idx + 3) / 3) - 1;
    int col = idx % 3;
    int tileX = tileOffsetX + (col * (tileWidth + tileSpacingX));
    int tileY = tileOffsetY + (row * (tileHeight + tileSpacingY));

    // create a clickable widget for the tile
    ClickableWidget *baseTile = new ClickableTile(tileX, tileY,
                                                  tileWidth, tileHeight,
                                                  tileData.name, ImageLoader(tileData.imgPath,
                                                                              tileData.imgW, tileData.imgH));

    // set the click function or next menu
    ClickableWidget *completeTile;
    if (tileData.isClickToMenu) {
      completeTile = new ClickToMenu(baseTile, tileData.nextMenu);
    } else {
      completeTile = new ClickToFunction(baseTile, tileData.nextFunction);
    }
    // add the widget to the display
    display->addWidget(completeTile);
    clickableWidgets.push_back(completeTile); // Store the clickable widget for later use
    widgets.push_back(completeTile); // Store the clickable widget for later use

  }

  if (haveParent) {
    // create a clickable widget for the parent menu
    int row = (((int) tiles.size() + 3) / 3) - 1;
    int col = (int) tiles.size() % 3;
    int tileX = tileOffsetX + (col * (tileWidth + tileSpacingX));
    int tileY = tileOffsetY + (row * (tileHeight + tileSpacingY));

    // create a clickable widget for the tile
    ClickableWidget *baseTile = new ClickToMenu(new ClickableTile(tileX, tileY,
                                                                  tileWidth, tileHeight,
                                                                  "Return",
                                                                  ImageLoader("/img/returnBig.bin",
                                                                              45, 33)),
                                                parentMenu);
    // add the widget to the display
    display->addWidget(baseTile);
    clickableWidgets.push_back(baseTile); // Store the clickable widget for later use
    widgets.push_back(baseTile); // Store the clickable widget for later use

  }

  // add the title of the menu at the top
  int textWidth = display->getTextWidth(title.c_str(), 2);
  int titleX = (display->getDisplayWidth() - textWidth) / 2; // Center the title
  Widget *titleWidget = new TextWidget(titleX, 5, textWidth + 5, 20, title.c_str(), 2, display->getHighlightColour());
  display->addWidget(titleWidget);
  widgets.push_back(titleWidget); // Store the title widget for later use


}
