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

struct Coord; // forward declaration for Maths::coordsToString

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

    // Template function to convert a vector of Coord to a bracketed comma-separated string, e.g. "[(0, 1), (2, 3)]"
    static std::string coordsToString(const std::vector<Coord>& coords);
};

#include "maths.tpp"

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

inline std::string Maths::coordsToString(const std::vector<Coord>& coords) {
    std::string s = "[";
    for (size_t i = 0; i < coords.size(); i++) {
        s += coords[i].toString();
        if (i + 1 < coords.size()) s += ", ";
    }
    s += "]";
    return s;
}

#endif //GENIUS_SQUARE_ARDUINO_MATHS_H
