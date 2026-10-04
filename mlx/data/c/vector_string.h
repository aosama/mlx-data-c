#ifndef MLXD_VECTOR_STRING_H
#define MLXD_VECTOR_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_vector_string Vector of strings
 * Owned list-of-strings handle (e.g. sample key lists).
 */
/**@{*/

/**
 * A vector of strings owned by the caller. Wraps a C++ std::vector<std::string>.
 */
typedef struct mlxd_vector_string_ {
  void* ctx;
} mlxd_vector_string;

/**
 * Returns a new empty vector.
 */
mlxd_vector_string mlxd_vector_string_new(void);

/**
 * Free the vector. Safe no-op returning 0 on an empty handle.
 */
int mlxd_vector_string_free(mlxd_vector_string vec);

/**
 * Writes the number of strings in the vector to `out`.
 *
 * @returns 0 on success, 1 if `vec` is an empty handle.
 */
int mlxd_vector_string_size(size_t* out, mlxd_vector_string vec);

/**
 * Writes a pointer to element `i`'s contents to `out`.
 *
 * The pointer is owned by `vec` and stays valid until the handle is freed
 * or its contents replaced.
 *
 * @returns 0 on success, 2 if `i` is out of range, 1 if `vec` is an empty
 * handle.
 */
int mlxd_vector_string_get(const char** out, mlxd_vector_string vec, size_t i);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
