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
        {"C5", "F6", "A4", "D6", "C6", "B5"}}; // all the dice faces.

/**
 * @brief an LCG-based hash function to generate a pseudo-random index for the combinations.
 * @param seed The seed to generate the combinations.
 * @return The new pseudo-random value
 */
int CombinationGenerator::hash(int seed) {
  const int n = 62208;

  // Much larger multiplier that still satisfies Hull-Dobell
  const int a = 38449;
  const int c = 17;

  int value = (a * seed + c) % n;
  if (value < 0) value += n;
  return value;
}

vector<int> CombinationGenerator::toMixedRadix(int value) {
  /**
   * Decode value into per-die indices with radices:
   * [6, 6, 2, 6, 4, 6, 6]
   */
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
  /**
   * Function to generate all possible combinations of dice faces.
   *
   * @param seed: The seed to generate the combinations.
   * @return: A vector of vectors of integers representing the dice faces.
   */

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
  /**
   * Function to check if a combination is valid.
   *
   * @param blockers: The combination to check.
   * @return: True if the combination is valid, false otherwise.
   */

  if (blockers.size() != 7) {
    return false;
  }

  int diceUsed[7] = {0, 0, 0, 0, 0, 0, 0};

  for (int i = 0; i < blockers.size(); i++) {
    Coord coord = blockers[i];

    // find which dice was used for this blocker

    int dice = 0;
    for (i = 0; i < 7; i++) {
      for (int face = 0; face < 6; face++) {
        string diceOption = diceFaces[i][0];
        Coord diceCoord = Dice::optionToCoords(diceOption);
        if (diceCoord.x == coord.x && diceCoord.y == coord.y) {
          dice = i;
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


