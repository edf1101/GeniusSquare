/*
 * Created by Ed Fillingham on 25/07/2025.
*/

#include "ImageWidget.h"

/**
 * Constructor for the ImageWidget class.
 *
 * @param x The x coordinate of the widget's position.
 * @param y The y coordinate of the widget's position.
 * @param _img The Image object to be displayed by this widget.
 */
ImageWidget::ImageWidget(int x, int y, Image *_img) :
        Widget(x, y, _img->getWidth(), _img->getHeight()),
        img(_img) {

  registerHashField(*img);// Register the image data for hashing
}

/**
 * Draws the image widget on the specified render target.
 *
 * @param renderTarget The Renderer object where the widget will be drawn.
 */
void ImageWidget::draw(Renderer &renderTarget) {
  renderTarget.drawXBM(x, y,
                       img->getBitmap(),
                       img->getWidth(),
                       img->getHeight(),
                       renderTarget.getHighlightColour());
}


