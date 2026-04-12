/*
 * Created by Ed Fillingham on 12/04/2026.
 *
 * A Screen that displays a scrollable vertical list of options.
 * Suitable for large option sets (50+ items). The first entry is always
 * an auto-injected "Back" item that calls ScreenManager::pop().
 */

#ifndef LIST_MENU_SCREEN_H
#define LIST_MENU_SCREEN_H

#include <TFT_eSPI.h>
#include "menu/Screen.h"
#include "menu/ListItem.h"
#include "menu/ScreenManager.h"

class ListMenuScreen : public Screen {
public:
    /**
     * @brief Construct a ListMenuScreen.
     * @param tft     Reference to the shared TFT display.
     * @param manager Reference to the ScreenManager.
     * @param items   Pointer to real item array (Back not included). Must outlive this object.
     * @param count   Number of real items (not including Back).
     * @param title   Text displayed in the title bar.
     */
    ListMenuScreen(TFT_eSPI& tft, ScreenManager& manager,
                   const ListItem* items, uint8_t count,
                   const char* title);

    void onEnter() override;
    void onExit() override;
    void onResume() override;
    void update() override;
    void render() override;
    void onEncoderChange(int delta) override;
    void onButtonPress() override;

    /// Number of rows visible at once. Adjust here to change list density.
    static constexpr uint8_t VISIBLE_ROWS = 4;

private:
    TFT_eSPI&        _tft;
    ScreenManager&   _manager;
    const ListItem*  _items;   ///< Real items (Back not included)
    uint8_t          _count;   ///< Number of real items
    const char*      _title;

    uint8_t  _selectedIndex;   ///< Absolute index of highlighted item (0 = Back)
    uint8_t  _viewStart;       ///< Absolute index of first visible row

    // ---- Animation state ----
    float         _borderPhase;          ///< Sine wave phase (radians)
    float         _borderEnvelope;       ///< Fade-in envelope [0.0, 1.0]
    float         _lastBorderThickness;  ///< -1.0 = no border currently drawn
    uint16_t      _lastBorderColor;      ///< Colour from previous frame (change detection)
    unsigned long _lastMs;               ///< Timestamp of last update() call
    bool          _dirty;                ///< True when render() should call updateNumberBoxBorder()

    // ---- Layout constants (landscape 280×240) ----
    static constexpr int SCR_W        = 280;
    static constexpr int SCR_H        = 240;
    static constexpr int TITLE_H      = 30;
    static constexpr int ROW_H        = (SCR_H - TITLE_H) / VISIBLE_ROWS; ///< 52px; 2px gap at screen bottom is intentional (black bg, invisible)
    static constexpr int NUM_TEXT_SIZE = 3;   ///< TFT_eSPI text scale for row numbers (18×24px per char)
    static constexpr int NUM_COL_W    = 50;   ///< Width of the left number column
    static constexpr int NUM_BOX_SIZE = 40;   ///< Side length of the number box square

    // ---- Border animation constants ----
    static constexpr float    BORDER_T_MIN       = 1.0f;   ///< Minimum glow border thickness (px)
    static constexpr float    BORDER_T_MAX       = 3.0f;   ///< Maximum glow border thickness (px)
    static constexpr float    BORDER_SPEED       = 4.0f;   ///< Phase advance rate (rad/sec)
    static constexpr float    BORDER_INTRO_SPEED = 8.0f;   ///< Envelope fade-in rate (~125ms to full)

    // ---- Colour palette (RGB565) ----
    static constexpr uint16_t COL_BG         = 0x0000; ///< Background — black
    static constexpr uint16_t COL_TEXT_SEL   = 0xFFFF; ///< Selected row text — white
    static constexpr uint16_t COL_TEXT_UNSEL = 0xAD75; ///< Unselected row text — grey
    static constexpr uint16_t COL_DIVIDER    = 0x2965; ///< Row divider line — dark grey
    static constexpr uint16_t COL_NUM_BOX    = 0xFFFF; ///< Number box outline — white
    static constexpr uint16_t COL_BORDER     = 0xFFFF; ///< Animated glow border — white
    static constexpr uint16_t COL_TITLE      = 0xAD75; ///< Title bar text — light grey

    /** @brief Total navigable items including the auto-injected Back entry. */
    uint16_t totalItems() const;

    /**
     * @brief Return the display label for an absolute index.
     *        Index 0 returns "Back"; indices 1..count return _items[index-1].label.
     */
    const char* labelFor(uint8_t absIndex) const;

    /** @brief Draw the title bar. Called once on onEnter() and onResume(). */
    void drawTitleBar();

    /** @brief Redraw all visible rows and dividers from current _selectedIndex / _viewStart. */
    void drawRowArea();

    /**
     * @brief Draw a single row.
     * @param screenRow On-screen row index (0..VISIBLE_ROWS-1).
     * @param absIndex  Absolute item index this row represents.
     * @param selected  Whether this row is the highlighted selection.
     */
    void drawRow(uint8_t screenRow, uint8_t absIndex, bool selected);

    /**
     * @brief Update the animated glow border on the selected row's number box.
     *        Only issues TFT commands when thickness or colour has changed.
     *        Called from render() every frame.
     */
    void updateNumberBoxBorder();

    /**
     * @brief Draw (or erase) the glow border around the number box.
     * @param boxX      Left edge of the number box.
     * @param boxY      Top edge of the number box.
     * @param thickness Float thickness; drawn as round(thickness) concentric drawRect calls.
     * @param colour    Border colour (use COL_BG to erase).
     */
    void drawNumberBoxBorder(int boxX, int boxY, float thickness, uint16_t colour);

    /**
     * @brief Blend two RGB565 colours by linear interpolation.
     * @param bg    Background colour.
     * @param fg    Foreground colour.
     * @param alpha Blend factor [0.0, 1.0].
     * @return Blended colour.
     */
    uint16_t blendColor(uint16_t bg, uint16_t fg, float alpha);
};

#endif // LIST_MENU_SCREEN_H
