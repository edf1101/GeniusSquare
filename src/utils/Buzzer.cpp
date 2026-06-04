/*
 * Created by Ed Fillingham on 02/06/2026.
 *
 * Buzzer — non-blocking sound effect sequencer using ESP32-S3 ledc.
 */

#include "utils/Buzzer.h"

// ---- Frequency constants (Hz) ----
static constexpr uint16_t NOTE_C4  = 261;
static constexpr uint16_t NOTE_E4  = 330;
static constexpr uint16_t NOTE_G4  = 392;
static constexpr uint16_t NOTE_A4  = 440;
static constexpr uint16_t NOTE_C5  = 523;
static constexpr uint16_t NOTE_E5  = 659;
static constexpr uint16_t NOTE_G5  = 784;
static constexpr uint16_t NOTE_A5  = 880;
static constexpr uint16_t NOTE_B5  = 988;
static constexpr uint16_t NOTE_C6  = 1047;

// ---- Sound sequences ----
static const Buzzer::Note SEQ_POWER_ON[]      = {{NOTE_E5,150},{NOTE_G5,150},{NOTE_B5,200}};
static const Buzzer::Note SEQ_MENU_TICK[]     = {{NOTE_A4, 40}};
static const Buzzer::Note SEQ_MENU_SELECT[]   = {{NOTE_C5, 60},{NOTE_E5, 80}};
static const Buzzer::Note SEQ_COUNTDOWN[]     = {{NOTE_A4,120}};
static const Buzzer::Note SEQ_GAME_START[]    = {{NOTE_E5,100},{NOTE_A5,200}};
static const Buzzer::Note SEQ_PIECE_PLACED[]  = {{NOTE_G4, 50}};
static const Buzzer::Note SEQ_WIN[]           = {{NOTE_C5,100},{NOTE_E5,100},{NOTE_G5,200}};
static const Buzzer::Note SEQ_NEW_BEST[]      = {{NOTE_C5, 80},{NOTE_E5, 80},{NOTE_G5, 80},{NOTE_C6,250}};
static const Buzzer::Note SEQ_POWER_OFF[]     = {{NOTE_G4,100},{NOTE_E4,100},{NOTE_C4,200}};

Buzzer::Buzzer(uint8_t pin)
    : _pin(pin), _head(0), _count(0), _noteStartMs(0), _playing(false)
{
    memset(_queue, 0, sizeof(_queue));
}

void Buzzer::begin() {
    ledcAttach(_pin, 2000, RESOLUTION);
    ledcWrite(_pin, 0);
}

void Buzzer::play(SoundEffect effect) {
    silence(); // stop any current note immediately
    _head  = 0;
    _count = 0;

    switch (effect) {
        case SoundEffect::POWER_ON:
            enqueue(SEQ_POWER_ON,     sizeof(SEQ_POWER_ON)     / sizeof(Note)); break;
        case SoundEffect::MENU_TICK:
            enqueue(SEQ_MENU_TICK,    sizeof(SEQ_MENU_TICK)    / sizeof(Note)); break;
        case SoundEffect::MENU_SELECT:
            enqueue(SEQ_MENU_SELECT,  sizeof(SEQ_MENU_SELECT)  / sizeof(Note)); break;
        case SoundEffect::COUNTDOWN_TICK:
            enqueue(SEQ_COUNTDOWN,    sizeof(SEQ_COUNTDOWN)    / sizeof(Note)); break;
        case SoundEffect::GAME_START:
            enqueue(SEQ_GAME_START,   sizeof(SEQ_GAME_START)   / sizeof(Note)); break;
        case SoundEffect::PIECE_PLACED:
            enqueue(SEQ_PIECE_PLACED, sizeof(SEQ_PIECE_PLACED) / sizeof(Note)); break;
        case SoundEffect::WIN:
            enqueue(SEQ_WIN,          sizeof(SEQ_WIN)          / sizeof(Note)); break;
        case SoundEffect::NEW_BEST:
            enqueue(SEQ_NEW_BEST,     sizeof(SEQ_NEW_BEST)     / sizeof(Note)); break;
        case SoundEffect::POWER_OFF:
            enqueue(SEQ_POWER_OFF,    sizeof(SEQ_POWER_OFF)    / sizeof(Note)); break;
    }
}

void Buzzer::update() {
    if (!_playing) return;

    if (millis() - _noteStartMs >= _queue[_head].durationMs) {
        _head  = (_head + 1) % QUEUE_SIZE;
        _count--;
        if (_count == 0) {
            _playing = false;
            silence();
            return;
        }
        startNote(_queue[_head]);
    }
}

// ---- Private ----

void Buzzer::enqueue(const Note* notes, uint8_t len) {
    if (len == 0) return;
    uint8_t tail = (_head + _count) % QUEUE_SIZE;
    for (uint8_t i = 0; i < len && _count < QUEUE_SIZE; i++) {
        _queue[tail] = notes[i];
        tail = (tail + 1) % QUEUE_SIZE;
        _count++;
    }
    startNote(_queue[_head]);
    _playing = true;
}

void Buzzer::startNote(const Note& note) {
    _noteStartMs = millis();
    if (note.freq == 0) {
        ledcWrite(_pin, 0);
    } else {
        ledcWriteTone(_pin, note.freq);
        ledcWrite(_pin, DUTY);
    }
}

void Buzzer::silence() {
    ledcWrite(_pin, 0);
}
