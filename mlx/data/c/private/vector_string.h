#ifndef MLXD_VECTOR_STRING_PRIVATE_H
#define MLXD_VECTOR_STRING_PRIVATE_H

#include <string>
#include <vector>

#include "mlx/data/c/vector_string.h"

inline mlxd_vector_string mlxd_vector_string_new_() {
  return mlxd_vector_string({nullptr});
}

inline mlxd_vector_string mlxd_vector_string_new_(
    const std::vector<std::string>& s) {
  return mlxd_vector_string({new std::vector<std::string>(s)});
}

inline mlxd_vector_string mlxd_vector_string_new_(
    std::vector<std::string>&& s) {
  return mlxd_vector_string({new std::vector<std::string>(std::move(s))});
}

inline mlxd_vector_string& mlxd_vector_string_set_(
    mlxd_vector_string& d,
    const std::vector<std::string>& s) {
  if (d.ctx) {
    *static_cast<std::vector<std::string>*>(d.ctx) = s;
  } else {
    d.ctx = new std::vector<std::string>(s);
  }
  return d;
}

inline mlxd_vector_string& mlxd_vector_string_set_(
    mlxd_vector_string& d,
    std::vector<std::string>&& s) {
  if (d.ctx) {
    *static_cast<std::vector<std::string>*>(d.ctx) = std::move(s);
  } else {
    d.ctx = new std::vector<std::string>(std::move(s));
  }
  return d;
}

inline std::vector<std::string>& mlxd_vector_string_get_(mlxd_vector_string d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_vector_string");
  }
  return *static_cast<std::vector<std::string>*>(d.ctx);
}

inline void mlxd_vector_string_free_(mlxd_vector_string d) {
  if (d.ctx) {
    delete static_cast<std::vector<std::string>*>(d.ctx);
  }
}

#endif
