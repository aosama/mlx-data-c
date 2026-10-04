#include "mlx/data/c/error.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>

namespace {

void mlxd_error_handler_default_(const char* msg, void* data) {
  (void)data;
  printf("MLXD error: %s\n", msg);
  exit(-1);
}

std::shared_ptr<void> mlxd_error_handler_data_ = nullptr;
mlx_error_handler_func mlxd_error_handler_ = mlxd_error_handler_default_;

} // namespace

extern "C" void mlxd_set_error_handler(
    mlx_error_handler_func handler,
    void* data,
    void (*dtor)(void*)) {
  // Replacing the shared_ptr runs the previous dtor, if any. Unlike mlx-c,
  // the data pointer stays reachable even without a dtor: the handler
  // contract guarantees it always receives `data` back.
  if (dtor) {
    mlxd_error_handler_data_ = std::shared_ptr<void>(data, dtor);
  } else {
    mlxd_error_handler_data_ =
        std::shared_ptr<void>(data, [](void*) {});
  }
  mlxd_error_handler_ = handler ? handler : mlxd_error_handler_default_;
}

extern "C" void
_mlxd_error(const char* file, const int line, const char* fmt, ...) {
  va_list args, args_copy;
  va_start(args, fmt);

  va_copy(args_copy, args);
  int size = vsnprintf(nullptr, 0, fmt, args_copy);
  va_end(args_copy);
  int size_loc = snprintf(nullptr, 0, " at %s:%d", file, line);

  std::vector<char> msg(size + size_loc + 1);
  size = vsnprintf(msg.data(), size + 1, fmt, args);
  snprintf(msg.data() + size, size_loc + 1, " at %s:%d", file, line);
  va_end(args);

  mlxd_error_handler_(msg.data(), mlxd_error_handler_data_.get());
}
