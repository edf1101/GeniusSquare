/*
 * Created by Ed Fillingham on 25/07/2025.
*/

#include "Hardware.h"
#include "SD.h"


/**
 * This constructor initialises the hardware components.
 */
Hardware::Hardware():sideDisplay(spiDisplay) {
}

/**
 * This function initialises the hardware components of the laser tag system.
 */
void Hardware::init() {
  // init wire and spi peripherals
  spiDisplay.begin(DISPLAY_SCLK_PIN,-1,DISPLAY_MOSI_PIN,-1);
  spiSD.begin(SD_SCK_PIN,SD_MISO_PIN,SD_MOSI_PIN,-1);


  if (!SD.begin(SD_CS_PIN,spiSD)) {

    while (1) Serial.println("ERROR: SD mount failed");
  }

  sideDisplay.init(); // Initialise the side display

  rotButton.init();

}

/**
 * This function runs the hardware loop, which is called repeatedly to update the hardware state.
 */
void Hardware::loop() {
  rotaryWrapper.poll();
  rotButton.poll();
}

/**
 * This function returns the last activity timestamp of the hardware.
 *
 * @return The last activity timestamp in milliseconds.
 */
unsigned long Hardware::getLastActivity() {
  return max(rotaryWrapper.getLastActivity(), rotButton.getLastActivity());
}

/**
 * Getter for the Main SPI peripheral
 *
 * @return The main SPI peripheral
 */
SPIClass &Hardware::getSpiDisplay() {
  return spiDisplay;
}

/**
 * Getter for the SD SPI peripheral
 *
 * @return The SD SPI peripheral
 */
SPIClass &Hardware::getSpiSd() {
  return spiSD;
}


