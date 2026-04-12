/*
 * Created by Ed Fillingham on 29/07/2025.
*/

#ifndef LASERTAG25_TEXTWIDGET_H
#define LASERTAG25_TEXTWIDGET_H

#include "hardware/displays/widgets/Widget.h"
#include <string>

class TextWidget : public Widget  {
public:
    TextWidget(int x, int y, int width, int height, std::string text, int textSize,uint16_t col);

    void setText(const std::string &newText);

    void setColour(uint16_t newCol);

    void draw(Renderer &renderTarget) override;
private:
    std::string text;
    int textSize;
    uint16_t col;
};


#endif //LASERTAG25_TEXTWIDGET_H
