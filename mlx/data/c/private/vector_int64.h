#ifndef MLXD_VECTOR_INT64_PRIVATE_H
#define MLXD_VECTOR_INT64_PRIVATE_H

#include <cstdint>
#include <vector>

#include "mlx/data/c/vector_int64.h"

inline mlxd_vector_int64 mlxd_vector_int64_new_() {
  return mlxd_vector_int64({nullptr});
}

inline mlxd_vector_int64 mlxd_vector_int64_new_(
    const std::vector<int64_t>& s) {
  return mlxd_vector_int64({new std::vector<int64_t>(s)});
}

inline mlxd_vector_int64 mlxd_vector_int64_new_(std::vector<int64_t>&& s) {
  return mlxd_vector_int64({new std::vector<int64_t>(std::move(s))});
}

inline mlxd_vector_int64& mlxd_vector_int64_set_(
    mlxd_vector_int64& d,
    const std::vector<int64_t>& s) {
  if (d.ctx) {
    *static_cast<std::vector<int64_t>*>(d.ctx) = s;
  } else {
    d.ctx = new std::vector<int64_t>(s);
  }
  return d;
}

inline mlxd_vector_int64& mlxd_vector_int64_set_(
    mlxd_vector_int64& d,
    std::vector<int64_t>&& s) {
  if (d.ctx) {
    *static_cast<std::vector<int64_t>*>(d.ctx) = std::move(s);
  } else {
    d.ctx = new std::vector<int64_t>(std::move(s));
  }
  return d;
}

inline std::vector<int64_t>& mlxd_vector_int64_get_(mlxd_vector_int64 d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_vector_int64");
  }
  return *static_cast<std::vector<int64_t>*>(d.ctx);
}

inline void mlxd_vector_int64_free_(mlxd_vector_int64 d) {
  if (d.ctx) {
    delete static_cast<std::vector<int64_t>*>(d.ctx);
  }
}

#endif
