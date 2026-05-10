/*
 * Created by Ed Fillingham on 10/05/2026.
 *
 * Displays the result after a practice game: the player's time,
 * the current best for the arrangement, and a "New best!" banner if beaten.
 * A single button press pops both this screen and the game screen,
 * returning to the arrangement selection menu.
 */

#ifndef PRACTICE_SCORE_SCREEN_H
#define PRACTICE_SCORE_SCREEN_H

#include <TFT_eSPI.h>
#include "menu/Screen.h"
#include "menu/ScreenManager.h"
#include "menu/BorderAnimator.h"

class PracticeScoreScreen : public Screen {
public:
    PracticeScoreScreen(TFT_eSPI& tft, ScreenManager& manager);

    /**
     * @brief Set result data before pushing this screen.
     * @param elapsedSeconds Time the player took.
     * @param bestSeconds    Current best for this arrangement (0.0f = no score yet).
     * @param isNewBest      True if elapsedSeconds beat the previous best.
     */
    void setResult(float elapsedSeconds, float bestSeconds, bool isNewBest);

    void onEnter()            override;
    void onExit()             override;
    void onResume()           override;
    void update()             override;
    void render()             override;
    void onEncoderChange(int delta) override;
    void onButtonPress()      override;

private:
    TFT_eSPI&      _tft;
    ScreenManager& _manager;

    float _elapsedSeconds;
    float _bestSeconds;
    bool  _isNewBest;

    BorderAnimator _border;
    unsigned long  _lastMs;
    bool           _dirty;

    static constexpr int SCR_W   = 280;
    static constexpr int SCR_H   = 240;
    static constexpr int TITLE_H = 30;

    static constexpr uint16_t COL_BG       = 0x0000;
    static constexpr uint16_t COL_TEXT     = 0xFFFF;
    static constexpr uint16_t COL_SUBTEXT  = 0xAD75;
    static constexpr uint16_t COL_TITLE    = 0xAD75;
    static constexpr uint16_t COL_NEW_BEST = 0x07E0;
    static constexpr uint16_t COL_BORDER   = 0xFFFF;

    static constexpr int BTN_W = 140;
    static constexpr int BTN_H = 32;
    static constexpr int BTN_X = (SCR_W - BTN_W) / 2;
    static constexpr int BTN_Y = SCR_H - BTN_H - 20;

    void drawAll();
    void updateBorder();

    /**
     * @brief Format a time in seconds as "MM:SS" into buf.
     * @param seconds  Time in seconds.
     * @param buf      Output buffer (must hold at least 6 chars).
     * @param bufSize  Size of buf.
     */
    void formatTime(float seconds, char* buf, size_t bufSize) const;
};

#endif // PRACTICE_SCORE_SCREEN_H
