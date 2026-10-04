#include "mlx/data/c/version.h"

#include <string>
#include <vector>

#include "mlx/data/core/Version.h"
#include "mlx/data/c/error.h"
#include "mlx/data/c/private/string.h"
#include "mlx/data/c/private/vector_string.h"

extern "C" int mlxd_version(mlxd_string* out) {
  try {
    mlxd_string_set_(*out, mlx::data::core::version());
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_libs_version(
    mlxd_vector_string* out_names,
    mlxd_vector_string* out_versions) {
  try {
    std::vector<std::string> names;
    std::vector<std::string> versions;
    for (auto& kv : mlx::data::core::libs_version()) {
      names.push_back(kv.first);
      versions.push_back(kv.second);
    }
    mlxd_vector_string_set_(*out_names, std::move(names));
    mlxd_vector_string_set_(*out_versions, std::move(versions));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}
