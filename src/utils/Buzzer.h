/*
 * Created by Ed Fillingham on 02/06/2026.
 *
 * Non-blocking buzzer driver. Plays predefined sound effect sequences
 * via the ESP32-S3 ledc peripheral. Call update() every loop iteration.
 */

#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

enum class SoundEffect : uint8_t {
    POWER_ON,
    MENU_TICK,
    MENU_SELECT,
    COUNTDOWN_TICK,
    GAME_START,
    PIECE_PLACED,
    WIN,
    NEW_BEST,
    POWER_OFF,
};

class Buzzer {
public:
    explicit Buzzer(uint8_t pin);

    /** @brief Initialise ledc peripheral. Call once in setup(). */
    void begin();

    /**
     * @brief Start playing a sound effect, interrupting any current sound.
     * @param effect The sound effect to play.
     */
    void play(SoundEffect effect);

    /** @brief Advance the note sequencer. Call every loop() iteration. */
    void update();

    struct Note {
        uint16_t freq;        ///< Hz; 0 = silence
        uint16_t durationMs;
    };

private:
    static constexpr uint8_t  RESOLUTION = 8;
    static constexpr uint8_t  DUTY       = 128; ///< 50% duty — max loudness
    static constexpr uint8_t  QUEUE_SIZE = 8;

    uint8_t       _pin;
    Note          _queue[QUEUE_SIZE];
    uint8_t       _head;
    uint8_t       _count;
    unsigned long _noteStartMs;
    bool          _playing;

    void enqueue(const Note* notes, uint8_t len);
    void startNote(const Note& note);
    void silence();
};

#endif // BUZZER_H
