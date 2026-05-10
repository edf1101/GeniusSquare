/*
 * Created by Ed Fillingham on 09/05/2026.
 *
 * SolverMenuScreen — single Back button, auto-solving on valid arrangements.
 *
 * Layout (landscape 280×240):
 * - Title bar: 30px
 * - Back button (32px, 10px below title, grid-width): x=14, w=144
 * - Left (x=14, y=80): 144×144 live grid, beige circles fade 100 ms
 * - Right (x=160, 120px wide):
 *     Status line centred at button row: reason (red) or Solving.../Solved!/No sln (green/red)
 *     Blocker tile list from y=80: white rounded-rect dice faces, 2 per row, max 8
 */

#include "menu/SolverMenuScreen.h"
#include "../utils/GridScanner.h"
#include "solver/CombinationGenerator.h"
#include "solver/Piece.h"
#include <math.h>
#include <string.h>
#include <vector>

SolverMenuScreen::SolverMenuScreen(TFT_eSPI& tft, ScreenManager& manager)
    : _tft(tft), _manager(manager),
      _selectedIndex(0),
      _lastMs(millis()), _dirty(false),
      _panelDirty(false), _solvable(false),
      _lastScanMs(0),
      _showSolution(false), _wasDone(true),
      _spinnerDots(0), _spinnerMs(0)
{
    memset(_grid,        0, sizeof(_grid));
    memset(_cellAlpha,   0, sizeof(_cellAlpha));
    memset(_cellDirty,   0, sizeof(_cellDirty));
    memset(_solutionGrid,0, sizeof(_solutionGrid));
    memset(_pieceDirty,  0, sizeof(_pieceDirty));
    _reason[0] = '\0';
}

int SolverMenuScreen::buttonX(uint8_t /*index*/) const { return BACK_X; }
int SolverMenuScreen::buttonW(uint8_t /*index*/) const { return BACK_W; }

// ---- Lifecycle ----

void SolverMenuScreen::onEnter() {
    _selectedIndex = 0;
    _border.reset();
    _lastMs      = millis();
    _dirty       = false;
    _panelDirty  = false;
    _lastScanMs  = millis() - SCAN_INTERVAL_MS;

    memset(_grid,        0, sizeof(_grid));
    memset(_cellAlpha,   0, sizeof(_cellAlpha));
    memset(_cellDirty,   0, sizeof(_cellDirty));
    memset(_solutionGrid,0, sizeof(_solutionGrid));
    memset(_pieceDirty,  0, sizeof(_pieceDirty));
    _reason[0]     = '\0';
    _showSolution  = false;
    _wasDone       = true;
    _spinnerDots   = 0;
    _spinnerMs     = millis();

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

    memset(_grid,        0, sizeof(_grid));
    memset(_cellAlpha,   0, sizeof(_cellAlpha));
    memset(_cellDirty,   0, sizeof(_cellDirty));
    memset(_solutionGrid,0, sizeof(_solutionGrid));
    memset(_pieceDirty,  0, sizeof(_pieceDirty));
    _reason[0]     = '\0';
    _showSolution  = false;
    _wasDone       = true;
    _spinnerDots   = 0;
    _spinnerMs     = millis();

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

    // Solution-copy: detect isDone() rising edge
    bool nowDone = _solver.isDone();
    if (nowDone && !_wasDone) {
        if (_solver.hasSolution()) {
            auto grid = _solver.getSolutionGrid();
            for (int r = 0; r < GRID_ROWS; r++)
                for (int c = 0; c < GRID_COLS; c++) {
                    _solutionGrid[r][c] = grid[r][c];
                    _pieceDirty[r][c]   = true;
                }
            _showSolution = true;
        }
        _panelDirty = true;
    }
    _wasDone = nowDone;

    // Spinner: advance dot count while solve is in progress
    if (_solvable && !nowDone) {
        if (now - _spinnerMs >= SPINNER_INTERVAL_MS) {
            _spinnerMs   = now;
            _spinnerDots = (_spinnerDots + 1) % 4;
            _panelDirty  = true;
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
            if (_pieceDirty[r][c]) {
                _pieceDirty[r][c] = false;
                renderCellPiece(r, c);
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

void SolverMenuScreen::onEncoderChange(int /*delta*/) {}

void SolverMenuScreen::onButtonPress() {
    _manager.pop();
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
    drawButton(BACK_X, BACK_W, "Back", true);
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
        // Added + GRID_LINE_W to the length so the grid line ends perfectly overlap
        _tft.fillRect(GRID_X,                 GRID_Y + i * CELL_SIZE, GRID_SIZE + GRID_LINE_W,   GRID_LINE_W, COL_GRID_LINE);
        _tft.fillRect(GRID_X + i * CELL_SIZE, GRID_Y,                 GRID_LINE_W, GRID_SIZE + GRID_LINE_W,   COL_GRID_LINE);
    }
}

void SolverMenuScreen::renderCellFaded(int r, int c, float alpha) {
    int cellX = GRID_X + c * CELL_SIZE;
    int cellY = GRID_Y + r * CELL_SIZE;

    // Always clear the cell interior first. This guarantees that when alpha hits 0,
    // the faint ghost circle from the previous frame is perfectly erased.
    _tft.fillRect(cellX + GRID_LINE_W, cellY + GRID_LINE_W,
                  CELL_SIZE - GRID_LINE_W, CELL_SIZE - GRID_LINE_W, COL_BG);

    if (alpha > 0.02f) {
        uint16_t col = (alpha >= 0.99f) ? COL_BLOCKER
                                        : BorderAnimator::blendColor(COL_BG, COL_BLOCKER, alpha);
        _tft.fillCircle(cellX + CELL_SIZE / 2, cellY + CELL_SIZE / 2, CIRCLE_R, col);
    }
}

void SolverMenuScreen::renderCellPiece(int r, int c) {
    int cellX = GRID_X + c * CELL_SIZE;
    int cellY = GRID_Y + r * CELL_SIZE;
    int rx    = cellX + GRID_LINE_W + PIECE_PAD;
    int ry    = cellY + GRID_LINE_W + PIECE_PAD;

    if (_showSolution && _solutionGrid[r][c] != 0 && !_grid[r][c]) {
        Colour col    = Piece::getPieceColourByUUID(_solutionGrid[r][c]);
        uint16_t c565 = _tft.color565(col.r, col.g, col.b);
        uint16_t c565Darker = _tft.color565(col.r*0.85, col.g*0.85, col.b*0.85);
        _tft.fillRoundRect(rx, ry, PIECE_RECT_SIZE, PIECE_RECT_SIZE, PIECE_RECT_R, c565);

        // Left connector
        if (c > 0 && _solutionGrid[r][c-1] == _solutionGrid[r][c] && !_grid[r][c-1]) {
            _tft.fillRect(rx - CONNECTOR_GAP, ry + CONNECTOR_OFFSET, CONNECTOR_GAP, CONNECTOR_SIZE, c565Darker);
        }
        // Right connector
        if (c < GRID_COLS - 1 && _solutionGrid[r][c+1] == _solutionGrid[r][c] && !_grid[r][c+1]) {
            _tft.fillRect(rx + PIECE_RECT_SIZE, ry + CONNECTOR_OFFSET, CONNECTOR_GAP, CONNECTOR_SIZE, c565Darker);
        }
        // Top connector
        if (r > 0 && _solutionGrid[r-1][c] == _solutionGrid[r][c] && !_grid[r-1][c]) {
            _tft.fillRect(rx + CONNECTOR_OFFSET, ry - CONNECTOR_GAP, CONNECTOR_SIZE, CONNECTOR_GAP, c565Darker);
        }
        // Bottom connector
        if (r < GRID_ROWS - 1 && _solutionGrid[r+1][c] == _solutionGrid[r][c] && !_grid[r+1][c]) {
            _tft.fillRect(rx + CONNECTOR_OFFSET, ry + PIECE_RECT_SIZE, CONNECTOR_SIZE, CONNECTOR_GAP, c565Darker);
        }

    } else {
        // --- CLEARING LOGIC ---

        // 1. Only erase the cell interior if it's NOT a blocker.
        // If it is a blocker, renderCellFaded handles the fading circle/interior.
        if (!_grid[r][c]) {
            _tft.fillRect(cellX + GRID_LINE_W, cellY + GRID_LINE_W,
                          CELL_SIZE - GRID_LINE_W, CELL_SIZE - GRID_LINE_W, COL_BG);
        }

        // 2. ALWAYS restore the grid lines.
        // Even if this cell is now a blocker, the old solution might have drawn
        // connectors over the grid lines separating it from its neighbor.
        _tft.fillRect(cellX, cellY, GRID_LINE_W, CELL_SIZE + GRID_LINE_W, COL_GRID_LINE);  // Left
        _tft.fillRect(cellX, cellY, CELL_SIZE + GRID_LINE_W, GRID_LINE_W, COL_GRID_LINE);  // Top
        if (c < GRID_COLS - 1)
            _tft.fillRect(GRID_X + (c+1)*CELL_SIZE, cellY, GRID_LINE_W, CELL_SIZE + GRID_LINE_W, COL_GRID_LINE);  // Right
        if (r < GRID_ROWS - 1)
            _tft.fillRect(cellX, GRID_Y + (r+1)*CELL_SIZE, CELL_SIZE + GRID_LINE_W, GRID_LINE_W, COL_GRID_LINE);  // Bottom
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
            clearSolution();
            _wasDone    = true;   // reset edge detector for new solve
            _spinnerDots = 0;
            _spinnerMs   = millis();
            _lastBlockers = coords;
            _solver.start(coords);
        }
    } else {
        if (!_lastBlockers.empty()) {
            Serial.println("[SolverMenu] Arrangement changed/invalid, stopping solve");
            clearSolution();
            _solver.stop();
            _lastBlockers.clear();
        }
    }
}

void SolverMenuScreen::clearSolution() {
    memset(_solutionGrid, 0, sizeof(_solutionGrid));
    if (!_showSolution) return;
    _showSolution = false;
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++)
            _pieceDirty[r][c] = true;
    _panelDirty = true;
}

// ---- Side panel ----

void SolverMenuScreen::drawSidePanel() {
    _tft.fillRect(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, COL_BG);

    // --- Single status line centred at the Back button row ---
    static const char* spinLabels[4] = { "Solving.", "Solving..", "Solving...", "Solving.." };
    const char* label;
    uint16_t    labelCol;
    if (!_solvable) {
        label    = (_reason[0] != '\0') ? _reason : "...";
        labelCol = COL_UNSOLVABLE;
    } else if (!_solver.isDone()) {
        label    = spinLabels[_spinnerDots];
        labelCol = COL_SOLVABLE;
    } else if (_showSolution) {
        label    = "Solved!";
        labelCol = COL_SOLVABLE;
    } else {
        label    = "No sln";
        labelCol = COL_UNSOLVABLE;
    }
    _tft.setTextSize(2);
    _tft.setTextColor(labelCol, COL_BG);
    int statusX = PANEL_X + (PANEL_W - (int)strlen(label) * 12) / 2;
    int statusY = BUTTON_Y + (BUTTON_H - 16) / 2;
    _tft.setCursor(statusX, statusY);
    _tft.print(label);

    // --- Tile list: 2 per row, max TILE_MAX ---
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
    int tx = x + (TILE_W - 24) / 2;   // 2 chars × 12px = 24px
    int ty = y + (TILE_H - 16) / 2;   // text height at size 2 = 16px
    _tft.setCursor(tx, ty);
    _tft.print(buf);
}
