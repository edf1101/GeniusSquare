/*
 * Created by Ed Fillingham on 10/05/2026.
 *
 * Three-phase practice game screen: PLACING → COUNTDOWN → PLAYING.
 *
 * Phase PLACING: all 7 blocker positions are permanently displayed as beige circles.
 *   The player fills them in any order. A red-X error is shown only for pieces placed
 *   in non-blocker positions. Advances to COUNTDOWN when all 7 positions are occupied.
 *
 * Phase COUNTDOWN: brief visual countdown (COUNTDOWN_SECONDS) before timer starts.
 *
 * Phase PLAYING: counts up a timer while the player fills the board with puzzle pieces.
 *   When all 36 cells are occupied the timer stops and PracticeScoreScreen is pushed.
 *
 * Call setArrangement() before pushing this screen to configure the arrangement.
 */

#ifndef PRACTICE_GAME_SCREEN_H
#define PRACTICE_GAME_SCREEN_H

#include <TFT_eSPI.h>
#include <vector>
#include "menu/Screen.h"
#include "menu/ScreenManager.h"
#include "menu/BorderAnimator.h"
#include "../utils/maths.h"

class PracticeScoreScreen;  // forward declaration — full include in .cpp

class PracticeGameScreen : public Screen {
public:
    /**
     * @brief Construct the game screen.
     * @param tft         Shared display reference.
     * @param manager     Shared ScreenManager reference.
     * @param scoreScreen Score screen pushed when the game ends.
     */
    PracticeGameScreen(TFT_eSPI& tft, ScreenManager& manager,
                       PracticeScoreScreen& scoreScreen);

    /**
     * @brief Configure the arrangement before pushing. Resets all phase state.
     * @param index       Index of this arrangement in practiceItems[].
     * @param blockers    The 7 blocker coordinates (any order accepted).
     * @param bestSeconds Pointer to practiceItems[index].seconds for in-RAM best update.
     */
    void setArrangement(uint8_t index, const std::vector<Coord>& blockers, float* bestSeconds);

    void onEnter()            override;
    void onExit()             override;
    void onResume()           override;
    void update()             override;
    void render()             override;
    void onEncoderChange(int delta) override;
    void onButtonPress()      override;

private:
    enum class Phase : uint8_t { PLACING, COUNTDOWN, PLAYING };

    TFT_eSPI&            _tft;
    ScreenManager&       _manager;
    PracticeScoreScreen& _scoreScreen;

    uint8_t              _arrangementIndex;
    std::vector<Coord>   _blockers;
    float*               _bestSeconds;

    Phase                _phase;
    uint8_t              _confirmedCount;  ///< Blocker positions occupied so far (0–7)

    bool                 _grid[6][6];
    bool                 _cellDirty[6][6];
    bool                 _panelDirty;
    bool                 _timerDirty;     ///< Only the timer digit strip needs redraw

    bool                 _hasError;
    uint8_t              _errorCol;
    uint8_t              _errorRow;

    float                _elapsedSeconds;
    unsigned long        _timerStartMs;
    unsigned long        _lastScanMs;
    unsigned long        _lastFrameMs;
    unsigned long        _lastMs;         ///< millis() at last update() — used for border dt

    uint8_t              _countdownRemaining;
    unsigned long        _countdownStartMs;

    BorderAnimator       _border;

    // ---- Layout (landscape 280×240, matching SolverMenuScreen) ----
    static constexpr int SCR_W       = 280;
    static constexpr int SCR_H       = 240;
    static constexpr int TITLE_H     = 30;
    static constexpr int MARGIN_X    = 14;

    // Back button (matches solver: 10px below title, 32px tall, grid-width)
    static constexpr int BTN_Y       = TITLE_H + 10;   ///< 40
    static constexpr int BTN_H       = 32;
    static constexpr int CELL_SIZE   = 24;
    static constexpr int GRID_ROWS   = 6;
    static constexpr int GRID_COLS   = 6;
    static constexpr int GRID_SIZE   = CELL_SIZE * GRID_ROWS;   ///< 144 px
    static constexpr int BTN_W       = GRID_SIZE;               ///< 144 — matches grid width
    static constexpr int GRID_LINE_W = 2;
    static constexpr int GRID_X      = MARGIN_X;                ///< 14
    static constexpr int GRID_Y      = BTN_Y + BTN_H + 8;      ///< 80
    static constexpr int PANEL_X     = GRID_X + GRID_SIZE + GRID_LINE_W; ///< 160
    static constexpr int PANEL_W     = SCR_W - PANEL_X;         ///< 120
    static constexpr int PANEL_Y     = GRID_Y;                  ///< 80

    static constexpr unsigned long SCAN_INTERVAL_MS  = 200;
    static constexpr uint8_t       COUNTDOWN_SECONDS = 3;

    // ---- Colours ----
    static constexpr uint16_t COL_BG        = 0x0000;
    static constexpr uint16_t COL_TEXT      = 0xFFFF;
    static constexpr uint16_t COL_SUBTEXT   = 0xAD75;
    static constexpr uint16_t COL_TITLE     = 0xAD75;
    static constexpr uint16_t COL_GRID_LINE = 0x4A49;
    static constexpr uint16_t COL_BLOCKER   = 0xDDB4;  ///< Blocker circle — beige
    static constexpr uint16_t COL_PIECE     = 0x8410;  ///< Puzzle piece — mid grey
    static constexpr uint16_t COL_TARGET    = 0xFFE0;  ///< Unplaced blocker target — yellow
    static constexpr uint16_t COL_ERROR     = 0xF800;  ///< Wrong cell — red
    static constexpr uint16_t COL_BORDER    = 0xFFFF;  ///< Button glow border

    // ---- Drawing ----
    void drawTitleBar();
    void drawBackButton();
    void drawGridLines();
    void drawCell(uint8_t row, uint8_t col);
    void drawPanel();
    void drawTimerArea();
    void drawAllCells();
    void updateBorder();

    // ---- Scan / logic ----
    void scanGrid();
    void finishGame();

    // ---- Helpers ----
    bool isBlockerCell(uint8_t row, uint8_t col) const;
    void formatTime(float seconds, char* buf, size_t bufSize) const;
};

#endif // PRACTICE_GAME_SCREEN_H
