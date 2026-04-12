#pragma once

// Project-local TFT_eSPI setup for ST7789 on ESP32-S3.
// This file is selected via platformio.ini:
//   -D USER_SETUP_LOADED=1
//   -D USER_SETUP_FILE="User_Setup_GeniusSquare.h"

#define ST7789_DRIVER

// Change to your panel resolution if different.
#define TFT_WIDTH  240
#define TFT_HEIGHT 280

// ESP32-S3 SPI control pins (set these to your wiring)
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8

// Optional backlight control pin (-1 if not connected)
#define TFT_BL   -1
#define TFT_BACKLIGHT_ON HIGH

// Fonts to load (trim if flash is tight)
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// SPI frequencies
#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000

