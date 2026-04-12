/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Provides a debounced button input with a callback interface,
 * mirroring the RotaryWrapper pattern.
 */

#ifndef BUTTON_WRAPPER_H
#define BUTTON_WRAPPER_H

#include <Arduino.h>
#include <functional>

class ButtonWrapper {
public:
    /**
     * @brief Construct a ButtonWrapper.
     * @param pin     GPIO pin the button is connected to (active LOW).
     * @param onPress Callback fired on a debounced falling edge.
     */
    ButtonWrapper(uint8_t pin, std::function<void()> onPress);

    /**
     * @brief Poll the button state. Call once per loop iteration.
     */
    void poll();

private:
    uint8_t _pin;
    bool _lastState;
    unsigned long _lastPressMs;
    std::function<void()> _onPress;

    static constexpr unsigned long DEBOUNCE_MS = 50;
};

#endif // BUTTON_WRAPPER_H
