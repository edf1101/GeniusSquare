/*
 * Created by Ed Fillingham on 27/10/2024.
*/

#include "maths.h"
#include <algorithm>
#include "Arduino.h"

template<typename T>
std::vector<std::vector<T>> Maths::fliplr(const std::vector<std::vector<T>> &matrix) {
  // Create an output matrix with the same dimensions
  std::vector<std::vector<T>> flipped = matrix;

  // Flip each row in place
  for (auto &row: flipped) {
    std::reverse(row.begin(), row.end());
  }

  return flipped;
}


template<typename T>
std::vector<std::vector<T>> Maths::rot90(const std::vector<std::vector<T>> &matrix, int times) {
  int rowCount = matrix.size();
  if (rowCount == 0) return {}; // Handle empty matrix
  int colCount = matrix[0].size();

  std::vector<std::vector<T>> rotated = matrix;

  for (int t = 0; t < (times % 4); ++t) { // Only need to rotate 0-3 times
    std::vector<std::vector<T>> temp(colCount, std::vector<T>(rowCount)); // Create a new matrix with swapped dimensions

    for (int i = 0; i < rowCount; ++i) {
      for (int j = 0; j < colCount; ++j) {
        temp[colCount - 1 - j][i] = rotated[i][j];
      }
    }

    rotated = temp; // Update rotated matrix
    std::swap(rowCount, colCount); // Update the dimensions for the next rotation
  }

  return rotated;
}

template<typename T>
std::string Maths::MatAsString(const std::vector<std::vector<T>> &matrix) {

  int rows = matrix.size();
  int cols = matrix[0].size();

  std::string result = std::to_string(rows) + "x" + std::to_string(cols) + " Matrix:\n";
  for (int i = 0; i < matrix.size(); i++) {
    for (int j = 0; j < matrix[i].size(); j++) {
      result += std::to_string(matrix[i][j]) + " ";
    }
    if (i != matrix.size() - 1) // Don't add a newline after the last row
      result += "\n";
  }
  return result;
}
