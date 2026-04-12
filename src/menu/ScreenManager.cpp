/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Manages the application screen stack.
 */

#include "menu/ScreenManager.h"

ScreenManager::ScreenManager() : _depth(0) {
    for (uint8_t i = 0; i < MAX_DEPTH; i++) _stack[i] = nullptr;
}

/**
 * @brief Push a screen onto the stack and call its onEnter().
 */
void ScreenManager::push(Screen* screen) {
    if (_depth >= MAX_DEPTH) return;
    _stack[_depth++] = screen;
    screen->onEnter();
}

/**
 * @brief Pop the top screen and resume the one below.
 */
void ScreenManager::pop() {
    if (_depth == 0) return;
    _stack[--_depth]->onExit();
    _stack[_depth] = nullptr;
    if (_depth > 0) _stack[_depth - 1]->onResume();
}

void ScreenManager::onEncoderChange(int delta) {
    if (_depth > 0) _stack[_depth - 1]->onEncoderChange(delta);
}

void ScreenManager::onButtonPress() {
    if (_depth > 0) _stack[_depth - 1]->onButtonPress();
}

void ScreenManager::update() {
    if (_depth > 0) _stack[_depth - 1]->update();
}

void ScreenManager::render() {
    if (_depth > 0) _stack[_depth - 1]->render();
}

bool ScreenManager::hasScreen() const {
    return _depth > 0;
}
