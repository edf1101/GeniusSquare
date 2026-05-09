/*
 * Created by Ed Fillingham on 2026-05-09.
 *
 * Generates packed difficulty data from classifications.txt.
 * Packs 4 classifications per byte (2 bits each).
 * Output format: C hex array suitable for pasting into a header file.
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

int main() {
    std::ifstream inFile("classifications.txt");
    if (!inFile.is_open()) {
        std::cerr << "Failed to open classifications.txt\n";
        return 1;
    }

    std::vector<uint8_t> packed;
    std::vector<int> values;

    // Read all values
    int val;
    while (inFile >> val) {
        values.push_back(val);
    }
    inFile.close();

    if (values.size() != 100) {
        std::cerr << "Expected 100 values, got " << values.size() << "\n";
        return 1;
    }

    // Pack 4 values per byte (2 bits each)
    for (size_t i = 0; i < values.size(); i += 4) {
        uint8_t byte = 0;

        for (int j = 0; j < 4 && i + j < values.size(); j++) {
            uint8_t bits = (uint8_t)values[i + j];
            byte |= (bits << (j * 2));
        }

        packed.push_back(byte);
    }

    // Output as C hex array
    std::cout << "// Packed difficulty data for 100 seeds\n";
    std::cout << "// 2 bits per seed: 0=invalid, 1=hard, 2=medium, 3=easy\n";
    std::cout << "static const uint8_t DIFFICULTY_DATA[25] = {\n";

    for (size_t i = 0; i < packed.size(); i++) {
        std::cout << "    0x" << std::hex << std::setw(2) << std::setfill('0')
                  << (int)packed[i];
        if (i < packed.size() - 1) std::cout << ",";
        std::cout << "  // seeds " << std::dec << (i * 4) << "-" << (i * 4 + 3) << "\n";
    }

    std::cout << "};\n";

    return 0;
}
