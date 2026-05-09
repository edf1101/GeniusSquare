/*
 * Created by Ed Fillingham on 05/07/2024.
 *
 * Thin wrapper around ESP32Encoder (PCNT-based counter) that provides
 * delta-based callback polling. Detects rotary encoder half-quad ticks
 * and fires a callback with signed step deltas (±1 per detent click).
 */

#ifndef ROTARY_ROTARY_H
#define ROTARY_ROTARY_H

#include "Arduino.h"
#include <ESP32Encoder.h>
#include <functional>

/**
 * @brief Rotary encoder input handler using ESP32-S3 PCNT hardware.
 *
 * Encapsulates ESP32Encoder and implements a polling interface. Call poll()
 * each loop iteration to check for step changes and fire the registered callback.
 *
 * Delta convention: positive = clockwise, negative = counter-clockwise.
 * The callback is fired once per detent click (±1 delta).
 */
class RotaryWrapper {
public:
    /**
     * @brief Construct a RotaryWrapper on two GPIO pins.
     * @param inputA First encoder pin (CLK).
     * @param inputB Second encoder pin (DT).
     */
    RotaryWrapper(uint8_t inputA, uint8_t inputB);

    /**
     * @brief Register a callback to fire when encoder steps are detected.
     * @param changeCallback std::function<void(long)> receiving signed step delta.
     */
    void setCallbackFunc(std::function<void(long)> changeCallback);

    /**
     * @brief Get the timestamp of the last encoder activity.
     * @return Milliseconds since boot (millis()), or 0 if no activity yet.
     */
    unsigned long getLastActivity() const;

    /**
     * @brief Poll the PCNT counter and fire callback if delta detected.
     *
     * Must be called every loop iteration. Debounces at 2 half-quad ticks
     * (1 detent step) and scales delta accordingly.
     */
    void poll();

private:
    /// PCNT-based hardware counter instance
    ESP32Encoder myEncoder;

    /// User-provided callback function
    std::function<void(long)> realCallbackFunc;

    /// Flag indicating callback is registered and ready
    bool callbackFuncSet = false;

    /**
     * @brief Internal helper — fire callback if it's set.
     * @param value Signed step delta to pass to callback.
     */
    void callCallbackFunc(long value);

    /// Timestamp of most recent encoder activity (for idle detection)
    unsigned long lastActivity = 0;

    /// Previous PCNT counter value (used to detect changes)
    int64_t lastCount = 0;
};

#endif // ROTARY_ROTARY_H
