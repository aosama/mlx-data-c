#ifndef MLXD_CLOSURE_PRIVATE_H
#define MLXD_CLOSURE_PRIVATE_H

#include <functional>
#include <memory>
#include <stdexcept>

#include "mlx/data/Array.h"
#include "mlx/data/Buffer.h"
#include "mlx/data/Sample.h"
#include "mlx/data/Stream.h"
#include "mlx/data/c/closure.h"
#include "mlx/data/c/private/array.h"
#include "mlx/data/c/private/buffer.h"
#include "mlx/data/c/private/sample.h"

// Shared closure state: the raw function pointer plus the owning ctx.
// The closure handle and every pipeline built from it hold a
// shared_ptr to this state, so fn/ctx outlive the handle.
struct mlxd_closure_state_ {
  void* fn;
  std::shared_ptr<void> ctx;
};

inline mlxd_closure_array mlxd_closure_array_new_() {
  return mlxd_closure_array({nullptr});
}

inline std::shared_ptr<mlxd_closure_state_>& mlxd_closure_array_state_(
    mlxd_closure_array d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_closure_array");
  }
  return *static_cast<std::shared_ptr<mlxd_closure_state_>*>(d.ctx);
}

inline mlxd_closure_sample mlxd_closure_sample_new_() {
  return mlxd_closure_sample({nullptr});
}

inline std::shared_ptr<mlxd_closure_state_>& mlxd_closure_sample_state_(
    mlxd_closure_sample d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_closure_sample");
  }
  return *static_cast<std::shared_ptr<mlxd_closure_state_>*>(d.ctx);
}

inline mlxd_closure_buffer mlxd_closure_buffer_new_() {
  return mlxd_closure_buffer({nullptr});
}

inline std::shared_ptr<mlxd_closure_state_>& mlxd_closure_buffer_state_(
    mlxd_closure_buffer d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_closure_buffer");
  }
  return *static_cast<std::shared_ptr<mlxd_closure_state_>*>(d.ctx);
}

// Builds the std::function objects the C++ ops consume. Each lambda
// holds its own reference to the state: freeing the C handle does not
// tear down a live pipeline.

inline std::function<std::shared_ptr<mlx::data::Array>(
    const std::shared_ptr<const mlx::data::Array>&)>
mlxd_key_transform_fn_(std::shared_ptr<mlxd_closure_state_> state) {
  return [state](const std::shared_ptr<const mlx::data::Array>& input)
      -> std::shared_ptr<mlx::data::Array> {
    mlxd_array in_handle =
        mlxd_array_new_(std::const_pointer_cast<mlx::data::Array>(input));
    mlxd_array out_handle =
        reinterpret_cast<mlxd_closure_array_fn>(state->fn)(
            in_handle, state->ctx.get());
    mlxd_array_free_(in_handle);
    if (!out_handle.ctx) {
      throw std::runtime_error(
          "key_transform closure returned an empty handle");
    }
    std::shared_ptr<mlx::data::Array> result = mlxd_array_get_(out_handle);
    mlxd_array_free_(out_handle);
    return result;
  };
}

inline std::function<mlx::data::Sample(const mlx::data::Sample&)>
mlxd_sample_transform_fn_(std::shared_ptr<mlxd_closure_state_> state) {
  return [state](const mlx::data::Sample& input) -> mlx::data::Sample {
    mlxd_sample in_handle = mlxd_sample_new_(input);
    mlxd_sample out_handle =
        reinterpret_cast<mlxd_closure_sample_fn>(state->fn)(
            in_handle, state->ctx.get());
    mlxd_sample_free_(in_handle);
    if (!out_handle.ctx) {
      throw std::runtime_error(
          "sample_transform closure returned an empty handle");
    }
    mlx::data::Sample result = std::move(mlxd_sample_get_(out_handle));
    mlxd_sample_free_(out_handle);
    return result;
  };
}

inline std::function<mlx::data::Buffer(const mlx::data::Buffer&)>
mlxd_buffer_transform_fn_(std::shared_ptr<mlxd_closure_state_> state) {
  return [state](const mlx::data::Buffer& input) -> mlx::data::Buffer {
    mlxd_buffer in_handle = mlxd_buffer_new_(input);
    mlxd_buffer out_handle =
        reinterpret_cast<mlxd_closure_buffer_fn>(state->fn)(
            in_handle, state->ctx.get());
    mlxd_buffer_free_(in_handle);
    if (!out_handle.ctx) {
      throw std::runtime_error("buffered closure returned an empty handle");
    }
    mlx::data::Buffer result = std::move(mlxd_buffer_get_(out_handle));
    mlxd_buffer_free_(out_handle);
    return result;
  };
}

#endif
