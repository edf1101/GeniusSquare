/*
 * Created by Ed Fillingham on 10/05/2026.
 *
 * PracticeScoreScreen — static results screen shown after completing a practice game.
 */

#include "menu/PracticeScoreScreen.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

PracticeScoreScreen::PracticeScoreScreen(TFT_eSPI& tft, ScreenManager& manager)
    : _tft(tft), _manager(manager),
      _elapsedSeconds(0.0f), _bestSeconds(0.0f), _isNewBest(false),
      _lastMs(0), _dirty(false)
{}

void PracticeScoreScreen::setResult(float elapsedSeconds, float bestSeconds, bool isNewBest) {
    _elapsedSeconds = elapsedSeconds;
    _bestSeconds    = bestSeconds;
    _isNewBest      = isNewBest;
}

void PracticeScoreScreen::onEnter() {
    _border.reset();
    _lastMs = millis();
    _dirty  = true;
    _tft.fillScreen(COL_BG);
    drawAll();
}

void PracticeScoreScreen::onExit() {}

void PracticeScoreScreen::onResume() {
    _border.reset();
    _lastMs = millis();
    _dirty  = true;
    _tft.fillScreen(COL_BG);
    drawAll();
}

void PracticeScoreScreen::update() {
    unsigned long now = millis();
    float dt = (now - _lastMs) / 1000.0f;
    _lastMs = now;
    _border.advance(dt);
    _dirty = true;
}

void PracticeScoreScreen::render() {
    if (!_dirty) return;
    _dirty = false;
    updateBorder();
}

void PracticeScoreScreen::onEncoderChange(int) {}

void PracticeScoreScreen::onButtonPress() {
    _manager.pop(); // pop PracticeScoreScreen; onResume() fires on PracticeGameScreen
    _manager.pop(); // pop PracticeGameScreen;  onResume() fires on ArrangementMenuScreen
}

void PracticeScoreScreen::drawAll() {
    // Title bar
    _tft.setTextColor(COL_TITLE, COL_BG);
    _tft.setTextSize(2);
    const char* title = "Results";
    int tx = (SCR_W - (int)strlen(title) * 12) / 2;
    _tft.setCursor(tx, (TITLE_H - 16) / 2);
    _tft.print(title);

    // Divider
    _tft.drawFastHLine(0, TITLE_H, SCR_W, COL_SUBTEXT);

    char elapsedBuf[8], bestBuf[8];
    formatTime(_elapsedSeconds, elapsedBuf, sizeof(elapsedBuf));
    if (_bestSeconds <= 0.0f) {
        snprintf(bestBuf, sizeof(bestBuf), "--:--");
    } else {
        formatTime(_bestSeconds, bestBuf, sizeof(bestBuf));
    }

    int bodyH = 16 + 8 + 16 + (_isNewBest ? (8 + 16) : 0);
    int bodyY = TITLE_H + (BTN_Y - TITLE_H - bodyH) / 2;

    char yourLine[24];
    snprintf(yourLine, sizeof(yourLine), "Your time: %s", elapsedBuf);
    char bestLine[24];
    snprintf(bestLine, sizeof(bestLine), "Best:      %s", bestBuf);

    _tft.setTextColor(COL_TEXT, COL_BG);
    _tft.setTextSize(2);

    int lineW = (int)strlen(yourLine) * 12;
    _tft.setCursor((SCR_W - lineW) / 2, bodyY);
    _tft.print(yourLine);

    lineW = (int)strlen(bestLine) * 12;
    _tft.setCursor((SCR_W - lineW) / 2, bodyY + 24);
    _tft.print(bestLine);

    if (_isNewBest) {
        _tft.setTextColor(COL_NEW_BEST, COL_BG);
        const char* nb = "New best!";
        lineW = (int)strlen(nb) * 12;
        _tft.setCursor((SCR_W - lineW) / 2, bodyY + 52);
        _tft.print(nb);
    }

    // Button text — animated border is drawn by updateBorder() in render()
    _tft.setTextColor(COL_TEXT, COL_BG);
    _tft.setTextSize(2);
    const char* btnLabel = "Main Menu";
    lineW = (int)strlen(btnLabel) * 12;
    _tft.setCursor(BTN_X + (BTN_W - lineW) / 2, BTN_Y + (BTN_H - 16) / 2);
    _tft.print(btnLabel);
}

void PracticeScoreScreen::updateBorder() {
    float exactThickness = BorderAnimator::BORDER_T_MIN
        + (BorderAnimator::BORDER_T_MAX - BorderAnimator::BORDER_T_MIN)
        * (0.5f + 0.5f * sinf(_border.phase));

    uint16_t borderColour = (_border.envelope >= 0.99f)
        ? COL_BORDER
        : BorderAnimator::blendColor(COL_BG, COL_BORDER, _border.envelope);

    if (fabsf(exactThickness - _border.lastThickness) < BorderAnimator::BORDER_CHANGE_TOL
        && borderColour == _border.lastColor) return;

    if (_border.envelope < 0.02f) {
        if (_border.lastThickness >= 0.0f)
            BorderAnimator::drawBorderWithRoundedCorners(
                _tft, BTN_X, BTN_Y, BTN_W, BTN_H, _border.lastThickness, COL_BG, COL_BG);
        _border.lastThickness = -1.0f;
        _border.lastColor     = 0;
        return;
    }

    BorderAnimator::drawBorderWithRoundedCorners(
        _tft, BTN_X, BTN_Y, BTN_W, BTN_H, exactThickness, borderColour, COL_BG);
    _border.lastThickness = exactThickness;
    _border.lastColor     = borderColour;
}

void PracticeScoreScreen::formatTime(float seconds, char* buf, size_t bufSize) const {
    int total = (int)seconds;
    snprintf(buf, bufSize, "%02d:%02d", total / 60, total % 60);
}
