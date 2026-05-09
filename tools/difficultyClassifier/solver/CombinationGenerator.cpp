/*
 * Created by Ed Fillingham on 30/10/2024.
*/

#include "CombinationGenerator.h"
#include <cmath>
#include "Dice.h"

vector<vector<std::string>>  CombinationGenerator::diceFaces = {
        {"E6", "F5", "E4", "F4", "D5", "E5"},
        {"F3", "D2", "E2", "C1", "A1", "D1"},
        {"F1", "A6"},
        {"B1", "C2", "A2", "B3", "A3", "B2"},
        {"A5", "E1", "B6", "F2",},
        {"C4", "D3", "B4", "C3", "D4", "E3"},
        {"C5", "F6", "A4", "D6", "C6", "B5"}};

int CombinationGenerator::hash(int seed) {
  const int n = 62208;
  const int a = 38449;
  const int c = 17;

  int value = (a * seed + c) % n;
  if (value < 0) value += n;
  return value;
}

vector<int> CombinationGenerator::toMixedRadix(int value) {
  static const int radices[7] = {6, 6, 2, 6, 4, 6, 6};

  vector<int> digits;
  digits.reserve(7);

  for (int i = 0; i < 7; i++) {
    digits.push_back(value % radices[i]);
    value /= radices[i];
  }

  return digits;
}

vector<Coord> CombinationGenerator::generateCombinations(int seed) {
  vector<Coord> combinations;
  int index = hash(seed);
  vector<int> digits = toMixedRadix(index);

  for (int i = 0; i < 7; i++) {
    string face = diceFaces[i][digits[i]];
    Coord coord = Dice::optionToCoords(face);
    combinations.push_back(coord);
  }

  return combinations;
}

bool CombinationGenerator::validCombination(vector<Coord> blockers) {
  if (blockers.size() != 7) {
    return false;
  }

  int diceUsed[7] = {0, 0, 0, 0, 0, 0, 0};

  for (size_t i = 0; i < blockers.size(); i++) {
    Coord coord = blockers[i];

    int dice = 0;
    for (int j = 0; j < 7; j++) {
      for (int face = 0; face < (int)diceFaces[j].size(); face++) {
        Coord diceCoord = Dice::optionToCoords(diceFaces[j][face]);
        if (diceCoord.x == coord.x && diceCoord.y == coord.y) {
          dice = j;
          break;
        }
      }
    }

    if (diceUsed[dice] == 1) {
      return false;
    }
    diceUsed[dice] = 1;
  }
  return true;
}
