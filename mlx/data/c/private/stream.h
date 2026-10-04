#ifndef MLXD_STREAM_PRIVATE_H
#define MLXD_STREAM_PRIVATE_H

#include "mlx/data/Stream.h"
#include "mlx/data/c/stream.h"

inline mlxd_stream mlxd_stream_new_() {
  return mlxd_stream({nullptr});
}

inline mlxd_stream mlxd_stream_new_(const mlx::data::Stream& s) {
  return mlxd_stream({new mlx::data::Stream(s)});
}

inline mlxd_stream mlxd_stream_new_(mlx::data::Stream&& s) {
  return mlxd_stream({new mlx::data::Stream(std::move(s))});
}

inline mlxd_stream& mlxd_stream_set_(mlxd_stream& d, const mlx::data::Stream& s) {
  if (d.ctx) {
    *static_cast<mlx::data::Stream*>(d.ctx) = s;
  } else {
    d.ctx = new mlx::data::Stream(s);
  }
  return d;
}

inline mlxd_stream& mlxd_stream_set_(mlxd_stream& d, mlx::data::Stream&& s) {
  if (d.ctx) {
    *static_cast<mlx::data::Stream*>(d.ctx) = std::move(s);
  } else {
    d.ctx = new mlx::data::Stream(std::move(s));
  }
  return d;
}

inline mlx::data::Stream& mlxd_stream_get_(mlxd_stream d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_stream");
  }
  return *static_cast<mlx::data::Stream*>(d.ctx);
}

inline void mlxd_stream_free_(mlxd_stream d) {
  if (d.ctx) {
    delete static_cast<mlx::data::Stream*>(d.ctx);
  }
}

#endif
