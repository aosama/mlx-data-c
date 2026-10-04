#include "mlx/data/c/vector_string.h"

#include "mlx/data/c/error.h"
#include "mlx/data/c/private/vector_string.h"

extern "C" mlxd_vector_string mlxd_vector_string_new(void) {
  try {
    return mlxd_vector_string_new_(std::vector<std::string>());
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_vector_string_new_();
  }
}

extern "C" int mlxd_vector_string_free(mlxd_vector_string vec) {
  try {
    mlxd_vector_string_free_(vec);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_vector_string_size(size_t* out, mlxd_vector_string vec) {
  try {
    *out = mlxd_vector_string_get_(vec).size();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int
mlxd_vector_string_get(const char** out, mlxd_vector_string vec, size_t i) {
  try {
    std::vector<std::string>& cpp_vec = mlxd_vector_string_get_(vec);
    if (i >= cpp_vec.size()) {
      return 2;
    }
    *out = cpp_vec[i].c_str();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}
