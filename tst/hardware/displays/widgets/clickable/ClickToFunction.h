/*
 * Created by Ed Fillingham on 27/07/2025.
 *
 * This file is a concrete decorator for a ClickableWidget that allows it to
 * call a function when clicked.
*/

#ifndef LASERTAG25_CLICKTOFUNCTION_H
#define LASERTAG25_CLICKTOFUNCTION_H

#include "ClickableWidgetDecorator.h"
#include <functional>

class ClickToFunction : public ClickableWidgetDecorator {
public:
    /**
     * Constructor for ClickToFunction.
     *
     * @tparam Callable a callable type (lambda, function pointer, etc.)
     * @param widget  the ClickableWidget to decorate
     * @param cb The function to call when clicked, can be a lambda, function pointer, etc.
     */
    template<class Callable>
    ClickToFunction(ClickableWidget *widget, Callable &&cb)
            : ClickableWidgetDecorator(widget),
              onClickFunc(std::forward<Callable>(cb)) {}

    void onClick() override;

    void setFunc(std::function<void()> cb);

private:
    std::function<void()> onClickFunc;   // stores lambdas, binds, f‑ptrs, whatever
};

#endif //LASERTAG25_CLICKTOFUNCTION_H
