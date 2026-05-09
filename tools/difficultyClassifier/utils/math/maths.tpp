/*
 * Created by Ed Fillingham on 27/10/2024.
*/

#include "maths.h"
#include <algorithm>

template<typename T>
std::vector<std::vector<T>> Maths::fliplr(const std::vector<std::vector<T>> &matrix) {
  std::vector<std::vector<T>> flipped = matrix;

  for (auto &row: flipped) {
    std::reverse(row.begin(), row.end());
  }

  return flipped;
}


template<typename T>
std::vector<std::vector<T>> Maths::rot90(const std::vector<std::vector<T>> &matrix, int times) {
  int rowCount = matrix.size();
  if (rowCount == 0) return {};
  int colCount = matrix[0].size();

  std::vector<std::vector<T>> rotated = matrix;

  for (int t = 0; t < (times % 4); ++t) {
    std::vector<std::vector<T>> temp(colCount, std::vector<T>(rowCount));

    for (int i = 0; i < rowCount; ++i) {
      for (int j = 0; j < colCount; ++j) {
        temp[colCount - 1 - j][i] = rotated[i][j];
      }
    }

    rotated = temp;
    std::swap(rowCount, colCount);
  }

  return rotated;
}

template<typename T>
std::string Maths::MatAsString(const std::vector<std::vector<T>> &matrix) {

  int rows = matrix.size();
  int cols = matrix[0].size();

  std::string result = std::to_string(rows) + "x" + std::to_string(cols) + " Matrix:\n";
  for (size_t i = 0; i < matrix.size(); i++) {
    for (size_t j = 0; j < matrix[i].size(); j++) {
      result += std::to_string(matrix[i][j]) + " ";
    }
    if (i != matrix.size() - 1)
      result += "\n";
  }
  return result;
}
