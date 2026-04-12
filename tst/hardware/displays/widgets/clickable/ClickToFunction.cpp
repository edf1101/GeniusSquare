/*
 * Created by Ed Fillingham on 27/07/2025.
*/

#include "ClickToFunction.h"

/**
 * Call the function when the widget is clicked.
 */
void ClickToFunction::onClick() {
  onClickFunc();
}

/**
 * Set the function to call when the widget is clicked.
 *
 * @param _onClickFunc the function to set
 */
void ClickToFunction::setFunc(std::function<void()> cb) {
  onClickFunc = std::move(cb);
}
