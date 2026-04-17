/*
 * Created by Ed Fillingham on 27/10/2024.
*/

#include "Dice.h"

#include <utility>

Dice::Dice(vector<string> diceOptions) {
  /**
   * Constructor for the Dice class
   * @param diceOptions: A vector of strings representing the dice options
   */

  this->diceOptions = std::move(diceOptions);

}

string Dice::roll() {
  /**
   * Returns a random dice option
   * @return: A random dice option
   */

  int randomIndex = random(0, this->diceOptions.size());
  return this->diceOptions[randomIndex];
}

string Dice::toString() {
  /**
   * Returns a string representation of the dice options
   * @return: A string representation of the dice options
   */

  string result = "A dice with options: ";
  for (string option: this->diceOptions) {
    result += option + ", ";
  }

  return result;
}

Coord Dice::optionToCoords(string option) {
  /**
   * Returns the coordinates of the dice option.
   * Genius square options may be "A1", "B2", etc,
   * This would convert them to coordinates. eg. "A1" -> (0, 0), "B2" -> (1, 1)
   *
   * @param option: The dice option
   * @return: The coordinates of the dice option
   */

  // make sure the option is in uppercase.
  transform(option.begin(), option.end(), option.begin(), ::toupper);

  int row = option[0] - 'A';
  int col = option[1] - '1';

  return Coord{col, row};
}
