#include "mlx/data/c/closure.h"

#include <stdexcept>

#include "mlx/data/c/error.h"
#include "mlx/data/c/private/closure.h"

namespace {

// Shared constructor tail: own the state in a shared_ptr held by the
// handle, so pipelines can copy that shared_ptr and outlive the handle.
template <typename HandleT>
int mlxd_closure_init_(
    HandleT* out,
    void* fn,
    void* ctx,
    void (*dtor)(void*)) {
  try {
    if (fn == nullptr) {
      throw std::invalid_argument("closure function must not be NULL");
    }
    auto state = std::make_shared<mlxd_closure_state_>();
    state->fn = fn;
    if (dtor) {
      state->ctx = std::shared_ptr<void>(ctx, dtor);
    } else {
      state->ctx = std::shared_ptr<void>(ctx, [](void*) {});
    }
    out->ctx = new std::shared_ptr<mlxd_closure_state_>(std::move(state));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    out->ctx = nullptr;
    return 1;
  }
  return 0;
}

template <typename HandleT>
int mlxd_closure_release_(HandleT closure) {
  try {
    if (closure.ctx) {
      delete static_cast<std::shared_ptr<mlxd_closure_state_>*>(closure.ctx);
    }
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

} // namespace

extern "C" int mlxd_closure_array_new(
    mlxd_closure_array* out,
    mlxd_closure_array_fn fn,
    void* ctx,
    void (*dtor)(void*)) {
  return mlxd_closure_init_(out, reinterpret_cast<void*>(fn), ctx, dtor);
}

extern "C" int mlxd_closure_array_free(mlxd_closure_array closure) {
  return mlxd_closure_release_(closure);
}

extern "C" int mlxd_closure_sample_new(
    mlxd_closure_sample* out,
    mlxd_closure_sample_fn fn,
    void* ctx,
    void (*dtor)(void*)) {
  return mlxd_closure_init_(out, reinterpret_cast<void*>(fn), ctx, dtor);
}

extern "C" int mlxd_closure_sample_free(mlxd_closure_sample closure) {
  return mlxd_closure_release_(closure);
}

extern "C" int mlxd_closure_buffer_new(
    mlxd_closure_buffer* out,
    mlxd_closure_buffer_fn fn,
    void* ctx,
    void (*dtor)(void*)) {
  return mlxd_closure_init_(out, reinterpret_cast<void*>(fn), ctx, dtor);
}

extern "C" int mlxd_closure_buffer_free(mlxd_closure_buffer closure) {
  return mlxd_closure_release_(closure);
}
