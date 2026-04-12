/*
 * Created by Ed Fillingham on 27/07/2025.
 *
 * A clickable tile widget is a widget that is composed of a rectangle with an image within
 * and text below it. It doesn't the onClick logic in it but does implement rendering differently
 * depending on highlighted state.
*/

#include "ClickableTile.h"

#include <utility>

/**
 * Constructor for the ClickableTile widget.
 *
 * @param x the x coordinate of the tile
 * @param y the y coordinate of the tile
 * @param width the width of the tile
 * @param height the height of the tile
 * @param text the text to display below the image
 * @param _img the image to display in the tile
 */
ClickableTile::ClickableTile(int x, int y, int width, int height, std::string text, ImageLoader _imgLoader) :
        ClickableWidget(x, y, width, height), imgLoader(std::move(_imgLoader)), title(std::move(text)) {

  // Add the hash fields for the image and text
  registerHashField(imgLoader);
  registerHashField(title);
}

/**
 * Destructor for the ClickableTile widget.
 */
ClickableTile::~ClickableTile() {
  if(imgLoader.loadImage() == nullptr) {
    return; // nothing to delete
  }
//  if (img!= nullptr && img->getBitmap() != nullptr) {
//    delete img; // clean up the image
//
//  }

}


/**
 * Draws the clickable tile on the given render target.
 *
 * @param renderTarget the renderer to draw the tile on
 */
void ClickableTile::draw(Renderer &renderTarget) {

  // get the colour to draw
  uint16_t colour = isHighlighted() ? renderTarget.getHighlightColour() : renderTarget.getMainColour();

  // draw image surrounding box (-10 pixels to allow for text below)
  renderTarget.drawRect(x+1, y+1, width-2, height - 12, colour);
  if(isHighlighted()){ // if highlighted make the border thicker
    renderTarget.drawRect(x + 2, y + 2, width - 4, height - 14, colour);
    renderTarget.drawRect(x , y , width , height - 10, colour);
    // slightly dodgy rounded edges
    renderTarget.drawPixel(x, y, renderTarget.getBackgroundColour());
    renderTarget.drawPixel(x + width - 1, y, renderTarget.getBackgroundColour());
    renderTarget.drawPixel(x, y + height - 11, renderTarget.getBackgroundColour());
    renderTarget.drawPixel(x + width - 1, y + height - 11, renderTarget.getBackgroundColour());

    renderTarget.drawPixel(x+3, y+3, colour);
    renderTarget.drawPixel(x + width - 4, y + 3, colour);
    renderTarget.drawPixel(x+3, y + height - 14, colour);
    renderTarget.drawPixel(x + width - 4, y + height - 14, colour);


  }

  // draw the image in the center of the box
  int imgX = x + (width - imgLoader.loadImage()->getWidth()) / 2;
  int imgY = y + (height - imgLoader.loadImage()->getHeight() - 10) / 2;
  renderTarget.drawXBM(imgX, imgY, imgLoader.loadImage()->getBitmap(),
                       imgLoader.loadImage()->getWidth(), imgLoader.loadImage()->getHeight(), colour);

  // draw the text below the image, it should be centered 8 px above the bottom
  int textX = x + (width - renderTarget.getTextWidth(title.c_str(), 1)) / 2;
  int textY = y + height - 9; // 8 pixels above the bottom
  renderTarget.drawText(textX, textY, title.c_str(), 1, colour);
}


