/*
 * Created by Ed Fillingham on 2026-05-09.
 *
 * Classifies solution counts from results.txt into 3 bands.
*/

#include <iostream>
#include <fstream>
#include <string>

static const char* INPUT_FILE = "results.txt";
static const char* OUTPUT_FILE = "classifications.txt";

static const int THRESHOLD_1 = 190;
static const int THRESHOLD_2 = 650;

int classifyCount(int count) {
    if (count <= THRESHOLD_1) return 1;
    if (count <= THRESHOLD_2) return 2;
    return 3;
}

int main() {
    std::ifstream inFile(INPUT_FILE);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open " << INPUT_FILE << "\n";
        return 1;
    }

    std::ofstream outFile(OUTPUT_FILE);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open " << OUTPUT_FILE << "\n";
        return 1;
    }

    std::string line;
    while (std::getline(inFile, line)) {
        if (line.empty()) continue;

        int count = std::stoi(line);
        int band = classifyCount(count);
        outFile << band << "\n";
    }

    inFile.close();
    outFile.close();
    std::cout << "Classification complete. Results written to " << OUTPUT_FILE << "\n";
    std::cout << "Thresholds: band 1 (<= " << THRESHOLD_1 << "), band 2 (<= " << THRESHOLD_2 << "), band 3 (> " << THRESHOLD_2 << ")\n";
    return 0;
}
