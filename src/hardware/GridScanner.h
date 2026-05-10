/*
 * Created by Ed Fillingham on 09/05/2026.
 *
 * Hardware abstraction for the 6×6 resistive sensor matrix.
 */

#ifndef GENIUS_SQUARE_GRID_SCANNER_H
#define GENIUS_SQUARE_GRID_SCANNER_H

#include <vector>

namespace GridScanner {

    /**
     * @brief Initialise sensor matrix hardware.
     *
     * Sets ADC attenuation, row pins as OUTPUT (idle HIGH),
     * and column pins as ANALOG with pull-ups/pull-downs disabled.
     * Call once from setup().
     */
    void init();

    /**
     * @brief Scan the matrix and return normalised ADC readings.
     *
     * @return 6×6 vector of floats in [0.0, 1.0] (raw ADC ÷ 4095).
     *         Outer index is row, inner index is column.
     */
    std::vector<std::vector<float>> analogReadMatrix();

    /**
     * @brief Scan the matrix and return boolean occupancy state.
     *
     * Calls analogReadMatrix() internally.
     *
     * @param threshold Values >= threshold map to true. Default 0.122f (≈ 500/4095).
     * @return 6×6 vector of bools. Outer index is row, inner index is column.
     */
    std::vector<std::vector<bool>> digitalReadMatrix(float threshold = 0.08f);

} // namespace GridScanner

#endif // GENIUS_SQUARE_GRID_SCANNER_H
