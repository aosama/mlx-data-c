#ifndef MLXD_VECTOR_INT64_H
#define MLXD_VECTOR_INT64_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_vector_int64 Vector of int64
 * Owned int64 list handle (e.g. token id sequences).
 */
/**@{*/

/**
 * A vector of int64 values owned by the caller. Wraps a C++
 * std::vector<int64_t>.
 */
typedef struct mlxd_vector_int64_ {
  void* ctx;
} mlxd_vector_int64;

/**
 * Returns a new empty vector.
 */
mlxd_vector_int64 mlxd_vector_int64_new(void);

/**
 * Free the vector. Safe no-op returning 0 on an empty handle.
 */
int mlxd_vector_int64_free(mlxd_vector_int64 vec);

/**
 * Writes the number of elements in the vector to `out`.
 *
 * @returns 0 on success, 1 if `vec` is an empty handle.
 */
int mlxd_vector_int64_size(size_t* out, mlxd_vector_int64 vec);

/**
 * Writes element `i` to `out`.
 *
 * @returns 0 on success, 2 if `i` is out of range, 1 if `vec` is an empty
 * handle.
 */
int mlxd_vector_int64_get(int64_t* out, mlxd_vector_int64 vec, size_t i);

/**
 * Writes a direct pointer to the vector contents to `out` and the element
 * count to `n`, for bulk consumption without per-element calls.
 *
 * The pointer is owned by `vec` and stays valid until the handle is freed
 * or its contents replaced.
 *
 * @returns 0 on success, 1 if `vec` is an empty handle.
 */
int mlxd_vector_int64_data(const int64_t** out, size_t* n, mlxd_vector_int64 vec);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
