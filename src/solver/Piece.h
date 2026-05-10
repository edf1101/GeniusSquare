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

#include "../utils/maths.h"

using namespace std;

class Piece {
public:
    // Getters for the properties of the piece.
    int getUuid();

    string getName();

    string getAnsiColour();

    Colour getLedColour();

    vector<vector<vector<bool>>> getMasks();

    static vector<Piece> getPieces(); // creates all the pieces.

    static Colour getPieceColourByUUID(int uuid); // gets the colour of the piece by the uuid.

private:
    Piece(string name, string ansiColour, Colour ledColour, vector<vector<bool>> defaultMask);

    static set<int> uuids; // holds all the uuids of the pieces so that they are unique.
    static int generateUuid(); // generates a unique id for the piece.

    static void createPieces(); // creates all the pieces.
    static vector<Piece> pieces; // holds all the pieces.

    // These are the properties of the piece object.
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
