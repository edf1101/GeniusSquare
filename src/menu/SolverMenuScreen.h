/*
 * Created by Ed Fillingham on 09/05/2026.
 *
 * Solver mode entry screen. Presents two horizontally split buttons (Solve / Back)
 * with an animated glow border on the selected one and a static dim border on the
 * other. Below the buttons:
 *   Left panel  — 6×6 live grid with fading blocker circles (GridScanner, 200 ms poll).
 *   Right panel — state (size 2) + reason (size 1) header followed by
 *                 blocker coordinates as white dice-face tiles, 3 per row (max 9).
 */

#ifndef SOLVER_MENU_SCREEN_H
#define SOLVER_MENU_SCREEN_H

#include <TFT_eSPI.h>
#include "menu/Screen.h"
#include "menu/ScreenManager.h"
#include "menu/BorderAnimator.h"
#include "solver/SolverTask.h"

class SolverMenuScreen : public Screen {
public:
    SolverMenuScreen(TFT_eSPI& tft, ScreenManager& manager);

    void onEnter() override;
    void onExit() override;
    void onResume() override;
    void update() override;
    void render() override;
    void onEncoderChange(int delta) override;
    void onButtonPress() override;

private:
    TFT_eSPI&      _tft;
    ScreenManager& _manager;

    uint8_t _selectedIndex;

    BorderAnimator _border;
    unsigned long  _lastMs;
    bool           _dirty;

    // ---- Grid / scan state ----
    bool  _grid[6][6];
    float _cellAlpha[6][6];
    bool  _cellDirty[6][6];
    bool  _panelDirty;
    bool  _solvable;
    char  _reason[12];          ///< "Too few", "Too many", "Invalid", or "" when solvable
    unsigned long _lastScanMs;

    // ---- Solver ----
    SolverTask          _solver;
    std::vector<Coord>  _lastBlockers;

    // ---- Layout: buttons ----
    static constexpr int SCR_W          = 280;
    static constexpr int SCR_H          = 240;
    static constexpr int TITLE_H        = 30;
    static constexpr int MARGIN_X       = 14;
    static constexpr int BUTTON_AREA_X  = MARGIN_X;
    static constexpr int BUTTON_AREA_W  = SCR_W - 2 * MARGIN_X;
    static constexpr int BUTTON_PADDING = 10;
    static constexpr int BUTTON_H       = 32;
    static constexpr int BUTTON_Y       = TITLE_H + BUTTON_PADDING;
    static constexpr int SOLVE_W        = (BUTTON_AREA_W * 3) / 4;
    static constexpr int BACK_X         = BUTTON_AREA_X + SOLVE_W;
    static constexpr int BACK_W         = BUTTON_AREA_W - SOLVE_W;

    // ---- Layout: grid ----
    static constexpr int   GRID_ROWS       = 6;
    static constexpr int   GRID_COLS       = 6;
    static constexpr int   GRID_TOP_PAD    = 8;
    static constexpr int   GRID_LINE_W     = 2;
    static constexpr int   CELL_SIZE       = 24;
    static constexpr int   GRID_SIZE       = CELL_SIZE * GRID_ROWS;
    static constexpr int   GRID_X          = BUTTON_AREA_X;
    static constexpr int   GRID_Y          = BUTTON_Y + BUTTON_H + GRID_TOP_PAD;
    static constexpr int   CIRCLE_R        = 8;
    static constexpr float FADE_DURATION_S = 0.1f;
    static constexpr unsigned long SCAN_INTERVAL_MS = 200;

    // ---- Layout: right panel ----
    // PANEL_X starts immediately after the rightmost grid line (no extra gap) so that
    // "Unsolvable" (10 chars × 12px = 120px) fits exactly to the screen edge (280px).
    static constexpr int PANEL_X = GRID_X + GRID_SIZE + GRID_LINE_W;  ///< 160px
    static constexpr int PANEL_W = SCR_W - PANEL_X;                   ///< 120px
    static constexpr int PANEL_Y = GRID_Y;

    // Status header: state word (size 2, 20px slot) + reason (size 1, 12px slot).
    // Tile list always starts at TILE_LIST_Y regardless of solvability.
    static constexpr int STATE_LINE_H  = 20;  ///< size-2 text (16px) + 4px gap
    static constexpr int REASON_LINE_H = 20;  ///< size-2 text (16px) + 4px gap
    static constexpr int TILE_LIST_Y   = PANEL_Y + STATE_LINE_H + REASON_LINE_H + 4;

    // Tile dice rendering — 3 per row, max 9 (3 rows).
    // *** Adjust TILE_LIST_CENTER_X to shift the tile columns left/right. ***
    static constexpr int TILE_LIST_CENTER_X = PANEL_X + PANEL_W / 2;  ///< 220 — adjustable
    static constexpr int TILE_W             = 28;   ///< Width of one tile rounded-rect
    static constexpr int TILE_H             = 22;   ///< Height of one tile rounded-rect
    static constexpr int TILE_R             = 4;    ///< Corner radius
    static constexpr int TILE_GAP           = 6;    ///< Horizontal gap between tiles
    static constexpr int TILE_ROW_H         = TILE_H + 4;  ///< Vertical pitch between tile rows (26px)
    static constexpr int TILES_PER_ROW      = 3;
    static constexpr int TILE_MAX           = 9;    ///< 3 rows × 3 per row

    // ---- Colour palette (RGB565) ----
    static constexpr uint16_t COL_BG           = 0x0000;
    static constexpr uint16_t COL_TEXT_SEL     = 0xFFFF;
    static constexpr uint16_t COL_TEXT_UNSEL   = 0xAD75;
    static constexpr uint16_t COL_DIVIDER      = 0x2965;
    static constexpr uint16_t COL_BORDER       = 0xFFFF;
    static constexpr uint16_t COL_BORDER_UNSEL = 0x4A49;
    static constexpr uint16_t COL_TITLE        = 0xAD75;
    static constexpr uint16_t COL_GRID_LINE    = 0x2965;
    static constexpr uint16_t COL_BLOCKER      = 0xDDB4;
    static constexpr uint16_t COL_SOLVABLE     = 0x07E0;   ///< Green
    static constexpr uint16_t COL_UNSOLVABLE   = 0xF800;   ///< Red
    static constexpr uint16_t COL_TILE_BG      = 0xFFFF;   ///< White dice face
    static constexpr uint16_t COL_TILE_TEXT    = 0x0000;   ///< Black text on tile

    // ---- Button helpers ----
    void drawTitleBar();
    void drawButtons();
    void drawButton(int x, int w, const char* label, bool selected);
    void drawDeselectedBorder(int x, int w);
    void eraseDeselectedBorder(int x, int w);
    void updateBorder();
    void eraseBorder(uint8_t index);
    int  buttonX(uint8_t index) const;
    int  buttonW(uint8_t index) const;

    // ---- Grid helpers ----
    void scanGrid();
    void drawGrid();
    void renderCellFaded(int r, int c, float alpha);

    // ---- Validity ----
    /** @brief Count blockers, run validCombination if count==6, update _solvable/_reason. */
    void checkValidity();

    // ---- Side panel ----
    void drawSidePanel();

    /**
     * @brief Draw one blocker coordinate as a white rounded-rect dice face.
     * @param x Pixel x of the tile's left edge.
     * @param y Pixel y of the tile's top edge.
     * @param r Grid row [0,5] — rendered as letter A–F.
     * @param c Grid col [0,5] — rendered as digit 1–6.
     */
    void drawTile(int x, int y, int r, int c);
};

#endif // SOLVER_MENU_SCREEN_H
