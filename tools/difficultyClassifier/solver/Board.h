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
#include <chrono>

#include "utils/math/maths.h"

#include "Piece.h"

using namespace std;

class Board {
public:
    Board(vector<Coord> blockers, int solutionLimit = INT_MAX, int timeLimit = 30);
    Board(const Board& other);

    string toString();

    vector<vector<vector<int>>> getSolutions();

    bool isSolved();

    bool solve();

    void cancel();

    vector<vector<int>> getSpace() const;

    ~Board() = default;

private:
    chrono::steady_clock::time_point startSolveTime;
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
};


#endif //GENIUS_SQUARE_ARDUINO_BOARD_H
