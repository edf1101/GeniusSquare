/*
 * Created by Ed Fillingham on 2026-05-09.
 *
 * Counts the number of solutions for each seed of the Genius Square puzzle.
 * Runs each seed for up to TIME_LIMIT_SECONDS, saves results to results.txt.
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <climits>

#include "solver/Board.h"
#include "solver/CombinationGenerator.h"

static const int TIME_LIMIT_SECONDS = 30;
static const int NUM_SEEDS = 100;
static const char* OUTPUT_FILE = "results.txt";

int main() {
    std::ofstream outFile(OUTPUT_FILE);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open " << OUTPUT_FILE << "\n";
        return 1;
    }

    for (int seed = 0; seed < NUM_SEEDS; seed++) {
        std::vector<Coord> blockers = CombinationGenerator::generateCombinations(seed);

        Board board(blockers, INT_MAX, TIME_LIMIT_SECONDS);

        auto wallStart = std::chrono::steady_clock::now();
        board.solve();
        auto wallEnd = std::chrono::steady_clock::now();

        int count = (int)board.getSolutions().size();
        outFile << count << "\n";
        outFile.flush();
    }

    outFile.close();
    return 0;
}
