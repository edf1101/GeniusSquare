/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Manages the application screen stack — owns navigation state and forwards
 * all input events and frame updates to the active (top-of-stack) screen.
 */

#include "menu/ScreenManager.h"

/**
 * @brief Construct a ScreenManager with an empty stack.
 */
ScreenManager::ScreenManager() : _depth(0) {
    for (uint8_t i = 0; i < MAX_DEPTH; i++) _stack[i] = nullptr;
}

/**
 * @brief Push a screen onto the stack and call its onEnter().
 *
 * onEnter() is responsible for drawing the initial frame and initialising
 * animation state. If the stack is full (depth == MAX_DEPTH), this is a no-op.
 *
 * @param screen Pointer to a Screen instance (must outlive this manager or until pop()).
 */
void ScreenManager::push(Screen* screen) {
    if (_depth >= MAX_DEPTH) return;
    _stack[_depth++] = screen;
    screen->onEnter();
}

/**
 * @brief Pop the top screen from the stack and resume the one below.
 *
 * Calls onExit() on the popped screen, then onResume() on the screen that
 * is now on top. onResume() typically redraws static regions without full reinit.
 * If the stack is empty, this is a no-op.
 */
void ScreenManager::pop() {
    if (_depth == 0) return;
    _stack[--_depth]->onExit();
    _stack[_depth] = nullptr;
    if (_depth > 0) _stack[_depth - 1]->onResume();
}

/**
 * @brief Forward an encoder delta (signed step count) to the active screen.
 *
 * @param delta Signed encoder change (positive = CW, negative = CCW).
 */
void ScreenManager::onEncoderChange(int delta) {
    if (_depth > 0) _stack[_depth - 1]->onEncoderChange(delta);
}

/**
 * @brief Forward a button press event to the active screen.
 *
 * Called when the button is debounced and released (falling edge detected).
 */
void ScreenManager::onButtonPress() {
    if (_depth > 0) _stack[_depth - 1]->onButtonPress();
}

/**
 * @brief Update the active screen's animation state.
 *
 * Called every loop iteration to advance time-based animations (carousel lerp,
 * border wave phase, etc.). The screen is responsible for tracking delta-time
 * internally and setting a dirty flag if redraw is needed.
 */
void ScreenManager::update() {
    if (_depth > 0) _stack[_depth - 1]->update();
}

/**
 * @brief Render the active screen to the TFT.
 *
 * Called every loop iteration after update(). The screen checks its dirty flag
 * and only draws if needed. Optimised screens minimise SPI traffic by only
 * redrawing changed regions.
 */
void ScreenManager::render() {
    if (_depth > 0) _stack[_depth - 1]->render();
}

/**
 * @brief Check if at least one screen is on the stack.
 *
 * @return true if _depth > 0, false if stack is empty.
 */
bool ScreenManager::hasScreen() const {
    return _depth > 0;
}
