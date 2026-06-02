/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Application entry point. Initialises hardware and launches the main menu.
 */

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "input/RotaryWrapper.h"
#include "input/ButtonWrapper.h"
#include "menu/ScreenManager.h"
#include "menu/MenuItem.h"
#include "menu/CarouselMenuScreen.h"
#include "menu/SolverMenuScreen.h"
#include "menu/ArrangementItem.h"
#include "menu/ArrangementMenuScreen.h"
#include "solver/CombinationGenerator.h"
#include "solver/DifficultyLookup.h"
#include "utils/GridScanner.h"
#include "menu/PracticeGameScreen.h"
#include "menu/PracticeScoreScreen.h"
#include "utils/PracticeScores.h"

// ---- Hardware pins ----
// Rotary encoder pins connected to ESP32-S3 using PCNT hardware counter
#define ROT_A    17  ///< Rotary encoder A pin
#define ROT_B    18  ///< Rotary encoder B pin
#define BTN_PIN  16  ///< Button pin (active LOW, internally pulled up)
#define LATCH_PIN 21

#define NO_ACTVITY_TIMEOUT_S (60*5)

// ---- Hardware objects ----
/// Shared TFT_eSPI display instance (240×280 landscape after rotation)
TFT_eSPI tft = TFT_eSPI();

/// Rotary encoder wrapper; provides delta-based callback on half-quad ticks
RotaryWrapper rot(ROT_A, ROT_B, true);

// ---- Application ----
/// Screen stack manager — owns all active screens and forwards input events
ScreenManager screenManager;

// Forward declarations for menu item actions
void onSolverSelected(ScreenManager& mgr);
void onPracticeSelected(ScreenManager& mgr);
void onPowerOffSelected(ScreenManager& mgr);

/// Main menu carousel: 3 game modes (Solver, Practice, Multiplayer — last disabled)
MenuItem mainMenuItems[] = {
    {nullptr, 0, 0, 'S', "Solver", onSolverSelected},
    {nullptr, 0, 0, 'P', "Practice", onPracticeSelected},
    {nullptr, 0, 0, 'D', "Power Off", onPowerOffSelected},
};

/// Main carousel screen — animates 3 menu options with tile morphing
CarouselMenuScreen mainMenu(tft, screenManager, mainMenuItems, 3, "Genius Square");

/// Solver entry screen — Solve / Back two-button layout
SolverMenuScreen solverMenu(tft, screenManager);

/// Practice arrangement items — populated in setup() from CombinationGenerator
static constexpr uint8_t PRACTICE_ITEM_COUNT = 100;
ArrangementItem practiceItems[PRACTICE_ITEM_COUNT];

/// Practice arrangement selection screen
ArrangementMenuScreen practiceMenu(tft, screenManager, practiceItems, PRACTICE_ITEM_COUNT, "Practice");

/// Score screen shown after a practice game ends
PracticeScoreScreen practiceScore(tft, screenManager);

/// Active practice game screen — configured via setArrangement() before push
PracticeGameScreen practiceGame(tft, screenManager, practiceScore);

/// Button input with 50ms debounce; reports presses to ScreenManager
ButtonWrapper button(BTN_PIN, []() { screenManager.onButtonPress(); }, []()
{
});

/**
 * @brief Solver menu action — push the solver list screen onto the stack.
 */
void onSolverSelected(ScreenManager& mgr) { mgr.push(&solverMenu); }

/**
 * @brief Practice menu action — push the arrangement selection screen.
 */
void onPracticeSelected(ScreenManager& mgr) { mgr.push(&practiceMenu); }

/**
 * @brief Turns off the device by entering an infinite loop with the latch pin LOW.
 */
void onPowerOffSelected(ScreenManager&)
{
    while (1)
    {
        digitalWrite(LATCH_PIN, LOW);
        pinMode(LATCH_PIN, OUTPUT);
        delay(500);
    }
}

/**
 * @brief Arduino setup() — initialises hardware and launches the main menu.
 *
 * 1. Start serial debug output (115200 baud)
 * 2. Initialise TFT display (ST7789 via SPI):
 *    - Set landscape rotation (280×240)
 *    - Force MADCTL RGB/BGR bit to match expected colour format
 *    - Fill screen with black
 * 3. Configure ADC attenuation for grid scanner column reads
 * 4. Attach rotary encoder callback (negated delta so CW = +1)
 * 5. Push main menu onto ScreenManager stack (calls onEnter())
 */
void setup()
{
    Serial.begin(115200);

    pinMode(LATCH_PIN, OUTPUT);
    digitalWrite(LATCH_PIN, HIGH);

    // Initialise ST7789 240×280 display with FSPI/HSPI hardware SPI.
    // Rotates to landscape 280×240 for buttons on the right edge.
    tft.init();
    tft.setRotation(1); // landscape: 280×240
    // Force RGB/BGR bit in MADCTL after rotation to fix colour channel order.
    // Without this, red and blue channels would be swapped (TFT_eSPI library quirk).
    tft.writecommand(0x36); // MADCTL (Memory Access Control)
    tft.writedata(0xA0); // MY=1, MX=0, MV=0, ML=0, RGB=1
    tft.fillScreen(TFT_BLACK);

    // GridScanner handles ADC configuration
    GridScanner::init();

    // Initialise EEPROM backing store for practice best scores.
    PracticeScores::init(PRACTICE_ITEM_COUNT);

    // Attach rotary encoder callback. Negated delta so CW rotation = positive delta.
    // (ESP32Encoder convention is CW = negative, but UI convention is CW = +1)
    rot.setCallbackFunc([](long delta)
    {
        screenManager.onEncoderChange(-(int)delta);
    });

    // Populate practice arrangements from the first N CombinationGenerator seeds.
    for (int i = 0; i < PRACTICE_ITEM_COUNT; i++)
    {
        PuzzleDifficulty packed = getDifficulty(i);
        switch (packed)
        {
        case PuzzleDifficulty::HARD: practiceItems[i].difficulty = Difficulty::HARD;
            break;
        case PuzzleDifficulty::MEDIUM: practiceItems[i].difficulty = Difficulty::MED;
            break;
        case PuzzleDifficulty::EASY: practiceItems[i].difficulty = Difficulty::EASY;
            break;
        default: practiceItems[i].difficulty = Difficulty::EASY;
            break;
        }
        uint8_t stored = PracticeScores::load(i);
        practiceItems[i].seconds = (stored > 0) ? (float)stored : 0.0f;
        practiceItems[i].arrangement = CombinationGenerator::generateCombinations(i);
        practiceItems[i].action = [i](ScreenManager& mgr)
        {
            practiceGame.setArrangement(i, practiceItems[i].arrangement, &practiceItems[i].seconds);
            mgr.push(&practiceGame);
        };
    }

    // Push main menu screen to kick off the UI.
    screenManager.push(&mainMenu);
}

/**
 * @brief Arduino loop() — the main event loop, called ~1000 times per second.
 *
 * Each iteration:
 * 1. Poll rotary encoder for step changes (reads PCNT counter)
 * 2. Poll button for debounced presses
 * 3. Update all animation state (carousel lerp, border wave, etc.)
 * 4. Render the current screen to the TFT
 *
 * The ScreenManager forwards input events and render calls to the active screen.
 */
void loop()
{
    rot.poll(); // Check encoder; fires callback on delta
    button.poll(); // Check button; fires callback on debounced press
    screenManager.update(); // Advance animations (called every frame)
    screenManager.render(); // Draw current screen to TFT

    if (millis() - max(rot.getLastActivity(), button.getLastActivity()) > 1000*NO_ACTVITY_TIMEOUT_S)
    {
       // turn off device
         while (1)
          {
                digitalWrite(LATCH_PIN, LOW);
                pinMode(LATCH_PIN, OUTPUT);
                delay(500);
          }
    }
}
