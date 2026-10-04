#ifndef MLXD_ARRAY_PRIVATE_H
#define MLXD_ARRAY_PRIVATE_H

#include <memory>

#include "mlx/data/Array.h"
#include "mlx/data/c/array.h"

inline mlxd_array mlxd_array_new_() {
  return mlxd_array({nullptr});
}

inline mlxd_array mlxd_array_new_(const std::shared_ptr<mlx::data::Array>& s) {
  return mlxd_array({new std::shared_ptr<mlx::data::Array>(s)});
}

inline mlxd_array mlxd_array_new_(std::shared_ptr<mlx::data::Array>&& s) {
  return mlxd_array({new std::shared_ptr<mlx::data::Array>(std::move(s))});
}

inline mlxd_array& mlxd_array_set_(
    mlxd_array& d,
    const std::shared_ptr<mlx::data::Array>& s) {
  if (d.ctx) {
    *static_cast<std::shared_ptr<mlx::data::Array>*>(d.ctx) = s;
  } else {
    d.ctx = new std::shared_ptr<mlx::data::Array>(s);
  }
  return d;
}

inline std::shared_ptr<mlx::data::Array>& mlxd_array_get_(mlxd_array d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_array");
  }
  return *static_cast<std::shared_ptr<mlx::data::Array>*>(d.ctx);
}

inline void mlxd_array_free_(mlxd_array d) {
  if (d.ctx) {
    delete static_cast<std::shared_ptr<mlx::data::Array>*>(d.ctx);
  }
}

#endif
