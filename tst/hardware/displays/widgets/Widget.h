/*
 * Created by Ed Fillingham on 25/07/2025.
 *
 * Widget is the base class for all display widgets. It has a virtual draw method
 * which must be implemented by concrete widget classes.
*/

#ifndef LASERTAG25_WIDGET_H
#define LASERTAG25_WIDGET_H

#include "hardware/displays/Renderer.h"
#include "utils/HashHelpers.h"
#include <vector>
class ClickableWidgetDecorator;      // forward declaration
class Widget {
    friend class ClickableWidgetDecorator; // allow decorators to access protected members
public:
    Widget(int x, int y, int width, int height)
            : x(x), y(y), width(width), height(height) {}

    virtual ~Widget() = default;

    void erase(Renderer &renderTarget);

    void updateWidget(Renderer &renderTarget,bool force = false);

protected:
    const int x; // X position of the widget
    const int y = 0; // Y position of the widget
    int width = 0; // Width of the widget
    int height = 0; // Height of the widget

    virtual void draw(Renderer &renderTarget) = 0;

    // needs to be here rather than source code as it is template
    /**
     * Registers a field to be included in the widget's hash.
     *
     * @tparam T the type of the field, which must be hashable
     * @param field the field to register, passed by reference
     */
    template<typename T>
    void registerHashField(const T& field) {
      hashFields.emplace_back(
              // capture the field by reference so we always see its latest value
              [&field](std::size_t &seed){
                  hash_combine(seed, field);
              });
    }

    // protected so subclass ctor can call it:

private:
    // called by subclasses to register one more field to be hashed

    std::vector<std::function<void(std::size_t&)>> hashFields;

    std::size_t oldHash = 1; // cant be 0 as if no registered fields hash will always be 0
};


#endif //LASERTAG25_WIDGET_H
