# Private C++ bridge

Never installed; consumed only by the `.cpp` implementations in `mlx/data/c/`.

Every wrapped C++ type `X` gets an inline bridge header here named after its
public header (`mlx/data/c/<x>.h` → `mlx/data/c/private/<x>.h`) with four
helper families casting the handle's `void* ctx` to the C++ object:

```cpp
#include "mlx/data/c/<x>.h"
#include "mlx/data/<UpstreamHeader>.h"

// Empty handle: same as mlxd_x_new().
inline mlxd_x mlxd_x_new_() {
  return mlxd_x({nullptr});
}

// Handle owning a fresh copy of a C++ object.
inline mlxd_x mlxd_x_new_(const mlx::data::X& s) {
  return mlxd_x({new mlx::data::X(s)});
}
inline mlxd_x mlxd_x_new_(mlx::data::X&& s) {
  return mlxd_x({new mlx::data::X(std::move(s))});
}

// Overwrite the handle's target in place.
inline mlxd_x& mlxd_x_set_(mlxd_x& d, const mlx::data::X& s);
inline mlxd_x& mlxd_x_set_(mlxd_x& d, mlx::data::X&& s);

// Borrow the C++ object. Throws std::runtime_error on an empty handle,
// which the surrounding extern "C" wrapper turns into status 1.
inline mlx::data::X& mlxd_x_get_(mlxd_x d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_x");
  }
  return *static_cast<mlx::data::X*>(d.ctx);
}

// Delete the target; no-op on an empty handle.
inline void mlxd_x_free_(mlxd_x d) {
  if (d.ctx) {
    delete static_cast<mlx::data::X*>(d.ctx);
  }
}
```

For types wrapped by `std::shared_ptr` (Array, Trie, Graph), the ctx holds the
shared_ptr itself, so `_new_` takes `std::shared_ptr<X>const&` and `&&`
overloads and `_free_` deletes the shared_ptr.
