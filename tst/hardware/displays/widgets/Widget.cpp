/*
 * Created by Ed Fillingham on 25/07/2025.
*/

#include "Widget.h"

/**
 * Erase the widget from the render target.
 *
 * @param renderTarget the renderer to erase the widget from
 */
void Widget::erase(Renderer &renderTarget) {
  renderTarget.drawFilledRect(x, y, width, height, renderTarget.getBackgroundColour());
  oldHash = 0;
}

/**
 * Update the widget if it has changed.
 *
 * @param renderTarget the renderer to update the widget on
 */
void Widget::updateWidget(Renderer &renderTarget, bool force) {

  if (force)
    oldHash = 1;
  std::size_t newHash = 0;
  for (auto &hf: hashFields) hf(newHash);

  if (newHash != oldHash) {
    erase(renderTarget); // erase the old widget content
    draw(renderTarget); // call the subclass’s draw
    oldHash = newHash; // remember for next time
  }
}
