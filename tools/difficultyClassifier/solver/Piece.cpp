/*
 * Created by Ed Fillingham on 27/10/2024.
*/

#include "Piece.h"

#include <utility>

#include "utils/math/maths.h"


set<int> Piece::uuids;

Piece::Piece(string name, string ansiColour, Colour ledColour, vector<vector<bool>> defaultMask) {
  this->name = std::move(name);
  this->ansiColour = std::move(ansiColour);
  this->ledColour = ledColour;
  this->defaultMask = std::move(defaultMask);
  this->uuid = generateUuid();

  createMasks();
}

void Piece::createMasks() {
  for (bool flip: {true, false}) {
    for (int rot = 0; rot < 4; rot++) {
      vector<vector<bool>> mask = defaultMask;
      mask = flip ? Maths::fliplr(mask) : mask;
      mask = Maths::rot90(mask, rot);

      if (!containsMask(mask)) {
        masks.push_back(mask);
      }
    }
  }
}

bool Piece::containsMask(vector<vector<bool>> mask) {
  for (auto &i: masks) {
    if (i == mask) {
      return true;
    }
  }
  return false;
}

int Piece::generateUuid() {
  for (int i = 1; i < 128; i++) {
    if (uuids.find(i) == uuids.end()) {
      uuids.insert(i);
      return i;
    }
  }
  return 0;
}

int Piece::getUuid() {
  return uuid;
}

string Piece::getName() {
  return name;
}

string Piece::getAnsiColour() {
  return ansiColour;
}

Colour Piece::getLedColour() {
  return ledColour;
}

vector<vector<vector<bool>>> Piece::getMasks() {
  return masks;
}

void Piece::createPieces() {
  if (!pieces.empty())
    return;

  Piece Blocker = Piece("Blocker", "[38;5;94m", {5, 5, 5},
                        {{true}});
  Piece Blue = Piece("Blue", "[38;5;21m", {0, 0, 255},
                     {{true}});
  Piece Brown = Piece("Brown", "[38;5;52m", {128, 77, 77},
                      {{true, true}});
  Piece Orange = Piece("Orange", "[38;5;208m", {255, 102, 0},
                       {{true, true, true}});
  Piece Grey = Piece("Grey", "[38;5;248m", {128, 128, 128},
                     {{true, true, true, true}});
  Piece Red = Piece("Red", "[38;5;1m", {255, 0, 0},
                    {{false, true, true},
                     {true,  true, false}});
  Piece Yellow = Piece("Yellow", "[38;5;11m", {255, 179, 0},
                       {{true,  true, true},
                        {false, true, false}});
  Piece Cyan = Piece("Cyan", "[38;5;45m", {51, 128, 255},
                     {{true, true,  true},
                      {true, false, false}});
  Piece Green = Piece("Green", "[38;5;22m", {0, 255, 0},
                      {{true, true},
                       {true, true}});
  Piece Purple = Piece("Purple", "[38;5;54m", {128, 0, 128},
                       {{true, true},
                        {true, false}});

  pieces = {Blocker,Grey,Red,Yellow,Cyan,Orange,Green,Purple,Brown,Blue};
}

vector<Piece> Piece::pieces = {};

vector<Piece> Piece::getPieces() {
  if (pieces.empty())
    createPieces();

  return pieces;
}

Colour Piece::getPieceColourByUUID(int uuid) {
    if (pieces.empty())
        createPieces();

    Colour col = {0,0,0};

    for (Piece piece : pieces) {
        if (piece.getUuid() == uuid) {
            col = piece.getLedColour();
            break;
        }
    }
    return col;
}
