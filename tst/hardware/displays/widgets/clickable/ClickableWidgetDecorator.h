/*
 * Created by Ed Fillingham on 27/07/2025.
 *
 * This class is the abstract base class for clickable widgets that can be decorated.
*/

#ifndef LASERTAG25_CLICKABLEWIDGETDECORATOR_H
#define LASERTAG25_CLICKABLEWIDGETDECORATOR_H

#include "ClickableWidget.h"

class ClickableWidgetDecorator : public ClickableWidget {
public:
    explicit ClickableWidgetDecorator(ClickableWidget *widget) : ClickableWidget(*widget), innerClass(widget) {};

    /**
     * Destructor for ClickableWidgetDecorator.
     * This will delete the inner class pointer to avoid memory leaks.
     */
    ~ClickableWidgetDecorator() override {
      delete innerClass; // Delete the inner class pointer
    }

    virtual void onClick() override = 0; // Force this to be overriden again
    void draw(Renderer &r) override { innerClass->draw(r); }

    void erase(Renderer &r) { innerClass->erase(r); }

    void updateWidget(Renderer &r, bool f = false) { innerClass->updateWidget(r, f); }

    void setHighlightState(bool h) { innerClass->setHighlightState(h); }

    bool isHighlighted() const { return innerClass->isHighlighted(); }

private:
    ClickableWidget *innerClass; // Pointer to the ClickableWidget being decorated
};


#endif //LASERTAG25_CLICKABLEWIDGETDECORATOR_H
