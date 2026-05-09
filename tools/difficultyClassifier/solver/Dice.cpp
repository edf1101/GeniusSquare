/*
 * Created by Ed Fillingham on 27/10/2024.
*/

#include "Dice.h"

#include <utility>
#include <cstdlib>
#include <algorithm>

Dice::Dice(vector<string> diceOptions) {
  this->diceOptions = std::move(diceOptions);
}

string Dice::roll() {
  int randomIndex = rand() % this->diceOptions.size();
  return this->diceOptions[randomIndex];
}

string Dice::toString() {
  string result = "A dice with options: ";
  for (string option: this->diceOptions) {
    result += option + ", ";
  }
  return result;
}

Coord Dice::optionToCoords(string option) {
  transform(option.begin(), option.end(), option.begin(), ::toupper);

  int row = option[0] - 'A';
  int col = option[1] - '1';

  return Coord{col, row};
}
