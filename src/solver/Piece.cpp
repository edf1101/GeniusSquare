/*
 * Created by Ed Fillingham on 27/10/2024.
*/

#include "Piece.h"

#include <utility>

#include "utils/math/maths.h"


set<int> Piece::uuids; // initialise the static variable here as can't in header file.

Piece::Piece(string name, string ansiColour, Colour ledColour, vector<vector<bool>> defaultMask) {
  /**
   * This is the constructor for the Piece class.
   */

  this->name = std::move(name);
  this->ansiColour = std::move(ansiColour);
  this->ledColour = ledColour;
  this->defaultMask = std::move(defaultMask);
  this->uuid = generateUuid();

  createMasks();

}

void Piece::createMasks() {
/**
 * This function creates all possible masks for the piece.
 */

  for (bool flip: {true, false}) {

    for (int rot = 0; rot < 4; rot++) {

      vector<vector<bool>> mask = defaultMask;
      mask = flip ? Maths::fliplr(mask) : mask;
      mask = Maths::rot90(mask, rot);

      // if the mask is not already in the set of masks, add it.
      if (!containsMask(mask)) {
        masks.push_back(mask);
      }
    }
  }
}

bool Piece::containsMask(vector<vector<bool>> mask) {
  /**
   * This function checks if the piece contains a mask.
   */

  bool found = false;

  for (auto &i: masks) {
    if (i == mask) {
      found = true;
      break;
    }
  }

  return found;
}

int Piece::generateUuid() {
  /**
   * This function generates a unique id for the piece.
   */

  for (int i = 1; i < 128; i++) {
    if (uuids.find(i) == uuids.end()) { // If the uuid is not in the set of uuids.
      uuids.insert(i);
      return i;
    }
  }

  return 0; // Could not generate a unique id. This should never happen.
}

int Piece::getUuid() {
  /**
   * This function returns the uuid of the piece.
   */
  return uuid;
}

string Piece::getName() {
  /**
   * This function returns the name of the piece.
   */
  return name;
}

string Piece::getAnsiColour() {
  /**
   * This function returns the ansi colour of the piece.
   */
  return ansiColour;
}

Colour Piece::getLedColour() {
  /**
   * This function returns the led colour of the piece.
   */
  return ledColour;
}

vector<vector<vector<bool>>> Piece::getMasks() {
  /**
   * This function returns the masks of the piece.
   */

  return masks;
}

void Piece::createPieces() {
  /**
   * This function creates all the pieces.
   */

  if (!pieces.empty())
    return;

  Piece Blocker = Piece("Blocker", "\u001b[38;5;94m", {5, 5, 5},
                        {{true}});
  Piece Blue = Piece("Blue", "\u001b[38;5;21m", {0, 0, 255},
                     {{true}});
  Piece Brown = Piece("Brown", "\u001b[38;5;52m", {128, 77, 77},
                      {{true, true}});
  Piece Orange = Piece("Orange", "\u001b[38;5;208m", {255, 102, 0},
                       {{true, true, true}});
  Piece Grey = Piece("Grey", "\u001b[38;5;248m", {128, 128, 128},
                     {{true, true, true, true}});
  Piece Red = Piece("Red", "\u001b[38;5;1m", {255, 0, 0},
                    {{false, true, true},
                     {true,  true, false}});
  Piece Yellow = Piece("Yellow", "\u001b[38;5;11m", {255, 179, 0},
                       {{true,  true, true},
                        {false, true, false}});
  Piece Cyan = Piece("Cyan", "\u001b[38;5;45m", {51, 128, 255},
                     {{true, true,  true},
                      {true, false, false}});
  Piece Green = Piece("Green", "\u001b[38;5;22m", {0, 255, 0},
                      {{true, true},
                       {true, true}});
  Piece Purple = Piece("Purple", "\u001b[38;5;54m", {128, 0, 128},
                       {{true, true},
                        {true, false}});

  // The blocker MUST BE THE FIRST piece in the list as it is referenced elsewhere as pieces[0].
  pieces = {Blocker,Grey,Red,Yellow,Cyan,Orange,Green,Purple,Brown,Blue};

}

vector<Piece> Piece::pieces = {};

vector<Piece> Piece::getPieces() {
  /**
   * This function gets all the pieces.
   */

  if (pieces.empty())
    createPieces();

  return pieces;
}

Colour Piece::getPieceColourByUUID(int uuid) {
  /**
   * This function gets the colour of the piece by the uuid.
   *
   * @param uuid The uuid of the piece.
   * @returns The colour of the piece.
   */

    if (pieces.empty())
        createPieces();

    Colour col = {0,0,0}; // default value in case we can't find the piece by its UUID.

    for (Piece piece : pieces) {
        if (piece.getUuid() == uuid) {
            col = piece.getLedColour();
            break;
        }
    }
    return col;
}
