#ifndef MLXD_BUFFER_H
#define MLXD_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "mlx/data/c/sample.h"

// Incomplete handle type; defined in mlx/data/c/stream.h (buffer.h and
// stream.h reference each other's handles).
typedef struct mlxd_stream_ mlxd_stream;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_buffer Buffer
 * The Buffer container handle: finite, indexable sample sequence.
 */
/**@{*/

/**
 * A buffer of samples. Wraps mlx::data::Buffer; operations are lazy —
 * they build a pipeline and the work happens at mlxd_buffer_get.
 */
typedef struct mlxd_buffer_ {
  void* ctx;
} mlxd_buffer;

/**
 * Returns a new empty handle; not a usable buffer. Buffers come from
 * mlxd_buffer_from_samples, mlxd_buffer_files_from_tar, or the operations
 * below.
 */
mlxd_buffer mlxd_buffer_new(void);

/**
 * Free the buffer handle. Safe no-op returning 0 on an empty handle. The
 * handle is unusable after this call; obtain a fresh one with
 * mlxd_buffer_new().
 */
int mlxd_buffer_free(mlxd_buffer buffer);

/**
 * Copies `src`'s contents into `dst`.
 *
 * @returns 0 on success, 1 if `src` is an empty handle.
 */
int mlxd_buffer_set(mlxd_buffer* dst, mlxd_buffer src);

/**
 * Writes the sample at `idx` to `out` (concrete: this is where lazy
 * pipelines do their work).
 *
 * @returns 0 on success, 2 if `idx` is out of range, 1 on an empty handle
 * or other error.
 */
int mlxd_buffer_get(mlxd_sample* out, mlxd_buffer buffer, int64_t idx);

/**
 * Writes the number of samples to `out`.
 *
 * @returns 0 on success, 1 if `buffer` is an empty handle.
 */
int mlxd_buffer_size(int64_t* out, mlxd_buffer buffer);

/**
 * Fixed-size batching with per-key padding. `pad_keys`/`pad_values` and
 * `dim_keys`/`dim_values` are parallel arrays (empty map: NULL, 0).
 */
int mlxd_buffer_batch(
    mlxd_buffer* out,
    mlxd_buffer buffer,
    int64_t batch_size,
    const char** pad_keys,
    const double* pad_values,
    size_t pad_num,
    const char** dim_keys,
    const int* dim_values,
    size_t dim_num);

/**
 * Variable-size batching: element i of `batch_sizes` samples per batch,
 * with the same padding maps as mlxd_buffer_batch.
 */
int mlxd_buffer_batch_sizes(
    mlxd_buffer* out,
    mlxd_buffer buffer,
    const int64_t* batch_sizes,
    size_t batch_sizes_num,
    const char** pad_keys,
    const double* pad_values,
    size_t pad_num,
    const char** dim_keys,
    const int* dim_values,
    size_t dim_num);

/**
 * Shape-aware batching: groups consecutive samples until `key`'s array
 * reaches `max_data_size` (0 = batch everything), respecting
 * `min_data_size` (0 = ignore).
 */
int mlxd_buffer_dynamic_batch(
    mlxd_buffer* out,
    mlxd_buffer buffer,
    const char* key,
    int64_t min_data_size,
    int64_t max_data_size,
    const char** pad_keys,
    const double* pad_values,
    size_t pad_num,
    const char** dim_keys,
    const int* dim_values,
    size_t dim_num,
    bool drop_outliers);

/**
 * Same as mlxd_buffer_dynamic_batch, but sizes come from `key` in
 * `size_buffer` instead of the data buffer itself.
 */
int mlxd_buffer_dynamic_batch_from_size_buffer(
    mlxd_buffer* out,
    mlxd_buffer buffer,
    mlxd_buffer size_buffer,
    const char* key,
    int64_t min_data_size,
    int64_t max_data_size,
    const char** pad_keys,
    const double* pad_values,
    size_t pad_num,
    const char** dim_keys,
    const int* dim_values,
    size_t dim_num,
    bool drop_outliers);

/**
 * Background loading that PRESERVES sample order (unlike
 * mlxd_stream_prefetch, which does not).
 */
int mlxd_buffer_ordered_prefetch(
    mlxd_stream* out,
    mlxd_buffer buffer,
    int prefetch_size,
    int num_thread);

/**
 * Keeps only samples belonging to `partition` of `num_partitions`
 * (sample idx % num_partitions == partition).
 */
int mlxd_buffer_partition(
    mlxd_buffer* out,
    mlxd_buffer buffer,
    int64_t num_partitions,
    int64_t partition);

/**
 * mlxd_buffer_partition applied only when `cond` is true; otherwise the
 * buffer passes through unchanged.
 */
int mlxd_buffer_partition_if(
    mlxd_buffer* out,
    mlxd_buffer buffer,
    bool cond,
    int64_t num_partitions,
    int64_t partition);

/**
 * Appends `other`'s samples after `buffer`'s.
 */
int mlxd_buffer_append(mlxd_buffer* out, mlxd_buffer buffer, mlxd_buffer other);

/**
 * Reorders the buffer by `perm` (element i of the result is sample
 * perm[i]).
 */
int mlxd_buffer_perm(
    mlxd_buffer* out,
    mlxd_buffer buffer,
    const int64_t* perm,
    size_t perm_num);

/**
 * Randomly permutes the buffer.
 */
int mlxd_buffer_shuffle(mlxd_buffer* out, mlxd_buffer buffer);

/**
 * mlxd_buffer_shuffle applied only when `cond` is true.
 */
int mlxd_buffer_shuffle_if(mlxd_buffer* out, mlxd_buffer buffer, bool cond);

/**
 * Converts the buffer to a stream visiting the same samples in order.
 */
int mlxd_buffer_to_stream(mlxd_stream* out, mlxd_buffer buffer);

/**
 * Materializes all lazy pipeline stages; the result's mlxd_buffer_get does
 * no further work.
 */
int mlxd_buffer_concretize(mlxd_buffer* out, mlxd_buffer buffer);

/**
 * Builds a buffer from `num` samples (map contents copied, array buffers
 * shared) — wraps buffer_from_vector.
 */
int mlxd_buffer_from_samples(
    mlxd_buffer* out,
    const mlxd_sample* samples,
    size_t num);

/**
 * Builds a buffer of one sample per file contained in `tarfile`, with key
 * "file" (and "tarfile" when nested) — wraps files_from_tar.
 */
int mlxd_buffer_files_from_tar(
    mlxd_buffer* out,
    const char* tarfile,
    bool nested,
    int num_threads);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
