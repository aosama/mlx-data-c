#ifndef MLXD_MAPS_PRIVATE_H
#define MLXD_MAPS_PRIVATE_H

#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>

// Parallel C arrays -> C++ map parameters. Empty map: {NULL, NULL, 0}.
// Callers passing only one of the two arrays get a loud failure instead of
// a read through a NULL values pointer.
inline std::unordered_map<std::string, double> mlxd_pad_values_(
    const char** keys,
    const double* values,
    size_t n) {
  std::unordered_map<std::string, double> map;
  if (keys != nullptr || values != nullptr) {
    if (keys == nullptr || values == nullptr) {
      throw std::invalid_argument(
          "pad_values: keys and values must both be NULL or both be set");
    }
    for (size_t i = 0; i < n; i++) {
      map[keys[i]] = values[i];
    }
  }
  return map;
}

inline std::unordered_map<std::string, int> mlxd_batch_dims_(
    const char** keys,
    const int* values,
    size_t n) {
  std::unordered_map<std::string, int> map;
  if (keys != nullptr || values != nullptr) {
    if (keys == nullptr || values == nullptr) {
      throw std::invalid_argument(
          "batch_dims: keys and values must both be NULL or both be set");
    }
    for (size_t i = 0; i < n; i++) {
      map[keys[i]] = values[i];
    }
  }
  return map;
}

#endif
