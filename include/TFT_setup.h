// include/TFT_Setup.h
#ifndef TFT_SETUP_H
#define TFT_SETUP_H

// --- Display Driver ---
#define ST7789_2_DRIVER 1
#define TFT_WIDTH 240
#define TFT_HEIGHT 280
#define TFT_RGB_ORDER TFT_RGB


// --- Hardware Pins ---
#define TFT_MOSI 5
#define TFT_SCLK 4
#define TFT_CS 11
#define TFT_DC 12
#define TFT_RST 13
#define TFT_MISO -1 // Prevents default MISO from colliding with Pin 13 (RST)

// --- Display Offsets ---
#define CGRAM_OFFSET 1  // Required for ST7789 240x280 screens
#define TFT_X_OFFSET 0  // Tweak to 20 if the screen wraps horizontally
#define TFT_Y_OFFSET 20 // Standard offset for 1.69" 240x280 displays

// --- SPI Settings ---
#define USE_HSPI_PORT 1 // Binds to HSPI to fix ESP32-S3 routing
#define SPI_FREQUENCY 40000000
#define SPI_READ_FREQUENCY 20000000

// --- Fonts ---
#define LOAD_GLCD   // Font 1 (8x6) - used by setTextFont(1)
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF  // FreeFonts support (optional)


#endif // TFT_SETUP_H