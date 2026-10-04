#ifndef MLXD_STREAM_H
#define MLXD_STREAM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "mlx/data/c/sample.h"

// Incomplete handle type; defined in mlx/data/c/buffer.h (buffer.h and
// stream.h reference each other's handles).
typedef struct mlxd_buffer_ mlxd_buffer;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_stream Stream
 * The Stream container handle: resettable, possibly infinite sample
 * sequence.
 */
/**@{*/

/**
 * A stream of samples. Wraps mlx::data::Stream; operations are lazy —
 * they build a pipeline and the work happens at mlxd_stream_next.
 *
 * Stream::buffered (background loading with a user on_refill callback)
 * is not wrapped yet; it arrives with the closures milestone.
 */
typedef struct mlxd_stream_ {
  void* ctx;
} mlxd_stream;

/**
 * Returns a new empty handle; not a usable stream. Streams come from the
 * reader constructors or the operations below.
 */
mlxd_stream mlxd_stream_new(void);

/**
 * Free the stream handle. Safe no-op returning 0 on an empty handle. The
 * handle is unusable after this call; obtain a fresh one with
 * mlxd_stream_new().
 */
int mlxd_stream_free(mlxd_stream stream);

/**
 * Copies `src`'s contents into `dst`.
 *
 * @returns 0 on success, 1 if `src` is an empty handle.
 */
int mlxd_stream_set(mlxd_stream* dst, mlxd_stream src);

/**
 * Writes the next sample to `out` (concrete: this is where lazy pipelines
 * do their work).
 *
 * END-OF-STREAM CONTRACT: at exhaustion this returns 0 with an EMPTY
 * sample (mlxd_sample_size == 0) — the same signal the Python binding
 * maps to None. Poll mlxd_stream_reset to iterate again.
 *
 * @returns 0 on success (including end-of-stream), 1 on an empty handle.
 */
int mlxd_stream_next(mlxd_sample* out, mlxd_stream stream);

/**
 * Rewinds the stream to its beginning.
 *
 * @returns 0 on success, 1 on an empty handle.
 */
int mlxd_stream_reset(mlxd_stream stream);

/**
 * Fixed-size batching with per-key padding (same map convention as
 * mlxd_buffer_batch).
 */
int mlxd_stream_batch(
    mlxd_stream* out,
    mlxd_stream stream,
    int64_t batch_size,
    const char** pad_keys,
    const double* pad_values,
    size_t pad_num,
    const char** dim_keys,
    const int* dim_values,
    size_t dim_num);

/**
 * Shape-aware batching on streams: looks ahead up to `buffer_size`
 * samples, sizes batches by `key`'s array size between `min_data_size`
 * and `max_data_size` (0 = ignore / batch everything), optionally
 * shuffling the batch order and dropping outliers that never fit the
 * window; gives up after `max_skipped_samples`.
 */
int mlxd_stream_dynamic_batch(
    mlxd_stream* out,
    mlxd_stream stream,
    int64_t buffer_size,
    const char* key,
    int64_t min_data_size,
    int64_t max_data_size,
    const char** pad_keys,
    const double* pad_values,
    size_t pad_num,
    const char** dim_keys,
    const int* dim_values,
    size_t dim_num,
    bool shuffle,
    bool drop_outliers,
    int64_t max_skipped_samples,
    int num_thread);

/**
 * Reads each sample's `key` as CSV text and parses it into typed keys —
 * the per-sample counterpart of mlxd_stream_csv_reader.
 */
int mlxd_stream_csv_reader_from_key(
    mlxd_stream* out,
    mlxd_stream stream,
    const char* key,
    char sep,
    char quote,
    bool from_memory,
    const char* local_prefix);

/**
 * Reads each sample's `key` as a (optionally zipped) text file and emits
 * one sample per line, stored under `dst_key`.
 */
int mlxd_stream_line_reader_from_key(
    mlxd_stream* out,
    mlxd_stream stream,
    const char* key,
    const char* dst_key,
    bool from_memory,
    bool unzip,
    const char* local_prefix);

/**
 * Keeps only samples belonging to `partition` of `num_partitions`.
 */
int mlxd_stream_partition(
    mlxd_stream* out,
    mlxd_stream stream,
    int64_t num_partitions,
    int64_t partition);

/**
 * mlxd_stream_partition applied only when `cond` is true.
 */
int mlxd_stream_partition_if(
    mlxd_stream* out,
    mlxd_stream stream,
    bool cond,
    int64_t num_partitions,
    int64_t partition);

/**
 * Background loading that does NOT preserve sample order (contrast
 * mlxd_buffer_ordered_prefetch).
 */
int mlxd_stream_prefetch(
    mlxd_stream* out,
    mlxd_stream stream,
    int prefetch_size,
    int num_thread);

/**
 * mlxd_stream_prefetch applied only when `cond` is true.
 */
int mlxd_stream_prefetch_if(
    mlxd_stream* out,
    mlxd_stream stream,
    bool cond,
    int prefetch_size,
    int num_thread);

/**
 * Repeats the stream `num_time` times (a negative value repeats forever).
 */
int mlxd_stream_repeat(mlxd_stream* out, mlxd_stream stream, int64_t num_time);

/**
 * Shuffles the stream with a look-ahead window of `buffer_size` samples.
 */
int mlxd_stream_shuffle(mlxd_stream* out, mlxd_stream stream, int64_t buffer_size);

/**
 * mlxd_stream_shuffle applied only when `cond` is true.
 */
int mlxd_stream_shuffle_if(
    mlxd_stream* out,
    mlxd_stream stream,
    bool cond,
    int64_t buffer_size);

/**
 * Emits sliding windows of `size` elements along `dim` (default -1: last)
 * of `key`'s array, stepping `stride`; window start indices go to
 * `index_key` when non-empty.
 */
int mlxd_stream_sliding_window(
    mlxd_stream* out,
    mlxd_stream stream,
    const char* key,
    int64_t size,
    int64_t stride,
    int dim,
    const char* index_key);

/**
 * Drains the stream into a buffer (only for streams you can exhaust).
 */
int mlxd_stream_to_buffer(mlxd_buffer* out, mlxd_stream stream);

/**
 * Returns a stream reading CSV records from `filename`.
 */
int mlxd_stream_csv_reader(
    mlxd_stream* out,
    const char* filename,
    char sep,
    char quote,
    const char* local_prefix);

/**
 * Returns a stream reading CSV records from an in-memory string.
 */
int mlxd_stream_csv_reader_from_string(
    mlxd_stream* out,
    const char* contents,
    char sep,
    char quote);

/**
 * Returns a stream emitting one sample per line of `filename`, stored
 * under `key`.
 */
int mlxd_stream_line_reader(
    mlxd_stream* out,
    const char* filename,
    const char* key,
    bool unzip,
    const char* local_prefix);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
