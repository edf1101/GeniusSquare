/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Manages the application screen stack. Owns navigation (push/pop)
 * and forwards encoder and button events to the active screen.
 */

#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <Arduino.h>
#include "menu/Screen.h"

class ScreenManager {
public:
    ScreenManager();

    /**
     * @brief Push a screen onto the stack and call its onEnter().
     * @param screen Pointer to a Screen instance. Must remain valid for the screen's lifetime.
     */
    void push(Screen* screen);

    /**
     * @brief Pop the top screen (calls onExit()), then calls onResume() on the screen below.
     *        No-op if the stack is empty.
     */
    void pop();

    /**
     * @brief Forward a signed encoder delta to the active screen.
     */
    void onEncoderChange(int delta);

    /**
     * @brief Forward a button press to the active screen.
     */
    void onButtonPress();

    /**
     * @brief Call update() on the active screen.
     */
    void update();

    /**
     * @brief Call render() on the active screen.
     */
    void render();

    /**
     * @brief Returns true if at least one screen is on the stack.
     */
    bool hasScreen() const;

private:
    static constexpr uint8_t MAX_DEPTH = 8;
    Screen* _stack[MAX_DEPTH];
    uint8_t _depth;
};

#endif // SCREEN_MANAGER_H
