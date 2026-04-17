/*
 * Created by Ed Fillingham on 27/10/2024.
 *
 * This is a header file for the maths class.
 * The maths class will do a few numpy esque things.
 * eg. fliplr, rot90, etc.
*/

#ifndef GENIUS_SQUARE_ARDUINO_MATHS_H
#define GENIUS_SQUARE_ARDUINO_MATHS_H


#include <vector>
#include <string>

class Maths {
public:
    // Template function to flip a 2D vector horizontally (like numpy's fliplr)
    template <typename T>
    static std::vector<std::vector<T>> fliplr(const std::vector<std::vector<T>>& matrix);

    // Template function to rotate a 2D vector by 90 degrees
    template <typename T>
    static std::vector<std::vector<T>> rot90(const std::vector<std::vector<T>>& matrix, int times);

    // Template function to print a 2D vector to the serial monitor
    template <typename T>
    static std::string MatAsString(const std::vector<std::vector<T>>& matrix);
};

#include "maths.tpp" // Ensure this is included

struct Coord {
    // The coordinates of a point
    int x;
    int y;

    // Function to convert the coordinates to a string
    std::string toString() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }

    bool operator==(const Coord& other) const {
      return (x == other.x && y == other.y);
    }

};

struct Colour {
    // The RGB values of a colour
    int r;
    int g;
    int b;

    // Function to convert the colour to a string
    std::string toString() const {
        return "(" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ")";
    }

};

#endif //GENIUS_SQUARE_ARDUINO_MATHS_H
