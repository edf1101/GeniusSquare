/*
 * Created by Ed Fillingham on 26/07/2025.
 *
 * This class represents a clickable widget on the display. Will be used for menus etc.
*/

#ifndef LASERTAG25_CLICKABLEWIDGET_H
#define LASERTAG25_CLICKABLEWIDGET_H

#include "hardware/displays/widgets/Widget.h"

class ClickableWidget : public Widget {

public:
    ClickableWidget(int x, int y, int width, int height);
    virtual void setHighlightState(bool highlight);

    virtual bool isHighlighted() const;

    virtual void onClick() {}; // Base function simply does nothing on click, decorators will override this
private:
    bool highlighted = false; // whether the widget is highlighted or not

};


#endif //LASERTAG25_CLICKABLEWIDGET_H
