/*
 * Created by Ed Fillingham on 10/05/2026.
 *
 * PracticeScores — EEPROM-backed best-time persistence.
 */

#include "utils/PracticeScores.h"
#include <EEPROM.h>

static uint8_t _count = 0;

void PracticeScores::init(uint8_t count) {
    _count = count;
    EEPROM.begin(count);
}

uint8_t PracticeScores::load(uint8_t index) {
    if (index >= _count) return 0;
    uint8_t val = EEPROM.read(index);
    if (val ==255) return 0;  // treat uninitialised EEPROM (0xFF) as no score

    return val;
}

void PracticeScores::save(uint8_t index, uint8_t seconds) {
    if (index >= _count) return;
    EEPROM.write(index, seconds);
    EEPROM.commit();
}
