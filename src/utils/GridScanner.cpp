/*
 * Created by Ed Fillingham on 09/05/2026.
 *
 * Implementation of the GridScanner hardware abstraction.
 */

#include "GridScanner.h"
#include <Arduino.h>
#include <driver/gpio.h>

static const int ROW_PINS[] = { 14, 47, 48, 38, 41, 42 };
static const int COL_PINS[] = {  7,  8, 10,  9,  1,  2 };
static const int NUM_ROWS   = sizeof(ROW_PINS) / sizeof(ROW_PINS[0]);
static const int NUM_COLS   = sizeof(COL_PINS) / sizeof(COL_PINS[0]);

static const int   ROW_SETTLE_US = 800;
static const int   ADC_PRIME_US  = 50;
static const int   ROW_DRAIN_US  = 100;
static const int   ADC_SETTLE_US = 10;
static const float ADC_MAX_F     = 4095.0f;

void GridScanner::init() {
    analogSetAttenuation(ADC_11db);

    for (int i = 0; i < NUM_ROWS; i++) {
        pinMode(ROW_PINS[i], OUTPUT);
        digitalWrite(ROW_PINS[i], HIGH);
    }

    for (int i = 0; i < NUM_COLS; i++) {
        pinMode(COL_PINS[i], ANALOG);
        gpio_pulldown_dis((gpio_num_t)COL_PINS[i]);
        gpio_pullup_dis((gpio_num_t)COL_PINS[i]);
    }
}

std::vector<std::vector<float>> GridScanner::analogReadMatrix() {
    std::vector<std::vector<float>> matrix(NUM_ROWS, std::vector<float>(NUM_COLS, 0.0f));

    for (int r = 0; r < NUM_ROWS; r++) {
        digitalWrite(ROW_PINS[r], LOW);
        delayMicroseconds(ROW_SETTLE_US);

        // Prime the ADC MUX after the voltage swing on this row
        analogRead(COL_PINS[0]);
        delayMicroseconds(ADC_PRIME_US);

        for (int c = 0; c < NUM_COLS; c++) {
            // Throwaway read to clear ADC capacitor from previous channel
            analogRead(COL_PINS[c]);
            delayMicroseconds(ADC_SETTLE_US);
            matrix[r][c] = analogRead(COL_PINS[c]) / ADC_MAX_F;
        }

        digitalWrite(ROW_PINS[r], HIGH);
        delayMicroseconds(ROW_DRAIN_US);
    }

    return matrix;
}

std::vector<std::vector<bool>> GridScanner::digitalReadMatrix(float threshold) {
    auto analog = analogReadMatrix();
    std::vector<std::vector<bool>> result(NUM_ROWS, std::vector<bool>(NUM_COLS, false));

    for (int r = 0; r < NUM_ROWS; r++) {
        for (int c = 0; c < NUM_COLS; c++) {
            result[r][c] = analog[r][c] >= threshold;
        }
    }

    return result;
}
