/*
 * Created by Ed Fillingham on 28/07/2025.
*/

#include "Button.h"
#include <utility>

/**
 * Constructor for the Button class.
 *
 * @param pin The pin number to which the button is connected.
 * @param gpioExpander If true, indicates that the button is connected to a GPIO expander (like MCP23008).
 * @param mcp if using a GPIO expander, a pointer to the MCP23X08 instance.
 */
Button::Button(uint8_t pin, bool gpioExpander, Adafruit_MCP23X08 *mcp) {
  this->pin = pin;
  this->extendedGPIO = gpioExpander;
  this->mcp = mcp;
}

/**
 * Initialises the button by setting the pin mode.
 */
void Button::init() {

  // set the pinMode based on whether we are using a GPIO expander or not
  if (extendedGPIO) {
    // If using a GPIO expander, set the pin mode accordingly

  } else {
    // If not using a GPIO expander, set the pin mode directly
    pinMode(pin, INPUT);
  }
}

/**
 * Sets the pressed callback for the button.
 *
 * @param callback The function to call when the button is pressed.
 */
void Button::SetPressedCallback(std::function<void(void)> callback) {
  // Set the callback function for when the button is pressed
  this->pressedCallback = std::move(callback);
  this->setPressedCallback = true;
}

/**
 * Sets the released callback for the button.
 *
 * @param callback The function to call when the button is released.
 */
void Button::SetReleasedCallback(std::function<void(void)> callback) {
  // Set the callback function for when the button is released
  this->releasedCallback = std::move(callback);
  this->setReleasedCallback = true;
}

/**
 * Polls the button to check its state and trigger callbacks if necessary.
 */
void Button::poll() {
  // Poll the button to see if it has been pressed or released

  // read the pin mode using diff functions depending on if we are using extended GPIO or not
  bool currentState;
  if (extendedGPIO) {
    currentState = false;
  } else {
    currentState = digitalRead(pin);
  }

  if (currentState != lastState) {
    lastState = currentState;
    lastActivity = millis();
    if (currentState == HIGH) {

      if (setPressedCallback) {
        pressedCallback();
      }
    } else {
      if (setReleasedCallback) {
        releasedCallback();
      }
    }
  }
}

/**
 * Checks if the button is currently pressed.
 *
 * @return True if the button is pressed, false otherwise.
 */
bool Button::isPressed() const {
  // Return whether the button is currently pressed

  bool currentState;
  if (extendedGPIO) {
    currentState = false;
  } else {
    currentState = digitalRead(pin);
  }

  return currentState == HIGH;
}

/**
 * Gets the last activity time of the button.
 *
 * @return The last time the button was pressed or released, in milliseconds since the system started.
 */
unsigned long Button::getLastActivity() const {
  // Return the last time the button was pressed or released

  return lastActivity;
}
