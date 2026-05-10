/*
 * Created by Ed Fillingham on 10/05/2026.
 *
 * Persistent best-score storage for practice arrangements.
 *
 * Layout: one byte per arrangement index, value = best time in whole seconds.
 * 0 means no score recorded. Max storable time is 255 s.
 */

#ifndef PRACTICE_SCORES_H
#define PRACTICE_SCORES_H

#include <stdint.h>

namespace PracticeScores {

    /**
     * @brief Initialise EEPROM backing store. Call once in setup() before load().
     * @param count Number of arrangements (bytes to reserve).
     */
    void init(uint8_t count);

    /**
     * @brief Read the stored best time for an arrangement.
     * @param index Arrangement index (0-based).
     * @return Best time in whole seconds, or 0 if no score recorded.
     */
    uint8_t load(uint8_t index);

    /**
     * @brief Persist a new best time for an arrangement.
     * @param index   Arrangement index (0-based).
     * @param seconds Best time in whole seconds (clamped to 255).
     */
    void save(uint8_t index, uint8_t seconds);

} // namespace PracticeScores

#endif // PRACTICE_SCORES_H
