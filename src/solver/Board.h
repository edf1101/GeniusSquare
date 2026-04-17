/*
 * Created by Ed Fillingham on 27/10/2024.
 *
 * This class represents the board of the game.
*/

#ifndef GENIUS_SQUARE_ARDUINO_BOARD_H
#define GENIUS_SQUARE_ARDUINO_BOARD_H

#include <vector>
#include <string>

#include "utils/math/maths.h"

#include "Piece.h"

using namespace std;

class Board {
public:
    Board(vector<Coord> blockers, int solutionLimit = 3, int timeLimit = 20);

    string toString(); // represent the board as a string.

    vector<vector<vector<int>>> getSolutions();

    bool isSolved();

    bool solve();


private:
    unsigned long startSolveTime = 0;
    vector<Coord> blockers;
    vector<Piece> pieces;
    int solutionLimit;
    int timeLimit;
    vector<vector<vector<int>>> solutions;
    int space[6][6]{};

    static vector<Piece> createPieces();

    Piece *getPiece(int uuid);

    int pieceFitsAtSpace(Coord coord, Piece piece);

    vector<vector<int>> getSpace();

    bool recursiveSolve(Board *rootBoard, vector<Piece> remainingPieces);

    void addSolution(vector<vector<int>> solution);

    void placePiece(Coord coord, int orientation, Piece piece);

    bool getOutOfTime() const;

    int getSolutionLimit() const;

};


#endif //GENIUS_SQUARE_ARDUINO_BOARD_H
