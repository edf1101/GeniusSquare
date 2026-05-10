/*
 * Created by Ed Fillingham on 10/05/2026.
 *
 * PracticeGameScreen — three-phase practice game (blocker placement, countdown, timed solve).
 */

#include "menu/PracticeGameScreen.h"
#include "menu/PracticeScoreScreen.h"
#include "../utils/GridScanner.h"
#include "utils/PracticeScores.h"
#include <cstring>
#include <cstdio>
#include <math.h>

PracticeGameScreen::PracticeGameScreen(TFT_eSPI& tft, ScreenManager& manager,
                                       PracticeScoreScreen& scoreScreen)
    : _tft(tft), _manager(manager), _scoreScreen(scoreScreen),
      _arrangementIndex(0), _bestSeconds(nullptr),
      _phase(Phase::PLACING), _confirmedCount(0),
      _panelDirty(false), _timerDirty(false),
      _hasError(false), _errorCol(0), _errorRow(0),
      _elapsedSeconds(0.0f), _timerStartMs(0),
      _lastScanMs(0), _lastFrameMs(0), _lastMs(0),
      _countdownRemaining(0), _countdownStartMs(0)
{
    memset(_grid,      false, sizeof(_grid));
    memset(_cellDirty, false, sizeof(_cellDirty));
}

void PracticeGameScreen::setArrangement(uint8_t index, const std::vector<Coord>& blockers, float* bestSeconds) {
    _arrangementIndex   = index;
    _blockers           = blockers;
    _bestSeconds        = bestSeconds;
    _phase              = Phase::PLACING;
    _confirmedCount     = 0;
    _hasError           = false;
    _elapsedSeconds     = 0.0f;
    _panelDirty         = false;
    _timerDirty         = false;
    _countdownRemaining = 0;
    memset(_grid,      false, sizeof(_grid));
    memset(_cellDirty, false, sizeof(_cellDirty));
}

void PracticeGameScreen::onEnter() {
    _border.reset();
    _lastMs = millis();
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawBackButton();
    drawGridLines();
    drawAllCells();
    drawPanel();
    _lastScanMs = _lastFrameMs = millis();
}

void PracticeGameScreen::onExit() {}

void PracticeGameScreen::onResume() {
    _border.reset();
    _lastMs = millis();
    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawBackButton();
    drawGridLines();
    drawAllCells();
    drawPanel();
}

void PracticeGameScreen::onEncoderChange(int) {}

void PracticeGameScreen::onButtonPress() {
    _manager.pop();
}

void PracticeGameScreen::update() {
    unsigned long now = millis();
    float dt = (now - _lastMs) / 1000.0f;
    _lastMs = now;
    _border.advance(dt);

    if (now - _lastScanMs >= SCAN_INTERVAL_MS) {
        _lastScanMs = now;
        scanGrid();
    }

    if (_phase == Phase::COUNTDOWN) {
        // Use fresh millis() — _countdownStartMs was set inside scanGrid() after `now` was captured,
        // so using `now` here would underflow and immediately skip the countdown.
        unsigned long curMs   = millis();
        unsigned long elapsed = curMs - _countdownStartMs;
        if (elapsed >= (unsigned long)COUNTDOWN_SECONDS * 1000UL) {
            _phase          = Phase::PLAYING;
            _timerStartMs   = curMs;
            _elapsedSeconds = 0.0f;
            _panelDirty     = true;
        } else {
            uint8_t rem = (uint8_t)(COUNTDOWN_SECONDS - elapsed / 1000);
            if (rem != _countdownRemaining) {
                _countdownRemaining = rem;
                _panelDirty = true;
            }
        }
    } else if (_phase == Phase::PLAYING) {
        float newElapsed = (now - _timerStartMs) / 1000.0f;
        if ((int)newElapsed != (int)_elapsedSeconds) {
            _timerDirty = true;
        }
        _elapsedSeconds = newElapsed;
    }

    _lastFrameMs = now;
}

void PracticeGameScreen::render() {
    for (uint8_t r = 0; r < GRID_ROWS; r++) {
        for (uint8_t c = 0; c < GRID_COLS; c++) {
            if (_cellDirty[r][c]) {
                _cellDirty[r][c] = false;
                drawCell(r, c);
            }
        }
    }
    if (_panelDirty) {
        _panelDirty = false;
        _timerDirty = false;
        drawPanel();
    } else if (_timerDirty) {
        _timerDirty = false;
        drawTimerArea();
    }
    updateBorder();
}

void PracticeGameScreen::drawTitleBar() {
    _tft.setTextColor(COL_TITLE, COL_BG);
    _tft.setTextSize(2);
    const char* title = "Practice";
    int tx = (SCR_W - (int)strlen(title) * 12) / 2;
    _tft.setCursor(tx, (TITLE_H - 16) / 2);
    _tft.print(title);
    _tft.drawFastHLine(0, TITLE_H, SCR_W, COL_SUBTEXT);
}

void PracticeGameScreen::drawBackButton() {
    _tft.fillRect(GRID_X, BTN_Y, BTN_W, BTN_H, COL_BG);
    _tft.setTextColor(COL_TEXT, COL_BG);
    _tft.setTextSize(2);
    const char* label = "Back";
    int lw = (int)strlen(label) * 12;
    _tft.setCursor(GRID_X + (BTN_W - lw) / 2, BTN_Y + (BTN_H - 16) / 2);
    _tft.print(label);
}

void PracticeGameScreen::updateBorder() {
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
                _tft, GRID_X, BTN_Y, BTN_W, BTN_H, _border.lastThickness, COL_BG, COL_BG);
        _border.lastThickness = -1.0f;
        _border.lastColor     = 0;
        return;
    }

    BorderAnimator::drawBorderWithRoundedCorners(
        _tft, GRID_X, BTN_Y, BTN_W, BTN_H, exactThickness, borderColour, COL_BG);
    _border.lastThickness = exactThickness;
    _border.lastColor     = borderColour;
}

void PracticeGameScreen::drawGridLines() {
    for (int i = 0; i <= GRID_ROWS; i++) {
        _tft.fillRect(GRID_X, GRID_Y + i * CELL_SIZE, GRID_SIZE, GRID_LINE_W, COL_GRID_LINE);
        _tft.fillRect(GRID_X + i * CELL_SIZE, GRID_Y, GRID_LINE_W, GRID_SIZE, COL_GRID_LINE);
    }
}

void PracticeGameScreen::drawCell(uint8_t row, uint8_t col) {
    int px = GRID_X + col * CELL_SIZE + GRID_LINE_W;
    int py = GRID_Y + row * CELL_SIZE + GRID_LINE_W;
    int sz = CELL_SIZE - GRID_LINE_W;   // 22 px
    int cx = px + sz / 2;
    int cy = py + sz / 2;
    int cr = sz / 2 - 4;               // circle radius with 4 px padding

    _tft.fillRect(px, py, sz, sz, COL_BG);

    if (_phase == Phase::PLACING) {
        if (isBlockerCell(row, col)) {
            if (_grid[row][col]) {
                _tft.fillCircle(cx, cy, cr, COL_BLOCKER);   // correctly placed blocker
            } else {
                _tft.drawRect(px, py, sz, sz, COL_TARGET);  // unplaced blocker target outline
            }
        } else if (_hasError && _errorRow == row && _errorCol == col) {
            // Red X: piece placed in a non-blocker cell
            int pad = 4;
            int x0 = px + pad, y0 = py + pad;
            int x1 = px + sz - pad - 1, y1 = py + sz - pad - 1;
            _tft.drawLine(x0,     y0,     x1,     y1,     COL_ERROR);
            _tft.drawLine(x0 + 1, y0,     x1,     y1 - 1, COL_ERROR);
            _tft.drawLine(x0,     y0 + 1, x1 - 1, y1,     COL_ERROR);
            _tft.drawLine(x0,     y1,     x1,     y0,     COL_ERROR);
            _tft.drawLine(x0 + 1, y1,     x1,     y0 + 1, COL_ERROR);
            _tft.drawLine(x0,     y1 - 1, x1 - 1, y0,     COL_ERROR);
        }
    } else {
        // COUNTDOWN and PLAYING: blocker circles always visible (ignores scanner state)
        // so circle persists even if the physical piece falls out of the scanner range
        if (isBlockerCell(row, col)) {
            _tft.fillCircle(cx, cy, cr, COL_BLOCKER);
        } else if (_grid[row][col]) {
            int pad = 3;
            _tft.fillRoundRect(px + pad, py + pad, sz - 2 * pad, sz - 2 * pad, 3, COL_PIECE);
        }
    }
}

void PracticeGameScreen::drawPanel() {
    _tft.fillRect(PANEL_X, PANEL_Y, PANEL_W, SCR_H - PANEL_Y, COL_BG);

    if (_phase == Phase::PLACING) {
        _tft.setTextColor(COL_SUBTEXT, COL_BG);
        _tft.setTextSize(1);
        _tft.setCursor(PANEL_X + 4, PANEL_Y + 8);
        _tft.print("Place blockers");

        char progBuf[8];
        snprintf(progBuf, sizeof(progBuf), "%d / %d", _confirmedCount, (int)_blockers.size());
        _tft.setTextColor(COL_TEXT, COL_BG);
        _tft.setTextSize(3);
        int progW = (int)strlen(progBuf) * 18;
        _tft.setCursor(PANEL_X + (PANEL_W - progW) / 2, PANEL_Y + 30);
        _tft.print(progBuf);

        if (_hasError) {
            _tft.setTextColor(COL_ERROR, COL_BG);
            _tft.setTextSize(1);
            _tft.setCursor(PANEL_X + 4, PANEL_Y + 80);
            _tft.print("Wrong!");
            _tft.setCursor(PANEL_X + 4, PANEL_Y + 90);
            _tft.print("Remove it.");
        }

    } else if (_phase == Phase::COUNTDOWN) {
        _tft.setTextColor(COL_SUBTEXT, COL_BG);
        _tft.setTextSize(2);
        const char* label = "Get ready!";
        int lw = (int)strlen(label) * 12;
        _tft.setCursor(PANEL_X + (PANEL_W - lw) / 2, PANEL_Y + 20);
        _tft.print(label);

        char cntBuf[4];
        snprintf(cntBuf, sizeof(cntBuf), "%d", _countdownRemaining);
        _tft.setTextColor(COL_TEXT, COL_BG);
        _tft.setTextSize(5);
        int cw = (int)strlen(cntBuf) * 30;
        _tft.setCursor(PANEL_X + (PANEL_W - cw) / 2, PANEL_Y + 55);
        _tft.print(cntBuf);

    } else {
        _tft.setTextColor(COL_SUBTEXT, COL_BG);
        _tft.setTextSize(2);
        const char* label = "Solve it!";
        int lw = (int)strlen(label) * 12;
        _tft.setCursor(PANEL_X + (PANEL_W - lw) / 2, PANEL_Y + 20);
        _tft.print(label);

        char timeBuf[8];
        formatTime(_elapsedSeconds, timeBuf, sizeof(timeBuf));
        _tft.setTextColor(COL_TEXT, COL_BG);
        _tft.setTextSize(3);
        int tw = (int)strlen(timeBuf) * 18;
        _tft.setCursor(PANEL_X + (PANEL_W - tw) / 2, PANEL_Y + 44);
        _tft.print(timeBuf);

        // Previous best time in small text below timer
        char bestBuf[8];
        if (_bestSeconds != nullptr && *_bestSeconds > 0.0f) {
            formatTime(*_bestSeconds, bestBuf, sizeof(bestBuf));
        } else {
            snprintf(bestBuf, sizeof(bestBuf), "--:--");
        }
        _tft.setTextColor(COL_SUBTEXT, COL_BG);
        _tft.setTextSize(1);
        char bestLine[16];
        snprintf(bestLine, sizeof(bestLine), "Best: %s", bestBuf);
        int bl = (int)strlen(bestLine) * 6;
        _tft.setCursor(PANEL_X + (PANEL_W - bl) / 2, PANEL_Y + 80);
        _tft.print(bestLine);
    }
}

void PracticeGameScreen::drawTimerArea() {
    // setTextColor with two args fills each glyph's background inline — no separate erase needed.
    // "MM:SS" is always 5 chars so the cursor position is fixed; old digits are overwritten in place.
    char timeBuf[8];
    formatTime(_elapsedSeconds, timeBuf, sizeof(timeBuf));
    _tft.setTextColor(COL_TEXT, COL_BG);
    _tft.setTextSize(3);
    int tw = (int)strlen(timeBuf) * 18;
    _tft.setCursor(PANEL_X + (PANEL_W - tw) / 2, PANEL_Y + 44);
    _tft.print(timeBuf);
}

void PracticeGameScreen::drawAllCells() {
    for (uint8_t r = 0; r < GRID_ROWS; r++)
        for (uint8_t c = 0; c < GRID_COLS; c++)
            drawCell(r, c);
}

void PracticeGameScreen::scanGrid() {
    auto raw = GridScanner::digitalReadMatrix();

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            bool newVal = raw[r][c];
            if (newVal != _grid[r][c]) {
                _grid[r][c]      = newVal;
                _cellDirty[r][c] = true;
            }
        }
    }

    if (_phase == Phase::PLACING) {
        if (_blockers.empty()) return;

        // Count how many blocker positions are currently occupied (any order)
        uint8_t placed = 0;
        for (const auto& b : _blockers) {
            if (_grid[(uint8_t)b.y][(uint8_t)b.x]) placed++;
        }
        if (placed != _confirmedCount) {
            _confirmedCount = placed;
            _panelDirty = true;
        }

        // Error: any occupied cell that is not a blocker position
        bool hasError = false;
        uint8_t errRow = 0, errCol = 0;
        for (int r = 0; r < GRID_ROWS && !hasError; r++) {
            for (int c = 0; c < GRID_COLS && !hasError; c++) {
                if (!_grid[r][c]) continue;
                if (isBlockerCell(r, c)) continue;
                hasError = true;
                errRow   = r;
                errCol   = c;
            }
        }

        bool prevError   = _hasError;
        uint8_t prevErrRow = _errorRow, prevErrCol = _errorCol;
        _hasError = hasError;
        _errorRow = errRow;
        _errorCol = errCol;
        if (_hasError != prevError || errRow != prevErrRow || errCol != prevErrCol) {
            if (prevError)  _cellDirty[prevErrRow][prevErrCol] = true;
            if (_hasError)  _cellDirty[errRow][errCol]         = true;
            _panelDirty = true;
        }

        // All 7 placed with no errors → start countdown
        if (_confirmedCount >= (uint8_t)_blockers.size() && !hasError) {
            _phase              = Phase::COUNTDOWN;
            _countdownRemaining = COUNTDOWN_SECONDS;
            _countdownStartMs   = millis();
            _panelDirty         = true;
            for (int r = 0; r < GRID_ROWS; r++)
                for (int c = 0; c < GRID_COLS; c++)
                    _cellDirty[r][c] = true;
        }

    } else if (_phase == Phase::PLAYING) {
        bool allFull = true;
        for (int r = 0; r < GRID_ROWS && allFull; r++)
            for (int c = 0; c < GRID_COLS && allFull; c++)
                if (!_grid[r][c]) allFull = false;

        if (allFull) {
            _elapsedSeconds = (millis() - _timerStartMs) / 1000.0f;
            finishGame();
        }
    }
}

void PracticeGameScreen::finishGame() {
    bool isNewBest = (_bestSeconds != nullptr)
        && (*_bestSeconds <= 0.0f || _elapsedSeconds < *_bestSeconds);
    if (isNewBest) {
        *_bestSeconds = _elapsedSeconds;
        uint8_t toStore = (_elapsedSeconds >= 255.0f) ? 255u : (uint8_t)_elapsedSeconds;
        if (toStore == 0) toStore = 1;  // 0 is reserved as "no score"
        PracticeScores::save(_arrangementIndex, toStore);
    }
    float best = (_bestSeconds != nullptr) ? *_bestSeconds : 0.0f;
    _scoreScreen.setResult(_elapsedSeconds, best, isNewBest);
    _manager.push(&_scoreScreen);
}

bool PracticeGameScreen::isBlockerCell(uint8_t row, uint8_t col) const {
    for (const auto& b : _blockers) {
        if ((uint8_t)b.y == row && (uint8_t)b.x == col) return true;
    }
    return false;
}

void PracticeGameScreen::formatTime(float seconds, char* buf, size_t bufSize) const {
    int total = (int)seconds;
    snprintf(buf, bufSize, "%02d:%02d", total / 60, total % 60);
}
