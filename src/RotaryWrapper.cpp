/*
* Created by Ed Fillingham on 05/07/2024.
*
* Provides a simple interface around ESP32Encoder for rotary input callbacks.
*/

#include "RotaryWrapper.h"

#include <utility>

/**
 * Constructor for the RotaryEncoderWrapper class.
 */
RotaryWrapper::RotaryWrapper(uint8_t inputA, uint8_t inputB) {
// constructor for the RotaryEncoderWrapper class

  myEncoder.attachHalfQuad(inputA, inputB);
}

/**
 * Call the callback function with the given value.
 *
 * @param value The value to pass to the callback function, typically the change in count.
 */
void RotaryWrapper::callCallbackFunc(long value) {
  // call the callback function

  if (callbackFuncSet) {
    lastActivity = millis(); // set the last activity time
    realCallbackFunc(value);
  }
}

/** * Get the last activity time of the rotary encoder.
 *
 * @return The last time the rotary encoder was turned, in milliseconds since the program started.
 */
unsigned long RotaryWrapper::getLastActivity() const {
  return lastActivity;
}

/**
 * Set the callback function to be called when the rotary encoder changes.
 *
 * @param changeCallback The function to call when the rotary encoder changes.
 */
void RotaryWrapper::setCallbackFunc(std::function<void(long)> changeCallback) {
  // set the callback function

  callbackFuncSet = true;
  realCallbackFunc = std::move(changeCallback);
}

/**
 * Poll the rotary encoder for changes.
 */
void RotaryWrapper::poll() {
  // poll the rotary encoder for changes

  int64_t count = myEncoder.getCount();
  int diff = (int)(lastCount - count);
  if (abs(diff) >= 2) {
    lastCount = count;
    diff /= 2;

    callCallbackFunc(diff);
    lastActivity = millis();
  }
}
