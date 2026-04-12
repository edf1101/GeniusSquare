/*
 * Created by Ed Fillingham on 26/07/2025.
 *
 * This header holds useful functions to hash arbitrary numbers and types of values.
 * Mainly used for updating widgets
*/

#ifndef HASHHELPERS_HASHHELPERS_H
#define HASHHELPERS_HASHHELPERS_H

#include <functional>
#include <cstddef>

/**
 * Combine a value into the running hash
 *
 * @tparam T the type of the value to add
 * @param seed The current hash value, which will be updated with the new value
 * @param v the new value to combine into the hash
 */
template <typename T>
inline void hash_combine(std::size_t& seed, T const& v) {
  std::hash<T> hasher;
  // magic constant from Boost
  seed ^= hasher(v) + 0x9e3779b97f4a7c15ULL + (seed<<6) + (seed>>2);
}

/**
 * Combine multiple values into a single hash value.
 *
 * @tparam Ts Variadic template parameter for multiple types
 * @param args the values to combine into the hash
 * @return the hash value resulting from combining all the provided values
 */
template <typename... Ts>
inline std::size_t hash_values(Ts const&... args) {
  std::size_t seed = 0;
  (hash_combine(seed, args), ...);  // C++17 fold‑expression
  return seed;
}


#endif //HASHHELPERS_HASHHELPERS_H
