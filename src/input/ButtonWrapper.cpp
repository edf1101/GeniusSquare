/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Provides a debounced button input with short-press and long-press callback interface.
 */

#include "ButtonWrapper.h"

ButtonWrapper::ButtonWrapper(uint8_t pin, std::function<void()> onRelease, std::function<void()> onLongRelease)
    : _pin(pin), _onRelease(onRelease), _onLongRelease(onLongRelease), 
      _isPressed(false), _pressStartTime(0), _lastTransitionTime(0) {

    // Reverted to INPUT for Active HIGH (assumes you have a physical pull-down resistor)
    pinMode(_pin, INPUT);
    _lastState = digitalRead(_pin);
}

void ButtonWrapper::poll() {
    bool currentState = digitalRead(_pin);
    unsigned long now = millis();

    // Only process state changes that happen after the debounce window
    if (currentState != _lastState && (now - _lastTransitionTime) >= DEBOUNCE_MS) {
        _lastTransitionTime = now;
        _lastState = currentState;

        if (currentState == HIGH) {
            // Button went HIGH (Just Pressed)
            _pressStartTime = now;
            _isPressed = true;
        }
        else if (currentState == LOW && _isPressed) {
            // Button went LOW (Just Released)
            unsigned long duration = now - _pressStartTime;

            // Fire exactly ONE callback based on how long it was held
            if (_onLongRelease && duration >= LONG_PRESS_MS) {
                _onLongRelease();
            } else if (_onRelease) {
                _onRelease();
            }

            _isPressed = false;
        }
    }
}

unsigned long ButtonWrapper::getLastActivity() const {
    return _lastTransitionTime;
}
