#ifndef MLXD_SAMPLE_PRIVATE_H
#define MLXD_SAMPLE_PRIVATE_H

#include "mlx/data/Sample.h"
#include "mlx/data/c/sample.h"

inline mlxd_sample mlxd_sample_new_() {
  return mlxd_sample({nullptr});
}

inline mlxd_sample mlxd_sample_new_(const mlx::data::Sample& s) {
  return mlxd_sample({new mlx::data::Sample(s)});
}

inline mlxd_sample mlxd_sample_new_(mlx::data::Sample&& s) {
  return mlxd_sample({new mlx::data::Sample(std::move(s))});
}

inline mlxd_sample& mlxd_sample_set_(mlxd_sample& d, const mlx::data::Sample& s) {
  if (d.ctx) {
    *static_cast<mlx::data::Sample*>(d.ctx) = s;
  } else {
    d.ctx = new mlx::data::Sample(s);
  }
  return d;
}

inline mlxd_sample& mlxd_sample_set_(mlxd_sample& d, mlx::data::Sample&& s) {
  if (d.ctx) {
    *static_cast<mlx::data::Sample*>(d.ctx) = std::move(s);
  } else {
    d.ctx = new mlx::data::Sample(std::move(s));
  }
  return d;
}

inline mlx::data::Sample& mlxd_sample_get_(mlxd_sample d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_sample");
  }
  return *static_cast<mlx::data::Sample*>(d.ctx);
}

inline void mlxd_sample_free_(mlxd_sample d) {
  if (d.ctx) {
    delete static_cast<mlx::data::Sample*>(d.ctx);
  }
}

#endif
