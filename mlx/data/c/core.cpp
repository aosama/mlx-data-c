#include "mlx/data/c/core.h"

#include "mlx/data/core/Levenshtein.h"
#include "mlx/data/core/State.h"
#include "mlx/data/core/Utils.h"
#include "mlx/data/c/error.h"
#include "mlx/data/c/private/array.h"

extern "C" int mlxd_set_state(int64_t seed) {
  try {
    mlx::data::core::set_state(seed);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_levenshtein(
    mlxd_array* out,
    mlxd_array a,
    mlxd_array a_length,
    mlxd_array b,
    mlxd_array b_length) {
  try {
    mlxd_array_set_(
        *out,
        mlx::data::core::levenshtein(
            mlxd_array_get_(a),
            mlxd_array_get_(a_length),
            mlxd_array_get_(b),
            mlxd_array_get_(b_length)));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_uniq(
    mlxd_array* out,
    mlxd_array* out_length,
    mlxd_array src,
    mlxd_array src_length,
    int dim,
    double pad) {
  try {
    auto result = mlx::data::core::uniq(
        mlxd_array_get_(src), mlxd_array_get_(src_length), dim, pad);
    mlxd_array_set_(*out, result.first);
    mlxd_array_set_(*out_length, result.second);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_remove(
    mlxd_array* out,
    mlxd_array* out_length,
    mlxd_array src,
    mlxd_array src_length,
    int dim,
    double value,
    double pad) {
  try {
    auto result = mlx::data::core::remove(
        mlxd_array_get_(src), mlxd_array_get_(src_length), dim, value, pad);
    mlxd_array_set_(*out, result.first);
    mlxd_array_set_(*out_length, result.second);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}
