/*
 * Created by Ed Fillingham on 25/07/2025.
*/

#ifndef LASERTAG25_IMAGEWIDGET_H
#define LASERTAG25_IMAGEWIDGET_H

#include "hardware/displays/widgets/Widget.h"
#include "hardware/image/Image.h"

class ImageWidget : public Widget {
public:
    ImageWidget(int x, int y, Image* _img);
    void draw(Renderer &renderTarget) override;

private:
    Image* img;
};


#endif //LASERTAG25_IMAGEWIDGET_H
