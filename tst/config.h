/*
 * Created by Ed Fillingham on 18/03/2026.
*/

#ifndef GENIUSSQUARE_CONFIG_H
#define GENIUSSQUARE_CONFIG_H


const byte ROTARY_ENCODER_A_PIN = 18; // GPIO pin for rotary encoder A
const byte ROTARY_ENCODER_B_PIN = 17; // GPIO pin for rotary encoder B
const byte ROTARY_ENCODER_BUTTON_PIN = 16; // GPIO pin for rotary encoder button

const byte DISPLAY_MOSI_PIN = 5; // GPIO pin for display MOSI
const byte DISPLAY_SCLK_PIN = 4; // GPIO pin for display SCLK
const byte DISPLAY_CS_PIN = 11; // GPIO pin for display CS
const byte DISPLAY_DC_PIN = 12; // GPIO pin for display DC
const byte DISPLAY_RST_PIN = 13; // GPIO pin for display RST

const byte MATRIX_ROW_PINS[] = {14, 47, 48, 38, 41, 42}; // GPIO pins for matrix rows
const byte MATRIX_COL_PINS[] = {7, 8, 10, 9, 1, 2}; // GPIO pins for matrix columns (ADC pins)

const byte DEBUG_LED_PIN = 6; // GPIO pin for debug LED


// N.B. For whatever reason, sharing SD and display SPI buses causes SD reads to fail, so they are on separate buses.
const byte SD_CS_PIN = 15; // GPIO pin for SD card CS
const byte SD_SCK_PIN = 45; // GPIO pin for SD card SCK
const byte SD_MISO_PIN = 40; // GPIO pin for SD card MISO
const byte SD_MOSI_PIN = 39; // GPIO pin for SD card MOSI

const byte POWER_CTL_PIN = 21; // GPIO pin for power control

const byte BUZZER_PIN = 46; // GPIO pin for buzzer


#endif //GENIUSSQUARE_CONFIG_H