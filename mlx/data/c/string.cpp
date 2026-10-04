#include "mlx/data/c/string.h"

#include "mlx/data/c/error.h"
#include "mlx/data/c/private/string.h"

extern "C" mlxd_string mlxd_string_new(const char* c_str) {
  try {
    if (c_str) {
      return mlxd_string_new_(std::string(c_str));
    }
    return mlxd_string_new_(std::string());
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_string_new_();
  }
}

extern "C" int mlxd_string_free(mlxd_string str) {
  try {
    mlxd_string_free_(str);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_string_c_str(const char** out, mlxd_string str) {
  try {
    *out = mlxd_string_get_(str).c_str();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_string_size(size_t* out, mlxd_string str) {
  try {
    *out = mlxd_string_get_(str).size();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}
