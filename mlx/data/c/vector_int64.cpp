#include "mlx/data/c/vector_int64.h"

#include "mlx/data/c/error.h"
#include "mlx/data/c/private/vector_int64.h"

extern "C" mlxd_vector_int64 mlxd_vector_int64_new(void) {
  try {
    return mlxd_vector_int64_new_(std::vector<int64_t>());
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_vector_int64_new_();
  }
}

extern "C" int mlxd_vector_int64_free(mlxd_vector_int64 vec) {
  try {
    mlxd_vector_int64_free_(vec);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_vector_int64_size(size_t* out, mlxd_vector_int64 vec) {
  try {
    *out = mlxd_vector_int64_get_(vec).size();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int
mlxd_vector_int64_get(int64_t* out, mlxd_vector_int64 vec, size_t i) {
  try {
    std::vector<int64_t>& cpp_vec = mlxd_vector_int64_get_(vec);
    if (i >= cpp_vec.size()) {
      return 2;
    }
    *out = cpp_vec[i];
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_vector_int64_data(
    const int64_t** out,
    size_t* n,
    mlxd_vector_int64 vec) {
  try {
    std::vector<int64_t>& cpp_vec = mlxd_vector_int64_get_(vec);
    *out = cpp_vec.data();
    *n = cpp_vec.size();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}
