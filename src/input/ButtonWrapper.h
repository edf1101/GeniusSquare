/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Provides a debounced button input with short-press and long-press callback interface.
 */

#ifndef BUTTON_WRAPPER_H
#define BUTTON_WRAPPER_H

#include <Arduino.h>
#include <functional>

class ButtonWrapper {
public:
    ButtonWrapper(uint8_t pin, std::function<void()> onRelease, std::function<void()> onLongRelease = nullptr);
    unsigned long getLastActivity() const;
    void poll();

private:
    uint8_t _pin;
    bool _lastState;
    bool _isPressed;
    unsigned long _pressStartTime;
    unsigned long _lastTransitionTime;
    unsigned long _lastReleaseTime;

    std::function<void()> _onRelease;
    std::function<void()> _onLongRelease;

    static constexpr unsigned long DEBOUNCE_MS = 50;
    static constexpr unsigned long RELEASE_GUARD_MS = 100;
    static constexpr unsigned long LONG_PRESS_MS = 800;

};

#endif // BUTTON_WRAPPER_H
