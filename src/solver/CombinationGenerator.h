/*
 * Created by Ed Fillingham on 30/10/2024.
 *
 * This class is used to generate combinations of dice rolls.
 * It can also be used to check the validity of a combination.
*/

#ifndef GENIUS_SQUARE_ARDUINO_COMBINATIONGENERATOR_H
#define GENIUS_SQUARE_ARDUINO_COMBINATIONGENERATOR_H

#include <vector>
#include <string>
#include "../utils/maths.h"


using namespace std;

class CombinationGenerator {
public:
    static vector<Coord> generateCombinations(int seed);
    static bool validCombination(vector<Coord> blockers);

private:
    static vector<vector<string>> diceFaces;

    static int hash(int a);
    static vector<int> toMixedRadix(int value);
};


#endif //GENIUS_SQUARE_ARDUINO_COMBINATIONGENERATOR_H
