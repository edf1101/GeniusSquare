/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Provides a debounced button input with a callback interface,
 * mirroring the RotaryWrapper pattern.
 */

#include "input/ButtonWrapper.h"

/**
 * @brief Construct a ButtonWrapper and configure the pin.
 */
ButtonWrapper::ButtonWrapper(uint8_t pin, std::function<void()> onPress)
    : _pin(pin), _lastPressMs(0), _onPress(onPress) {
    pinMode(_pin, INPUT);
    _lastState = digitalRead(_pin);
}

/**
 * @brief Poll the button. Fires onPress on a debounced falling edge (HIGH → LOW).
 */
void ButtonWrapper::poll() {
    bool currentState = digitalRead(_pin);
    unsigned long now = millis();

    if (_lastState == HIGH && currentState == LOW
            && (now - _lastPressMs) > DEBOUNCE_MS) {
        _lastPressMs = now;
        if (_onPress) _onPress();
    }

    _lastState = currentState;
}
