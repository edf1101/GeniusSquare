/*
 * Created by Ed Fillingham on 27/10/2024.
*/

#include "Board.h"

#include <utility>
#include <cstring>

Board::Board(vector<Coord> blockers, int solution_limit, int time_limit) {
  this->blockers = std::move(blockers);
  this->solutionLimit = solution_limit;
  this->timeLimit = time_limit;

  for (auto &i: this->space) {
    for (int &j: i) {
      j = 0;
    }
  }

  pieces = createPieces();
  for (auto &blocker: this->blockers) {
    placePiece(blocker, 0, pieces[0]);
  }
}

Board::Board(const Board& other)
    : startSolveTime(other.startSolveTime),
      _cancel(other._cancel.load()),
      blockers(other.blockers),
      pieces(other.pieces),
      solutionLimit(other.solutionLimit),
      timeLimit(other.timeLimit),
      solutions(other.solutions)
{
    memcpy(space, other.space, sizeof(space));
}

void Board::cancel() {
    _cancel = true;
}

string Board::toString() {
  string resetCode = "\033[0m";
  string boardString = "";

  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j) {
      boardString += resetCode;
      int uuid = space[i][j];

      if (uuid == 0) {
        boardString += string("[0;37m") + " X ";
        continue;
      }

      Piece *piece = getPiece(uuid);
      string textColour = piece->getAnsiColour();
      bool isBlocker = piece->getName() == "Blocker";
      boardString += textColour + (isBlocker ? " ● " : " ■ ");
    }
    boardString += "\n";
  }

  boardString += resetCode;
  return boardString;
}

vector<Piece> Board::createPieces() {
    return Piece::getPieces();
}

void Board::placePiece(Coord coord, int orientation, Piece piece) {
  int row = coord.y;
  int col = coord.x;

  std::vector<std::vector<bool>> pieceMask = piece.getMasks()[orientation];
  int pieceRows = (int) pieceMask.size();
  int pieceCols = (int) pieceMask[0].size();

  for (int i = 0; i < pieceRows; ++i) {
    for (int j = 0; j < pieceCols; ++j) {
      if (row + i < 6 && col + j < 6) {
        int add_value = pieceMask[i][j] * piece.getUuid();
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
  auto elapsed = chrono::steady_clock::now() - startSolveTime;
  return chrono::duration_cast<chrono::seconds>(elapsed).count() >= timeLimit;
}

bool Board::isCancelled() const {
  return _cancel.load();
}

int Board::getSolutionLimit() const {
  return solutionLimit;
}

void Board::addSolution(vector<vector<int>> solution) {
  solutions.push_back(solution);
}

vector<vector<int>> Board::getSpace() const {
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
  return solutions;
}

int Board::pieceFitsAtSpace(Coord coord, Piece piece) {
  int row = coord.y;
  int col = coord.x;

  const auto &masks = piece.getMasks();

  for (size_t maskIndex = 0; maskIndex < masks.size(); ++maskIndex) {
    const auto &pieceMask = masks[maskIndex];
    int pieceRows = pieceMask.size();
    int pieceCols = pieceMask[0].size();

    if (row + pieceRows > 6 || col + pieceCols > 6) {
      continue;
    }

    bool fits = true;
    for (int i = 0; i < pieceRows && fits; ++i) {
      for (int j = 0; j < pieceCols; ++j) {
        if (pieceMask[i][j] && space[row + i][col + j] != 0) {
          fits = false;
          break;
        }
      }
    }

    if (fits) {
      return static_cast<int>(maskIndex);
    }
  }

  return -1;
}

bool Board::isSolved() {
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
  Piece piece = remainingPieces[0];

  if (rootBoard->getOutOfTime() || rootBoard->isCancelled()) {
    return false;
  }

  for (int row = 0; row < 6; row++) {
    for (int col = 0; col < 6; col++) {

      if (rootBoard->getOutOfTime() || rootBoard->isCancelled()) {
        return false;
      }

      int orientation = this->pieceFitsAtSpace({col, row}, piece);

      if (orientation != -1) {
        Board newBoard = Board(*this);
        newBoard.placePiece({col, row}, orientation, piece);

        vector<Piece> newRemainingPieces = vector<Piece>(remainingPieces.begin() + 1, remainingPieces.end());

        if (newBoard.isSolved()) {
          rootBoard->addSolution(newBoard.getSpace());
          return (int)rootBoard->getSolutions().size() >= rootBoard->getSolutionLimit();
        }

        bool hitLimit = newBoard.recursiveSolve(rootBoard, newRemainingPieces);
        if (hitLimit) {
          return true;
        }
      }
    }
  }

  return false;
}

bool Board::solve() {
  startSolveTime = chrono::steady_clock::now();

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
