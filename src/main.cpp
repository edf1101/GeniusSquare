/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Application entry point. Initialises hardware and launches the main menu.
 */

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "RotaryWrapper.h"
#include "input/ButtonWrapper.h"
#include "menu/ScreenManager.h"
#include "menu/MenuItem.h"
#include "menu/CarouselMenuScreen.h"

// ---- Hardware pins ----
#define ROT_A    17
#define ROT_B    18
#define BTN_PIN  16

// ---- Hardware objects ----
TFT_eSPI tft = TFT_eSPI();
RotaryWrapper rot(ROT_A, ROT_B);

// ---- Application ----
ScreenManager screenManager;

// Forward declarations for menu item actions
void onSolverSelected(ScreenManager& mgr);
void onPracticeSelected(ScreenManager& mgr);
void onMultiplayerSelected(ScreenManager& mgr);

MenuItem mainMenuItems[] = {
    { nullptr, 0, 0, 'S', "Solver",      onSolverSelected      },
    { nullptr, 0, 0, 'P', "Practice",    onPracticeSelected    },
    { nullptr, 0, 0, 'M', "Multiplayer", onMultiplayerSelected },
};

CarouselMenuScreen mainMenu(tft, screenManager, mainMenuItems, 3, "Genius Square");

ButtonWrapper button(BTN_PIN, []() { screenManager.onButtonPress(); });

void onSolverSelected(ScreenManager&)      { /* push SolverScreen when implemented */ }
void onPracticeSelected(ScreenManager&)    { /* push PracticeScreen when implemented */ }
void onMultiplayerSelected(ScreenManager&) { /* push MultiplayerScreen when implemented */ }

void setup() {
    Serial.begin(115200);

    tft.init();
    tft.setRotation(1); // landscape: 280×240
    tft.writecommand(0x36); // Forcefully fix RGB/BGR bit in MADCTL after rotation
    tft.writedata(0xA0);
    tft.fillScreen(TFT_BLACK);

    analogSetAttenuation(ADC_11db);

    rot.setCallbackFunc([](long delta) {
        screenManager.onEncoderChange(-(int)delta);
    });

    screenManager.push(&mainMenu);
}

void loop() {
    rot.poll();
    button.poll();
    screenManager.update();
    screenManager.render();
}
