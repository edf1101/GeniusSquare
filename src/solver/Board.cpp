/*
 * Created by Ed Fillingham on 27/10/2024.
*/

#include "Board.h"

#include <utility>
#include "Arduino.h"

Board::Board(vector<Coord> blockers, int solution_limit, int time_limit) {
  /**
   * Constructor for the Board class.
   *
   * @param blockers: A vector of coordinates that represent the blockers on the board.
   * @param solution_limit: The max number of solutions to find.
   * @param time_limit: The time limit for the solver.
   */

  this->blockers = std::move(blockers);
  this->solutionLimit = solution_limit;
  this->timeLimit = time_limit;

  // set up space array as 6x6 grid of 0s
  for (auto &i: this->space) {
    for (int &j: i) {
      j = 0;
    }
  }

  pieces = createPieces(); // create the pieces.
  // place all the blockers on the board.
  for (auto &blocker: this->blockers) {
    placePiece(blocker, 0, pieces[0]);
  }

}

string Board::toString() {
  /**
   * Convert the board to a string.
   *
   * @return: The board as a string.
   */
  string resetCode = "\033[0m";

  string boardString = "";
  for (int i = 0; i < 6; ++i) {

    for (int j = 0; j < 6; ++j) {

      boardString += resetCode; // reset the colour for each space.

      int uuid = space[i][j]; // get the uuid of the piece in the space.

      if (uuid == 0) { // if the space is empty.
        boardString += string("\u001b[0;37m") + " X ";
        continue;
      }
      // space is not empty, find the piece with the uuid.
      Piece *piece = getPiece(uuid);

      string textColour = piece->getAnsiColour();
      bool isBlocker = piece->getName() == "Blocker";

      boardString += textColour + (isBlocker ? " ● " : " ■ ");
    }
    boardString += "\n";
  }

  boardString += resetCode; // reset the colour for end of string.
  return boardString;
}

vector<Piece> Board::createPieces() {
  /**
   * Create the pieces for the board.
   *
   * @return: A vector of pieces.
   */

    return Piece::getPieces();
}

void Board::placePiece(Coord coord, int orientation, Piece piece) {
  /**
   * Place a piece on the board.
   *
   * @param coord: The coordinate to place the piece.
   * @param orientation: The orientation of the piece.
   * @param piece: The piece to place.
   */

  int row = coord.y;
  int col = coord.x;

  std::vector<std::vector<bool>> pieceMask = piece.getMasks()[orientation];
  int pieceRows = (int) pieceMask.size();
  int pieceCols = (int) pieceMask[0].size();

  // Create a slice of the board where the piece will be placed
  for (int i = 0; i < pieceRows; ++i) {
    for (int j = 0; j < pieceCols; ++j) {
      // Check if we're within the bounds of the board
      if (row + i < 6 && col + j < 6) {
        // Calculate the value to add
        int add_value = pieceMask[i][j] * piece.getUuid();
        // Update the board slice
        space[row + i][col + j] += add_value;
      }
    }
  }
}

Piece *Board::getPiece(int uuid) {
  Piece *piece = nullptr;

  for (auto &p: pieces) {
    if (p.getUuid() == uuid) {
      piece = &p;
      break;
    }
  }
  return piece;
}

bool Board::getOutOfTime() const {
  /**
   * Check if the solver has run out of time.
   *
   * @return: True if the solver has run out of time.
   */

  return (millis() - startSolveTime) > timeLimit * 1000;
}

int Board::getSolutionLimit() const {
  /**
   * Get the solution limit.
   *
   * @return: The solution limit.
   */

  return solutionLimit;
}

void Board::addSolution(vector<vector<int>> solution) {
  /**
   * Add a solution to the list of solutions.
   *
   * @param solution: The solution to add.
   */

  solutions.push_back(solution);
}

vector<vector<int>> Board::getSpace() {
  /**
   * Get the space array. It will need to be converted to a 2D vector of ints.
   *
   * @return: The space 2D vector.
   */

  vector<vector<int>> spaceVector;
  for (int i = 0; i < 6; ++i) {
    vector<int> row;
    for (int j = 0; j < 6; ++j) {
      row.push_back(space[i][j]);
    }
    spaceVector.push_back(row);
  }

  return spaceVector;
}

vector< vector<vector<int>> > Board::getSolutions() {
  /**
   * Get the solutions.
   *
   * @return: The solutions.
   */

  return solutions;
}

int Board::pieceFitsAtSpace(Coord coord, Piece piece) {
  /**
   * Checks if a Piece can fit on the Board at the given row & column,
   * by iterating through each of the piece's orientation masks. The top-
   * left of the piece mask will be used as the origin.
   *
   * @param coord: The coordinate to check.
   * @param piece: The piece to check.
   * @return: index to the Piece's mask for the first orientation that will fit on the
   * Board. If none fit, return -1.
   */

  int row = coord.y;
  int col = coord.x;

// Iterate through each orientation of the piece
  const auto &masks = piece.getMasks();

  for (size_t maskIndex = 0; maskIndex < masks.size(); ++maskIndex) {

    const auto &pieceMask = masks[maskIndex];
    int pieceRows = pieceMask.size();
    int pieceCols = pieceMask[0].size();

    // Check if the piece extends outside the board
    if (row + pieceRows > 6 || col + pieceCols > 6) {
      continue;
    }

    // Check if the piece fits in the specified location
    bool fits = true;
    for (int i = 0; i < pieceRows && fits; ++i) {
      for (int j = 0; j < pieceCols; ++j) {
        // Bitwise check: piece only fits if board cell is empty (0) or matches piece mask
        if (pieceMask[i][j] && space[row + i][col + j] != 0) {
          fits = false;
          break;
        }
      }
    }

    // If we found a fit, return the orientation index
    if (fits) {
      return static_cast<int>(maskIndex);
    }
  }

  return -1; // No orientation fits, return -1
}

bool Board::isSolved() {
  /**
   * Check if the board is solved. (If there are no empty spaces left)
   *
   * @return: True if the board is solved.
   */

  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j) {
      if (space[i][j] == 0) {
        return false;
      }
    }
  }
  return true;
}

bool Board::recursiveSolve(Board* rootBoard, vector<Piece> remainingPieces) {
  /**
   * Recursively solve the board.
   *
   * @param rootBoard: The board to solve.
   * @param remainingPieces: The remaining pieces to place.
   * @return: True if the board is solved.
   */

  Piece piece = remainingPieces[0];

  if (rootBoard->getOutOfTime()) {
    return false;
  }

  for (int row = 0; row < 6; row++) {
    for (int col = 0; col < 6; col++) {

      if (rootBoard->getOutOfTime()) { // so it doesnt try each space after time limit is reached.
        return false;
      }

      int orientation = this->pieceFitsAtSpace({col, row}, piece);

      if (orientation != -1) { // if the piece fits
        // create a new board with the piece placed.
        Board newBoard = Board(*this);
        newBoard.placePiece({col, row}, orientation, piece);


        // new remaining pieces is all the remaining pieces except the first one.
        vector<Piece> newRemainingPieces = vector<Piece>(remainingPieces.begin() + 1, remainingPieces.end());

        if (newBoard.isSolved()) {
          rootBoard->addSolution( newBoard.getSpace());
          return rootBoard->getSolutions().size() >= rootBoard->getSolutionLimit();
        }

        // We still have pieces to place, so recursively call this function again.
        bool hitLimit = newBoard.recursiveSolve(rootBoard, newRemainingPieces);
        if (hitLimit) {
          return true;
        }
      }
    }
  }

  return false; // No possible solutions with this current recursive state.
}

bool Board::solve() {
  /**
   * Solve the board.
   */

  startSolveTime = millis();

  vector<Piece> remainingPieces = vector<Piece>(pieces.begin() + 1, pieces.end());

  recursiveSolve(this, remainingPieces);

  if (!solutions.empty()) {
    vector<vector<int>> solutionSpace = solutions[0];
    for (int i = 0; i < 6; ++i) {
      for (int j = 0; j < 6; ++j) {
        space[i][j] = solutionSpace[i][j];
      }
    }
  }

  return !solutions.empty();
}
