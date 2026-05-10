/*
 * Created by Ed Fillingham on 27/10/2024.
 *
 * This class represents rolling a Genius Square dice.
*/

#ifndef GENIUS_SQUARE_ARDUINO_DICE_H
#define GENIUS_SQUARE_ARDUINO_DICE_H

#include <vector>
#include <string>
#include "../utils/maths.h"


using namespace std;

class Dice {
public:
    Dice(vector<string> diceOptions);

    string roll(); // Returns a random dice option

    string toString(); // Returns a string representation of the dice options

    static Coord optionToCoords(string option); // Returns the coordinates of the dice option


private:
    vector<string> diceOptions;
};


#endif //GENIUS_SQUARE_ARDUINO_DICE_H
