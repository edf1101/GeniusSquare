/*
 * Created by Ed Fillingham on 27/10/2024.
 *
 * This is a header file for the Piece class.
 * The Piece class is used to represent a piece in the Genius Square game.
*/

#ifndef GENIUS_SQUARE_ARDUINO_PIECE_H
#define GENIUS_SQUARE_ARDUINO_PIECE_H

#include <set>
#include <string>
#include <vector>

#include "utils/math/maths.h"

using namespace std;

class Piece {
public:
    int getUuid();

    string getName();

    string getAnsiColour();

    Colour getLedColour();

    vector<vector<vector<bool>>> getMasks();

    static vector<Piece> getPieces();

    static Colour getPieceColourByUUID(int uuid);

private:
    Piece(string name, string ansiColour, Colour ledColour, vector<vector<bool>> defaultMask);

    static set<int> uuids;
    static int generateUuid();

    static void createPieces();
    static vector<Piece> pieces;

    int uuid;
    string name;
    string ansiColour;
    Colour ledColour;
    vector<vector<bool>> defaultMask;

    vector<vector<vector<bool>>> masks;

    void createMasks();

    bool containsMask(vector<vector<bool>> mask);

};


#endif //GENIUS_SQUARE_ARDUINO_PIECE_H
