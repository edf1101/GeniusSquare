/*
 * Created by Ed Fillingham on 09/05/2026.
 *
 * SolverMenuScreen — two-button entry screen for solver mode.
 *
 * Layout (landscape 280×240):
 * - Title bar: 30px
 * - Button row (32px, 10px below title): Solve | Back
 * - Left (x=14, y=80): 144×144 live grid, beige circles fade 100 ms
 * - Right (x=160, y=80, 120px wide):
 *     State word (size 2, 20px): "Solvable" (green) OR "Unsolvable" (red)
 *     Reason (size 1, 12px): "Too few" / "Too many" / "Invalid" / blank
 *     y=116+: blocker tiles as white rounded-rect dice faces, 3 per row, max 9
 */

#include "menu/SolverMenuScreen.h"
#include "hardware/GridScanner.h"
#include "solver/CombinationGenerator.h"
#include <math.h>
#include <string.h>
#include <vector>

SolverMenuScreen::SolverMenuScreen(TFT_eSPI& tft, ScreenManager& manager)
    : _tft(tft), _manager(manager),
      _selectedIndex(0),
      _lastMs(millis()), _dirty(false),
      _panelDirty(false), _solvable(false),
      _lastScanMs(0)
{
    memset(_grid,      0, sizeof(_grid));
    memset(_cellAlpha, 0, sizeof(_cellAlpha));
    memset(_cellDirty, 0, sizeof(_cellDirty));
    _reason[0] = '\0';
}

int SolverMenuScreen::buttonX(uint8_t index) const { return (index == 0) ? BUTTON_AREA_X : BACK_X; }
int SolverMenuScreen::buttonW(uint8_t index) const { return (index == 0) ? SOLVE_W : BACK_W; }

// ---- Lifecycle ----

void SolverMenuScreen::onEnter() {
    _selectedIndex = 0;
    _border.reset();
    _lastMs      = millis();
    _dirty       = false;
    _panelDirty  = false;
    _lastScanMs  = millis() - SCAN_INTERVAL_MS;

    memset(_grid,      0, sizeof(_grid));
    memset(_cellAlpha, 0, sizeof(_cellAlpha));
    memset(_cellDirty, 0, sizeof(_cellDirty));
    _reason[0] = '\0';

    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawButtons();
    drawGrid();

    scanGrid();
    drawSidePanel();
    _lastScanMs = millis();
}

void SolverMenuScreen::onExit() {
    _solver.stop();
    _lastBlockers.clear();
}

void SolverMenuScreen::onResume() {
    _border.reset();
    _lastMs      = millis();
    _dirty       = false;
    _panelDirty  = false;
    _lastScanMs  = millis() - SCAN_INTERVAL_MS;

    memset(_grid,      0, sizeof(_grid));
    memset(_cellAlpha, 0, sizeof(_cellAlpha));
    memset(_cellDirty, 0, sizeof(_cellDirty));
    _reason[0] = '\0';

    _tft.fillScreen(COL_BG);
    drawTitleBar();
    drawButtons();
    drawGrid();
    scanGrid();
    drawSidePanel();
    _lastScanMs = millis();
}

void SolverMenuScreen::update() {
    unsigned long now = millis();
    float dt = (now - _lastMs) / 1000.0f;
    _lastMs = now;
    _border.advance(dt);
    _dirty = true;

    if (now - _lastScanMs >= SCAN_INTERVAL_MS) {
        _lastScanMs = now;
        scanGrid();
    }

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            float target = _grid[r][c] ? 1.0f : 0.0f;
            float alpha  = _cellAlpha[r][c];
            if (fabsf(alpha - target) < 0.001f) continue;
            float step = dt / FADE_DURATION_S;
            _cellAlpha[r][c] = (alpha < target) ? fminf(alpha + step, target)
                                                 : fmaxf(alpha - step, target);
            _cellDirty[r][c] = true;
        }
    }
}

void SolverMenuScreen::render() {
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (_cellDirty[r][c]) {
                _cellDirty[r][c] = false;
                renderCellFaded(r, c, _cellAlpha[r][c]);
            }
        }
    }

    if (_panelDirty) {
        _panelDirty = false;
        drawSidePanel();
    }

    if (!_dirty) return;
    _dirty = false;
    updateBorder();
}

// ---- Input ----

void SolverMenuScreen::onEncoderChange(int delta) {
    uint8_t prev = _selectedIndex;
    int next = (int)_selectedIndex + delta;
    if (next < 0) next = 0;
    if (next > 1) next = 1;
    _selectedIndex = (uint8_t)next;
    if (_selectedIndex == prev) return;

    eraseBorder(prev);
    drawButton(buttonX(prev),           buttonW(prev),           prev == 0 ? "Solve" : "Back", false);
    drawButton(buttonX(_selectedIndex), buttonW(_selectedIndex), _selectedIndex == 0 ? "Solve" : "Back", true);
    drawDeselectedBorder(buttonX(prev), buttonW(prev));
    eraseDeselectedBorder(buttonX(_selectedIndex), buttonW(_selectedIndex));
    _border.reset();
}

void SolverMenuScreen::onButtonPress() {
    if (_selectedIndex == 1) _manager.pop();
    // Solve: no-op for now
}

// ---- Button drawing ----

void SolverMenuScreen::drawTitleBar() {
    const char* title = "Solver";
    _tft.setTextColor(COL_TITLE, COL_BG);
    _tft.setTextSize(2);
    int titleX = (SCR_W - (int)strlen(title) * 12) / 2;
    _tft.setCursor(titleX, (TITLE_H - 16) / 2);
    _tft.print(title);
}

void SolverMenuScreen::drawButtons() {
    drawButton(BUTTON_AREA_X, SOLVE_W, "Solve", _selectedIndex == 0);
    drawButton(BACK_X,        BACK_W,  "Back",  _selectedIndex == 1);
    _tft.drawFastVLine(BACK_X, BUTTON_Y, BUTTON_H, COL_DIVIDER);
    uint8_t unsel = (_selectedIndex == 0) ? 1 : 0;
    drawDeselectedBorder(buttonX(unsel), buttonW(unsel));
}

void SolverMenuScreen::drawButton(int x, int w, const char* label, bool selected) {
    _tft.fillRect(x, BUTTON_Y, w, BUTTON_H, COL_BG);
    _tft.setTextColor(selected ? COL_TEXT_SEL : COL_TEXT_UNSEL, COL_BG);
    _tft.setTextSize(2);
    int textX = x + (w - (int)strlen(label) * 12) / 2;
    int textY = BUTTON_Y + (BUTTON_H - 16) / 2;
    _tft.setCursor(textX, textY);
    _tft.print(label);
}

void SolverMenuScreen::drawDeselectedBorder(int x, int w) {
    BorderAnimator::drawBorderWithRoundedCorners(_tft, x, BUTTON_Y, w, BUTTON_H, 1.0f, COL_BORDER_UNSEL, COL_BG);
}

void SolverMenuScreen::eraseDeselectedBorder(int x, int w) {
    BorderAnimator::drawBorderWithRoundedCorners(_tft, x, BUTTON_Y, w, BUTTON_H, 1.0f, COL_BG, COL_BG);
}

void SolverMenuScreen::updateBorder() {
    int x = buttonX(_selectedIndex);
    int w = buttonW(_selectedIndex);

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
            BorderAnimator::drawBorderWithRoundedCorners(_tft, x, BUTTON_Y, w, BUTTON_H, _border.lastThickness, COL_BG, COL_BG);
        _border.lastThickness = -1.0f;
        _border.lastColor     = 0;
        return;
    }

    BorderAnimator::drawBorderWithRoundedCorners(_tft, x, BUTTON_Y, w, BUTTON_H, exactThickness, borderColour, COL_BG);
    _border.lastThickness = exactThickness;
    _border.lastColor     = borderColour;
}

void SolverMenuScreen::eraseBorder(uint8_t index) {
    if (_border.lastThickness < 0.0f) return;
    BorderAnimator::drawBorderWithRoundedCorners(_tft, buttonX(index), BUTTON_Y, buttonW(index), BUTTON_H, _border.lastThickness, COL_BG, COL_BG);
}

// ---- Grid ----

void SolverMenuScreen::scanGrid() {
    auto result = GridScanner::digitalReadMatrix();
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            bool val = result[r][c];
            if (val != _grid[r][c]) {
                _grid[r][c] = val;
                _panelDirty = true;
            }
        }
    }
    if (_panelDirty) checkValidity();
}

void SolverMenuScreen::drawGrid() {
    for (int i = 0; i <= GRID_ROWS; i++) {
        _tft.fillRect(GRID_X,                 GRID_Y + i * CELL_SIZE, GRID_SIZE,   GRID_LINE_W, COL_GRID_LINE);
        _tft.fillRect(GRID_X + i * CELL_SIZE, GRID_Y,                 GRID_LINE_W, GRID_SIZE,   COL_GRID_LINE);
    }
}

void SolverMenuScreen::renderCellFaded(int r, int c, float alpha) {
    int cellX = GRID_X + c * CELL_SIZE;
    int cellY = GRID_Y + r * CELL_SIZE;
    _tft.fillRect(cellX + GRID_LINE_W, cellY + GRID_LINE_W,
                  CELL_SIZE - GRID_LINE_W, CELL_SIZE - GRID_LINE_W, COL_BG);
    if (alpha > 0.02f) {
        uint16_t col = (alpha >= 0.99f) ? COL_BLOCKER
                                        : BorderAnimator::blendColor(COL_BG, COL_BLOCKER, alpha);
        _tft.fillCircle(cellX + CELL_SIZE / 2, cellY + CELL_SIZE / 2, CIRCLE_R, col);
    }
}

// ---- Validity ----

void SolverMenuScreen::checkValidity() {
    int count = 0;
    std::vector<Coord> coords;
    coords.reserve(GRID_ROWS * GRID_COLS);

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (_grid[r][c]) {
                count++;
                coords.push_back(Coord{c, r});
            }
        }
    }

    if (count < 7) {
        _solvable = false;
        strncpy(_reason, "Too few", sizeof(_reason) - 1);
    } else if (count > 7) {
        _solvable = false;
        strncpy(_reason, "Too many", sizeof(_reason) - 1);
    } else {
        if (CombinationGenerator::validCombination(coords)) {
            _solvable = true;
            _reason[0] = '\0';
        } else {
            _solvable = false;
            strncpy(_reason, "Invalid", sizeof(_reason) - 1);
        }
    }
    _reason[sizeof(_reason) - 1] = '\0';

    if (_solvable) {
        if (coords != _lastBlockers) {
            Serial.println("[SolverMenu] Valid arrangement detected, starting solve");
            _lastBlockers = coords;
            _solver.start(coords);
        }
    } else {
        if (!_lastBlockers.empty()) {
            Serial.println("[SolverMenu] Arrangement changed/invalid, stopping solve");
            _solver.stop();
            _lastBlockers.clear();
        }
    }
}

// ---- Side panel ----

void SolverMenuScreen::drawSidePanel() {
    _tft.fillRect(PANEL_X, PANEL_Y, PANEL_W, GRID_SIZE + GRID_LINE_W, COL_BG);

    // --- State word (size 2) ---
    int y = PANEL_Y + 2;
    _tft.setTextSize(2);
    if (_solvable) {
        _tft.setTextColor(COL_SOLVABLE, COL_BG);
        _tft.setCursor(PANEL_X, y);
        _tft.print("Solvable");
    } else {
        _tft.setTextColor(COL_UNSOLVABLE, COL_BG);
        _tft.setCursor(PANEL_X, y);
        _tft.print("Unsolvable");
        // Reason (size 2, below state word)
        if (_reason[0] != '\0') {
            _tft.setTextSize(2);
            _tft.setCursor(PANEL_X, y + STATE_LINE_H);
            _tft.print(_reason);
        }
    }

    // --- Tile list: 3 per row, max TILE_MAX ---
    const int leftmost = TILE_LIST_CENTER_X - (TILES_PER_ROW * TILE_W + (TILES_PER_ROW - 1) * TILE_GAP) / 2;

    int tileY = TILE_LIST_Y;
    int col   = 0;
    int drawn = 0;

    for (int r = 0; r < GRID_ROWS && drawn < TILE_MAX; r++) {
        for (int c = 0; c < GRID_COLS && drawn < TILE_MAX; c++) {
            if (!_grid[r][c]) continue;

            drawTile(leftmost + col * (TILE_W + TILE_GAP), tileY, r, c);
            drawn++;
            if (++col >= TILES_PER_ROW) {
                col = 0;
                tileY += TILE_ROW_H;
            }
        }
    }
}

/**
 * @brief Draw one blocker coordinate as a white rounded-rect dice face with black text.
 *
 * The tile is TILE_W × TILE_H with corner radius TILE_R.
 * Text "A4" (letter=row A–F, digit=col 1–6) is centred inside at size 2.
 */
void SolverMenuScreen::drawTile(int x, int y, int r, int c) {
    _tft.fillRoundRect(x, y, TILE_W, TILE_H, TILE_R, COL_TILE_BG);

    char buf[3] = { (char)('A' + r), (char)('1' + c), '\0' };
    _tft.setTextColor(COL_TILE_TEXT, COL_TILE_BG);
    _tft.setTextSize(2);
    int tx = x + (TILE_W - 22) / 2;   // 2 chars × 12px = 24px
    int ty = y + (TILE_H - 16) / 2;   // text height at size 2 = 16px
    _tft.setCursor(tx, ty);
    _tft.print(buf);
}
