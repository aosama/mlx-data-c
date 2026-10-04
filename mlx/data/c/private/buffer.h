#ifndef MLXD_BUFFER_PRIVATE_H
#define MLXD_BUFFER_PRIVATE_H

#include "mlx/data/Buffer.h"
#include "mlx/data/c/buffer.h"

inline mlxd_buffer mlxd_buffer_new_() {
  return mlxd_buffer({nullptr});
}

inline mlxd_buffer mlxd_buffer_new_(const mlx::data::Buffer& s) {
  return mlxd_buffer({new mlx::data::Buffer(s)});
}

inline mlxd_buffer mlxd_buffer_new_(mlx::data::Buffer&& s) {
  return mlxd_buffer({new mlx::data::Buffer(std::move(s))});
}

inline mlxd_buffer& mlxd_buffer_set_(mlxd_buffer& d, const mlx::data::Buffer& s) {
  if (d.ctx) {
    *static_cast<mlx::data::Buffer*>(d.ctx) = s;
  } else {
    d.ctx = new mlx::data::Buffer(s);
  }
  return d;
}

inline mlxd_buffer& mlxd_buffer_set_(mlxd_buffer& d, mlx::data::Buffer&& s) {
  if (d.ctx) {
    *static_cast<mlx::data::Buffer*>(d.ctx) = std::move(s);
  } else {
    d.ctx = new mlx::data::Buffer(std::move(s));
  }
  return d;
}

inline mlx::data::Buffer& mlxd_buffer_get_(mlxd_buffer d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_buffer");
  }
  return *static_cast<mlx::data::Buffer*>(d.ctx);
}

inline void mlxd_buffer_free_(mlxd_buffer d) {
  if (d.ctx) {
    delete static_cast<mlx::data::Buffer*>(d.ctx);
  }
}

#endif
