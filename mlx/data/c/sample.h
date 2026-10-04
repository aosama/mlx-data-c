#ifndef MLXD_SAMPLE_H
#define MLXD_SAMPLE_H

#include <stdint.h>

#include "mlx/data/c/array.h"
#include "mlx/data/c/vector_string.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_sample Sample
 * The sample dict handle: string keys to mlxd_array values.
 */
/**@{*/

/**
 * A sample: the unit that flows through buffers, streams, and operations.
 * Wraps a C++ unordered_map<string, shared_ptr<Array>>.
 *
 * An EMPTY sample (mlxd_sample_size == 0) returned by mlxd_stream_next
 * signals end-of-stream — the same signal the Python binding uses.
 */
typedef struct mlxd_sample_ {
  void* ctx;
} mlxd_sample;

/**
 * Returns a new empty sample.
 */
mlxd_sample mlxd_sample_new(void);

/**
 * Free the sample. Arrays fetched with mlxd_sample_get stay alive through
 * their own handles. Safe no-op returning 0 on an empty handle. The handle
 * is unusable after this call; obtain a fresh one with mlxd_sample_new().
 */
int mlxd_sample_free(mlxd_sample sample);

/**
 * Copies `src`'s contents into `dst` (keys copied, array buffers shared).
 *
 * @returns 0 on success, 1 if `src` is an empty handle.
 */
int mlxd_sample_set(mlxd_sample* dst, mlxd_sample src);

/**
 * Writes the number of keys to `out`. 0 for the empty/end-of-stream sample.
 *
 * @returns 0 on success, 1 if `sample` is an empty handle.
 */
int mlxd_sample_size(int64_t* out, mlxd_sample sample);

/**
 * Writes the key list to `out` (caller-owned handle, overwritten).
 *
 * @returns 0 on success, 1 if `sample` is an empty handle.
 */
int mlxd_sample_keys(mlxd_vector_string* out, mlxd_sample sample);

/**
 * Shares the array stored under `key` into `out` (both then reference the
 * same buffer, which stays valid after the sample is modified or freed).
 *
 * @returns 0 on success, 2 if `key` is absent, 1 if `sample` is an empty
 * handle or `key` is NULL.
 */
int mlxd_sample_get(mlxd_array* out, mlxd_sample sample, const char* key);

/**
 * Stores `value` under `key`, inserting or overwriting. The sample shares
 * ownership of the array's buffer.
 *
 * @returns 0 on success, 1 on an empty handle or a NULL key.
 */
int mlxd_sample_set_key(mlxd_sample sample, const char* key, mlxd_array value);

/**
 * Removes `key` from the sample.
 *
 * @returns 0 on success, 2 if `key` is absent, 1 on an empty handle or a
 * NULL key.
 */
int mlxd_sample_erase(mlxd_sample sample, const char* key);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
