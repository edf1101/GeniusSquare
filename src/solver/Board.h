/*
 * Created by Ed Fillingham on 27/10/2024.
 *
 * This class represents the board of the game.
*/

#ifndef GENIUS_SQUARE_ARDUINO_BOARD_H
#define GENIUS_SQUARE_ARDUINO_BOARD_H

#include <atomic>
#include <vector>
#include <string>

#include "utils/math/maths.h"

#include "Piece.h"

using namespace std;

class Board {
public:
    Board(vector<Coord> blockers, int solutionLimit = 3, int timeLimit = 20);
    Board(const Board& other);

    string toString(); // represent the board as a string.

    vector<vector<vector<int>>> getSolutions();

    bool isSolved();

    bool solve();

    /** @brief Signal the solver to abort at the next recursion check. Safe to call from another core. */
    void cancel();

    /** @brief Print the current board state (solution) to the Serial monitor. */
    void printSolution();

    /** @brief Returns the current board grid (UUID per cell, 0 = empty). */
    vector<vector<int>> getSpace() const;

    ~Board();

private:
    unsigned long startSolveTime = 0;
    std::atomic<bool> _cancel{false};
    vector<Coord> blockers;
    vector<Piece> pieces;
    int solutionLimit;
    int timeLimit;
    vector<vector<vector<int>>> solutions;
    int space[6][6]{};

    static vector<Piece> createPieces();

    Piece *getPiece(int uuid);

    int pieceFitsAtSpace(Coord coord, Piece piece);

    bool recursiveSolve(Board *rootBoard, vector<Piece> remainingPieces);

    void addSolution(vector<vector<int>> solution);

    void placePiece(Coord coord, int orientation, Piece piece);

    bool getOutOfTime() const;

    bool isCancelled() const;

    int getSolutionLimit() const;

    bool _isRoot = false;

};


#endif //GENIUS_SQUARE_ARDUINO_BOARD_H
