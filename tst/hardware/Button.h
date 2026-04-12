/*
 * Created by Ed Fillingham on 28/07/2025.
 *
 * This code manages a button interface, it allows you to poll, attach callbacks for
 * on press and release events, and monitor last activity.
*/

#ifndef LASERTAG25_BUTTON_H
#define LASERTAG25_BUTTON_H

#include <Arduino.h>

class Adafruit_MCP23X08; // Forward declaration of the MCP23X08 class
class Button {
public:
    explicit Button(uint8_t pin, bool gpioExpander = false, Adafruit_MCP23X08 *mcp = nullptr);

    void init();

    void SetPressedCallback(std::function<void(void)> callback);

    void SetReleasedCallback(std::function<void(void)> callback);

    void poll();

    bool isPressed() const;

    unsigned long getLastActivity() const;

private:
    int pin;

    Adafruit_MCP23X08 *mcp = nullptr; // Pointer to the MCP23X08 instance if using GPIO expander
    bool extendedGPIO;

    bool lastState = false;

    std::function<void(void)> pressedCallback;

    bool setPressedCallback = false;

    std::function<void(void)> releasedCallback;

    bool setReleasedCallback = false;

    unsigned long lastActivity = 0; // The last time the button was pressed or released

};


#endif //LASERTAG25_BUTTON_H
