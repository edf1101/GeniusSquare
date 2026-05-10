/*
 * Created by Ed Fillingham on 09/05/2026.
 *
 * Solver mode entry screen. Single Back button (grid-width, left-aligned) with an
 * animated glow border. The solver runs automatically when a valid 7-blocker
 * arrangement is detected.
 *   Left panel  — 6×6 live grid with fading blocker circles (GridScanner, 200 ms poll).
 *   Right panel — state word + reason + blocker tile list, starting just below the title.
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

    bool  _locked;

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

    // ---- Solution display ----
    int   _solutionGrid[6][6];   ///< UUID per cell from getSolutionGrid(); 0 = empty
    bool  _showSolution;         ///< true once a solution has been copied and should be rendered
    bool  _wasDone;              ///< previous isDone() value — used to detect rising edge
    bool  _pieceDirty[6][6];    ///< cells needing a piece-rect redraw
    uint8_t       _spinnerDots;  ///< 0–3: maps to "Solving." / "Solving.." / "Solving..." / "Solving.."
    unsigned long _spinnerMs;    ///< millis() when _spinnerDots was last advanced

    // ---- Layout: buttons ----
    static constexpr int SCR_W          = 280;
    static constexpr int SCR_H          = 240;
    static constexpr int TITLE_H        = 30;
    static constexpr int MARGIN_X       = 14;
    static constexpr int BUTTON_PADDING = 10;
    static constexpr int BUTTON_H       = 32;
    static constexpr int BUTTON_Y       = TITLE_H + BUTTON_PADDING;

    // ---- Layout: grid ----
    static constexpr int   GRID_ROWS       = 6;
    static constexpr int   GRID_COLS       = 6;
    static constexpr int   GRID_TOP_PAD    = 8;
    static constexpr int   GRID_LINE_W     = 2;
    static constexpr int   CELL_SIZE       = 24;
    static constexpr int   GRID_SIZE       = CELL_SIZE * GRID_ROWS;

    static constexpr int   BTN_BACK_W      = 54;
    static constexpr int   BTN_GAP         = 10;
    static constexpr int   BTN_LOCK_X      = MARGIN_X + BTN_BACK_W + BTN_GAP;
    static constexpr int   BTN_LOCK_W      = GRID_SIZE - BTN_BACK_W - BTN_GAP; // 80px

    static constexpr int   GRID_X          = MARGIN_X;
    static constexpr int   GRID_Y          = BUTTON_Y + BUTTON_H + GRID_TOP_PAD;
    static constexpr int   CIRCLE_R        = 8;
    static constexpr float FADE_DURATION_S = 0.1f;
    static constexpr unsigned long SCAN_INTERVAL_MS = 200;
    static constexpr int   PIECE_PAD        = 2;   ///< px between grid line and piece rect edge
    static constexpr int   PIECE_RECT_SIZE  = CELL_SIZE - GRID_LINE_W - 2 * PIECE_PAD; ///< 18 px
    static constexpr int   PIECE_RECT_R     = 3;   ///< corner radius of piece rect
    static constexpr int   CONNECTOR_GAP    = GRID_LINE_W + 2 * PIECE_PAD; ///< 6px gap between adjacent rects
    static constexpr int   CONNECTOR_SIZE   = (PIECE_RECT_SIZE * 4) / 5;   ///< 80% of rect edge = 14px
    static constexpr int   CONNECTOR_OFFSET = (PIECE_RECT_SIZE - CONNECTOR_SIZE) / 2; ///< centering offset = 2px
    static constexpr unsigned long SPINNER_INTERVAL_MS = 400;

    // ---- Layout: right panel ----
    static constexpr int PANEL_X = GRID_X + GRID_SIZE + GRID_LINE_W;          ///< 160px
    static constexpr int PANEL_W = SCR_W - PANEL_X;                           ///< 120px
    static constexpr int PANEL_Y = TITLE_H + 4;                               ///< 34px — starts just below title
    static constexpr int PANEL_H = GRID_Y + GRID_SIZE + GRID_LINE_W - PANEL_Y; ///< 192px

    // Status text: single line centred at button row, then tile list from GRID_Y down.
    static constexpr int TILE_LIST_Y        = GRID_Y;  ///< 80px — aligns with grid top

    // Tile dice rendering — 2 per row, max 8 (4 rows).
    // *** Adjust TILE_LIST_CENTER_X to shift the tile columns left/right. ***
    static constexpr int TILE_LIST_CENTER_X = PANEL_X + PANEL_W / 2;  ///< 220 — adjustable
    static constexpr int TILE_W             = 32;   ///< Width of one tile rounded-rect
    static constexpr int TILE_H             = 32;   ///< Height of one tile rounded-rect
    static constexpr int TILE_R             = 4;    ///< Corner radius
    static constexpr int TILE_GAP           = 8;    ///< Horizontal gap between tiles
    static constexpr int TILE_ROW_H         = TILE_H + 4;  ///< Vertical pitch between tile rows (36px)
    static constexpr int TILES_PER_ROW      = 2;
    static constexpr int TILE_MAX           = 8;    ///< 4 rows × 2 per row

    // ---- Colour palette (RGB565) ----
    static constexpr uint16_t COL_BG           = 0x0000;
    static constexpr uint16_t COL_TEXT_SEL     = 0xFFFF;
    static constexpr uint16_t COL_TEXT_UNSEL   = 0xAD75;
    static constexpr uint16_t COL_DIVIDER      = 0x2965;
    static constexpr uint16_t COL_BORDER       = 0xFFFF;
    static constexpr uint16_t COL_BORDER_UNSEL = 0x4A49;
    static constexpr uint16_t COL_TITLE        = 0xAD75;
    static constexpr uint16_t COL_GRID_LINE    = 0x4A49;   ///< slightly brighter grid lines
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
    void renderCellPiece(int r, int c);
    void clearSolution();

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
