/*
 * Created by Ed Fillingham on 05/07/2024.
 *
 * Wrapper around ESP32Encoder using the PCNT hardware counter.
 * Detects half-quad ticks and fires a callback with signed delta steps.
 * Call poll() each loop iteration to check for changes.
 */

#include "RotaryWrapper.h"
#include <utility>

/**
 * @brief Construct a RotaryWrapper on two GPIO pins.
 *
 * Attaches the encoder to the ESP32-S3's PCNT (pulse counter) hardware,
 * configured for half-quad mode (counts every half-step, 2 counts per detent).
 *
 * @param inputA First encoder pin (typically CLK).
 * @param inputB Second encoder pin (typically DT).
 * @param reverse Optional flag to reverse the direction of the delta (CW vs CCW). Default false.
 */
RotaryWrapper::RotaryWrapper(uint8_t inputA, uint8_t inputB, bool reverse) {
    myEncoder.attachHalfQuad(inputA, inputB);
    this->reverse = reverse;
}

/**
 * @brief Fire the registered callback with a signed step delta.
 *
 * Updates lastActivity timestamp before calling the callback.
 * No-op if callback is not set.
 *
 * @param value Signed delta in half-quad ticks (typically ±1 per poll).
 */
void RotaryWrapper::callCallbackFunc(long value) {
    if (callbackFuncSet) {
        lastActivity = millis();
        realCallbackFunc(value);
    }
}

/**
 * @brief Get the timestamp of the most recent encoder activity.
 *
 * Useful for idle timeout detection (e.g., turn off display after N seconds).
 *
 * @return Milliseconds since boot (from millis()), 0 if no activity yet.
 */
unsigned long RotaryWrapper::getLastActivity() const {
    return lastActivity;
}

/**
 * @brief Register a callback function to fire on encoder changes.
 *
 * The callback receives a signed delta value:
 *   - Positive: clockwise rotation
 *   - Negative: counter-clockwise rotation
 *
 * The callback is typically set to forward the delta to ScreenManager.
 *
 * @param changeCallback std::function<void(long)> callback to fire on delta change.
 */
void RotaryWrapper::setCallbackFunc(std::function<void(long)> changeCallback) {
    callbackFuncSet = true;
    realCallbackFunc = std::move(changeCallback);
}

/**
 * @brief Poll the PCNT counter for changes and fire callback if delta detected.
 *
 * Must be called every loop iteration. Reads the PCNT counter via myEncoder.getCount(),
 * compares to lastCount, and fires the callback if diff >= 2 half-quad ticks
 * (representing one detent click). Delta is halved before callback
 * (since PCNT counts 2 per detent in half-quad mode).
 *
 * Delta convention: positive = clockwise, negative = counter-clockwise.
 */
void RotaryWrapper::poll() {
    int64_t count = myEncoder.getCount();
    int diff = (int)(lastCount - count);

    // Only fire callback if change is >= 2 half-quad ticks (1 detent step).
    // This debounces noise and ensures we don't fire multiple times per detent.
    if (abs(diff) >= 2) {
        lastCount = count;
        diff /= 2;  // Convert from half-quad ticks to detent steps

        if (reverse) {
            diff = -diff;  // Optionally reverse direction if configured
        }

        callCallbackFunc(diff);
        lastActivity = millis();
    }
}
