/*
 * Created by Ed Fillingham on 27/07/2025.
 *
 * A clickable tile widget is a widget that is composed of a rectangle with an image within
 * and text below it. It doesn't the onClick logic in it but does implement rendering differently
 * depending on highlighted state.
*/

#ifndef LASERTAG25_CLICKABLETILE_H
#define LASERTAG25_CLICKABLETILE_H

#include "ClickableWidget.h"
#include "hardware/image/ImageLoader.h"

class ClickableTile : public virtual ClickableWidget {
public:
    ClickableTile(int x, int y, int width, int height, std::string text, ImageLoader _imgLoader);
    ~ClickableTile() override;

    void draw(Renderer &renderTarget) override;

private:
    ImageLoader imgLoader; // The image to display in the tile
    std::string title; // The text to display below the image
};


#endif //LASERTAG25_CLICKABLETILE_H
