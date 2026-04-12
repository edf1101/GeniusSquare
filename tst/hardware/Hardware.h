/*
 * Created by Ed Fillingham on 25/07/2025.
*/

#ifndef LASERTAG25_HARDWARE_H
#define LASERTAG25_HARDWARE_H

#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include<hardware/displays/SideDisplay.h>
#include "RotaryWrapper.h"
#include <config.h>
#include "Button.h"
#include <SPI.h>
#include <Wire.h>


class Hardware {
public:
    Hardware();
    void init(); // set up the hardware
    void loop(); // run the hardware loop

    // accessors
    SideDisplay &getSideDisplay() { return sideDisplay; }

    RotaryWrapper &getRotaryWrapper() { return rotaryWrapper; }

    Button &getRotButton() { return rotButton; }

    TwoWire &getWire();
    SPIClass &getSpiDisplay();
    SPIClass & getSpiSd();



private:
    SPIClass spiDisplay = SPIClass(FSPI);
    SPIClass spiSD = SPIClass(HSPI);

    SideDisplay sideDisplay; // the side display

    RotaryWrapper rotaryWrapper = RotaryWrapper(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN);

    Button rotButton = Button(ROTARY_ENCODER_BUTTON_PIN);

    static Adafruit_MCP23X08 mcp23008;

    unsigned long getLastActivity();
};


#endif //LASERTAG25_HARDWARE_H
