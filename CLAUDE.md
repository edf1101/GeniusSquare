# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

GeniusSquare is an embedded firmware project for an ESP32-S3 device that implements the Genius Square puzzle game. It targets the `esp32-s3-devkitc1-n16r8` board and is built with PlatformIO using the Arduino framework.

## Build & Development Commands

This project uses [PlatformIO](https://platformio.org/). All commands require the `pio` CLI.

```bash
# Build the project
pio run

# Upload firmware to the connected ESP32-S3
pio run --target upload

# Open serial monitor (115200 baud)
pio device monitor

# Build and upload in one step
pio run --target upload && pio device monitor

# Clean build artifacts
pio run --target clean
```

There is no automated test suite — the `test/` directory contains only a PlatformIO README placeholder.

## Hardware

- **MCU**: ESP32-S3 (N16R8 variant)
- **Display**: ST7789 240×280 TFT via SPI (FSPI/HSPI)
- **Input**: Rotary encoder with push button
- **Grid scanner**: 6×6 resistive matrix (6 digital row outputs + 6 ADC column inputs)

Pin assignments are defined in `src/main.cpp` and mirrored in `include/TFT_setup.h`.

## Architecture

```
src/
  main.cpp            # Arduino setup()/loop(), hardware init, grid scan + display render
  RotaryWrapper.h/.cpp # Thin wrapper over ESP32Encoder; poll()-based callback model
  User_Setup_GeniusSquare.h  # Legacy TFT_eSPI user setup (currently unused)

include/
  TFT_setup.h         # Compiler-injected TFT config (-include flag in platformio.ini)

lib/
  ESP32Encoder/       # Vendored encoder library using ESP32 PCNT hardware
```

`TFT_setup.h` is force-included via the `-include` compiler flag in `platformio.ini` with `-D USER_SETUP_LOADED=1` to override the Adafruit ST7789 library's internal defaults.

`RotaryWrapper` wraps `ESP32Encoder` with a `std::function` callback. Call `poll()` each loop iteration; the callback fires with the signed step delta when a half-quad tick is detected (counts divide by 2 to debounce).

The grid scanner works by pulling each row LOW in sequence, waiting 800µs for settling, then reading all 6 ADC column pins. ADC values are threshold-mapped to colours and drawn directly to the TFT via `fillRect`.

## Planned Modes (from Plan.md)

Three game modes are planned: **Solver** (scan grid, compute solution), **Practice** (solo timed play with EEPROM high scores), and **Multiplayer** (ESP-NOW local wireless, host/join rooms). The shared **Game Mode** handles blocker placement via the grid scanner before starting the timer.

## Code Style

Every file starts with:
```cpp
/*
* Created by Ed Fillingham on DATE.
*
* FILE DESCRIPTION
*/
```

- C++17 (`-std=gnu++17`)
- Header guards (`#ifndef`/`#define`/`#endif`) in all `.h` files
- Doxygen-style docstrings on classes, functions, and enums (brief + `@param`/`@return`)
- Comments only for non-obvious logic