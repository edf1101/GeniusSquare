/*
 * Created by Ed Fillingham on 11/04/2026.
 *
 * Abstract interface that every application screen must implement.
 * All screens (menus, game, score, solver) share this contract.
 */

#ifndef SCREEN_H
#define SCREEN_H

class Screen {
public:
    /**
     * @brief Called when this screen is pushed onto the ScreenManager stack.
     *        Draw all static UI regions and initialise state here.
     */
    virtual void onEnter() = 0;

    /**
     * @brief Called when this screen is popped from the stack.
     *        Release any resources held by the screen.
     */
    virtual void onExit() = 0;

    /**
     * @brief Called when a child screen has been popped and this screen
     *        is once again on top. Redraw static regions without full reinitialisation.
     */
    virtual void onResume() = 0;

    /**
     * @brief Animation and state update tick. Called every loop iteration.
     */
    virtual void update() = 0;

    /**
     * @brief Draw the current frame to the TFT. Called every loop iteration.
     */
    virtual void render() = 0;

    /**
     * @brief Called when the rotary encoder value changes.
     * @param delta Signed change in encoder steps.
     */
    virtual void onEncoderChange(int delta) = 0;

    /**
     * @brief Called when the button is pressed.
     */
    virtual void onButtonPress() = 0;

    virtual ~Screen() = default;
};

#endif // SCREEN_H
