#ifndef MLXD_VERSION_H
#define MLXD_VERSION_H

#include "mlx/data/c/string.h"
#include "mlx/data/c/vector_string.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_version Version
 * mlx-data version information.
 */
/**@{*/

/**
 * Writes the mlx-data version string to `out`.
 *
 * @returns 0 on success, 1 on error.
 */
int mlxd_version(mlxd_string* out);

/**
 * Writes the optional-backend versions (SndFile, FFmpeg, ...) to `out_names`
 * and `out_versions` (same length, same order).
 *
 * @returns 0 on success, 1 on error.
 */
int mlxd_libs_version(
    mlxd_vector_string* out_names,
    mlxd_vector_string* out_versions);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
