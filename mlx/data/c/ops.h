#ifndef MLXD_OPS_H
#define MLXD_OPS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "mlx/data/c/bpe.h"
#include "mlx/data/c/buffer.h"
#include "mlx/data/c/closure.h"
#include "mlx/data/c/file_fetcher.h"
#include "mlx/data/c/stream.h"
#include "mlx/data/c/trie.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_ops Dataset operations
 *
 * Per-sample operations applied to every element of a Buffer or Stream,
 * in both plain and conditional `_if` forms (`_if(false, ...)` returns a
 * logically unchanged dataset). Operations are LAZY: they build a
 * pipeline and do no work until mlxd_buffer_get / mlxd_stream_next.
 *
 * THE output_key CONVENTION (applies to every `okey` parameter below):
 * an EMPTY `okey` (`""`) overwrites the input key; a non-empty `okey`
 * writes the result to a new key and leaves the input key unchanged.
 *
 * NULL PARAMETERS: key parameters that identify data (ikey, key, tarkey,
 * filename_key, old, replacement, size_key) and the semantics-critical
 * `okey` must not be NULL — NULL returns status 1. Parameters with an
 * upstream default accept NULL as that default: prefix/tar_prefix/
 * info_key/filename_prefix (""), preset ("default"), format ("RGB").
 * List parameters accept NULL only with a zero count.
 *
 * Media operations (image loading/saving, audio, video) need optional
 * mlx-data backends; when a backend is absent the operation fails at
 * data-access time with status 1 and the underlying C++ message.
 *
 * FILTERING ON BUFFER: the C++ core cannot remove elements from an
 * indexed buffer. A filter op (filter_by_shape, filter_key with
 * remove) that rejects a sample makes mlxd_buffer_get return status 1
 * ("cannot return empty sample"). For drop semantics, convert with
 * mlxd_buffer_to_stream, filter, then mlxd_stream_to_buffer.
 */
/**@{*/

/**
 * Tokenization mode (mirrors op::TokenizeMode).
 */
typedef enum {
  MLXD_TOKENIZE_SHORTEST = 0,
  MLXD_TOKENIZE_RAND = 1
} mlxd_tokenize_mode;

/**
 * Audio info selection (mirrors op::LoadAudioInfo).
 */
typedef enum {
  MLXD_LOAD_AUDIO_INFO_ALL = 0,
  MLXD_LOAD_AUDIO_INFO_NUM_FRAMES = 1,
  MLXD_LOAD_AUDIO_INFO_NUM_CHANNELS = 2,
  MLXD_LOAD_AUDIO_INFO_SAMPLE_RATE = 3,
  MLXD_LOAD_AUDIO_INFO_NUM_SECONDS = 4
} mlxd_load_audio_info;

/**
 * Audio resampling quality (mirrors op::LoadAudioResamplingQuality).
 */
typedef enum {
  MLXD_RESAMPLING_SINC_BEST = 0,
  MLXD_RESAMPLING_SINC_MEDIUM = 1,
  MLXD_RESAMPLING_SINC_FASTEST = 2,
  MLXD_RESAMPLING_ZERO_ORDER_HOLD = 3,
  MLXD_RESAMPLING_LINEAR = 4
} mlxd_load_audio_resampling_quality;

/* ---------------- filtering ---------------- */

int mlxd_buffer_filter_by_shape(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* key,
    int dim,
    int64_t low,
    int64_t high);
int mlxd_buffer_filter_by_shape_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* key,
    int dim,
    int64_t low,
    int64_t high);
int mlxd_stream_filter_by_shape(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* key,
    int dim,
    int64_t low,
    int64_t high);
int mlxd_stream_filter_by_shape_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* key,
    int dim,
    int64_t low,
    int64_t high);

int mlxd_buffer_filter_key(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* key,
    bool remove);
int mlxd_buffer_filter_key_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* key,
    bool remove);
int mlxd_stream_filter_key(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* key,
    bool remove);
int mlxd_stream_filter_key_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* key,
    bool remove);

/* ---------------- image transforms ---------------- */

int mlxd_buffer_image_center_crop(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey);
int mlxd_buffer_image_center_crop_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey);
int mlxd_stream_image_center_crop(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey);
int mlxd_stream_image_center_crop_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey);

int mlxd_buffer_image_channel_reduction(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* preset,
    const char* okey);
int mlxd_buffer_image_channel_reduction_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* preset,
    const char* okey);
int mlxd_stream_image_channel_reduction(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* preset,
    const char* okey);
int mlxd_stream_image_channel_reduction_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* preset,
    const char* okey);

int mlxd_buffer_image_random_area_crop(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    float area_low,
    float area_high,
    float aspect_low,
    float aspect_high,
    int num_trial,
    const char* okey);
int mlxd_buffer_image_random_area_crop_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    float area_low,
    float area_high,
    float aspect_low,
    float aspect_high,
    int num_trial,
    const char* okey);
int mlxd_stream_image_random_area_crop(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    float area_low,
    float area_high,
    float aspect_low,
    float aspect_high,
    int num_trial,
    const char* okey);
int mlxd_stream_image_random_area_crop_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    float area_low,
    float area_high,
    float aspect_low,
    float aspect_high,
    int num_trial,
    const char* okey);

int mlxd_buffer_image_random_crop(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey);
int mlxd_buffer_image_random_crop_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey);
int mlxd_stream_image_random_crop(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey);
int mlxd_stream_image_random_crop_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey);

int mlxd_buffer_image_random_h_flip(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    float prob,
    const char* okey);
int mlxd_buffer_image_random_h_flip_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    float prob,
    const char* okey);
int mlxd_stream_image_random_h_flip(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    float prob,
    const char* okey);
int mlxd_stream_image_random_h_flip_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    float prob,
    const char* okey);

int mlxd_buffer_image_resize(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey);
int mlxd_buffer_image_resize_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey);
int mlxd_stream_image_resize(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey);
int mlxd_stream_image_resize_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey);

int mlxd_buffer_image_resize_smallest_side(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int64_t size,
    const char* okey);
int mlxd_buffer_image_resize_smallest_side_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int64_t size,
    const char* okey);
int mlxd_stream_image_resize_smallest_side(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int64_t size,
    const char* okey);
int mlxd_stream_image_resize_smallest_side_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int64_t size,
    const char* okey);

int mlxd_buffer_image_rotate(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    double angle,
    bool crop,
    const char* okey);
int mlxd_buffer_image_rotate_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    double angle,
    bool crop,
    const char* okey);
int mlxd_stream_image_rotate(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    double angle,
    bool crop,
    const char* okey);
int mlxd_stream_image_rotate_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    double angle,
    bool crop,
    const char* okey);

/* ---------------- media loading ---------------- */

int mlxd_buffer_load_audio(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* prefix,
    bool info,
    bool from_memory,
    mlxd_load_audio_info info_type,
    int sample_rate,
    mlxd_load_audio_resampling_quality resampling_quality,
    const char* info_key,
    const char* okey);
int mlxd_buffer_load_audio_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    bool info,
    bool from_memory,
    mlxd_load_audio_info info_type,
    int sample_rate,
    mlxd_load_audio_resampling_quality resampling_quality,
    const char* info_key,
    const char* okey);
int mlxd_stream_load_audio(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* prefix,
    bool info,
    bool from_memory,
    mlxd_load_audio_info info_type,
    int sample_rate,
    mlxd_load_audio_resampling_quality resampling_quality,
    const char* info_key,
    const char* okey);
int mlxd_stream_load_audio_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    bool info,
    bool from_memory,
    mlxd_load_audio_info info_type,
    int sample_rate,
    mlxd_load_audio_resampling_quality resampling_quality,
    const char* info_key,
    const char* okey);

int mlxd_buffer_resample_audio(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int output_sample_rate,
    int input_sample_rate,
    const char* info_key,
    mlxd_load_audio_resampling_quality resampling_quality,
    const char* okey);
int mlxd_buffer_resample_audio_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int output_sample_rate,
    int input_sample_rate,
    const char* info_key,
    mlxd_load_audio_resampling_quality resampling_quality,
    const char* okey);
int mlxd_stream_resample_audio(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int output_sample_rate,
    int input_sample_rate,
    const char* info_key,
    mlxd_load_audio_resampling_quality resampling_quality,
    const char* okey);
int mlxd_stream_resample_audio_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int output_sample_rate,
    int input_sample_rate,
    const char* info_key,
    mlxd_load_audio_resampling_quality resampling_quality,
    const char* okey);

int mlxd_buffer_load_file(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* prefix,
    const char* okey);
int mlxd_buffer_load_file_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    const char* okey);
int mlxd_stream_load_file(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* prefix,
    const char* okey);
int mlxd_stream_load_file_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    const char* okey);

int mlxd_buffer_load_image(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* prefix,
    bool info,
    const char* format,
    bool from_memory,
    const char* okey);
int mlxd_buffer_load_image_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    bool info,
    const char* format,
    bool from_memory,
    const char* okey);
int mlxd_stream_load_image(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* prefix,
    bool info,
    const char* format,
    bool from_memory,
    const char* okey);
int mlxd_stream_load_image_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    bool info,
    const char* format,
    bool from_memory,
    const char* okey);

int mlxd_buffer_load_numpy(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* prefix,
    bool from_memory,
    const char* okey);
int mlxd_buffer_load_numpy_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    bool from_memory,
    const char* okey);
int mlxd_stream_load_numpy(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* prefix,
    bool from_memory,
    const char* okey);
int mlxd_stream_load_numpy_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    bool from_memory,
    const char* okey);

int mlxd_buffer_load_video(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* prefix,
    bool info,
    bool from_memory,
    const char* okey);
int mlxd_buffer_load_video_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    bool info,
    bool from_memory,
    const char* okey);
int mlxd_stream_load_video(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* prefix,
    bool info,
    bool from_memory,
    const char* okey);
int mlxd_stream_load_video_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    bool info,
    bool from_memory,
    const char* okey);

/* ---------------- padding ---------------- */

int mlxd_buffer_pad(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    int64_t lpad,
    int64_t rpad,
    double value,
    const char* okey);
int mlxd_buffer_pad_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t lpad,
    int64_t rpad,
    double value,
    const char* okey);
int mlxd_stream_pad(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    int64_t lpad,
    int64_t rpad,
    double value,
    const char* okey);
int mlxd_stream_pad_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t lpad,
    int64_t rpad,
    double value,
    const char* okey);

int mlxd_buffer_pad_to_multiple(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey);
int mlxd_buffer_pad_to_multiple_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey);
int mlxd_stream_pad_to_multiple(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey);
int mlxd_stream_pad_to_multiple_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey);

int mlxd_buffer_pad_to_size(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey);
int mlxd_buffer_pad_to_size_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey);
int mlxd_stream_pad_to_size(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey);
int mlxd_stream_pad_to_size_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey);

/**
 * Per-dimension target sizes: `dim` of `ikey`'s array is padded to
 * sizes[dim] elements.
 */
int mlxd_buffer_pad_to_sizes(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    const int64_t* sizes,
    size_t sizes_num,
    double value,
    const char* okey);
int mlxd_buffer_pad_to_sizes_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    const int64_t* sizes,
    size_t sizes_num,
    double value,
    const char* okey);
int mlxd_stream_pad_to_sizes(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    const int64_t* sizes,
    size_t sizes_num,
    double value,
    const char* okey);
int mlxd_stream_pad_to_sizes_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    const int64_t* sizes,
    size_t sizes_num,
    double value,
    const char* okey);

/* ---------------- slicing ---------------- */

int mlxd_buffer_random_slice(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    int64_t size,
    const char* okey);
int mlxd_buffer_random_slice_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t size,
    const char* okey);
int mlxd_stream_random_slice(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    int64_t size,
    const char* okey);
int mlxd_stream_random_slice_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t size,
    const char* okey);

int mlxd_buffer_random_slice_dims(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const int64_t* sizes,
    size_t sizes_num,
    const char* okey);
int mlxd_buffer_random_slice_dims_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const int64_t* sizes,
    size_t sizes_num,
    const char* okey);
int mlxd_stream_random_slice_dims(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const int64_t* sizes,
    size_t sizes_num,
    const char* okey);
int mlxd_stream_random_slice_dims_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const int64_t* sizes,
    size_t sizes_num,
    const char* okey);

int mlxd_buffer_slice(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    int64_t start,
    int64_t end,
    const char* okey);
int mlxd_buffer_slice_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t start,
    int64_t end,
    const char* okey);
int mlxd_stream_slice(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    int64_t start,
    int64_t end,
    const char* okey);
int mlxd_stream_slice_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t start,
    int64_t end,
    const char* okey);

int mlxd_buffer_slice_dims(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const int64_t* starts,
    size_t starts_num,
    const int64_t* ends,
    size_t ends_num,
    const char* okey);
int mlxd_buffer_slice_dims_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const int64_t* starts,
    size_t starts_num,
    const int64_t* ends,
    size_t ends_num,
    const char* okey);
int mlxd_stream_slice_dims(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const int64_t* starts,
    size_t starts_num,
    const int64_t* ends,
    size_t ends_num,
    const char* okey);
int mlxd_stream_slice_dims_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const int64_t* starts,
    size_t starts_num,
    const int64_t* ends,
    size_t ends_num,
    const char* okey);

/* ---------------- archives ---------------- */

/**
 * Reads `ikey` of each sample as a file contained in the tar named by
 * `tarkey` (from_key: file name; otherwise file content; nested: follow
 * nested tars). The `_with_fetcher` variants arrive with the file
 * fetcher milestone.
 */
int mlxd_buffer_read_from_tar(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* tarkey,
    const char* ikey,
    const char* okey,
    const char* prefix,
    const char* tar_prefix,
    bool from_key,
    bool nested,
    int num_threads);
int mlxd_buffer_read_from_tar_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* tarkey,
    const char* ikey,
    const char* okey,
    const char* prefix,
    const char* tar_prefix,
    bool from_key,
    bool nested,
    int num_threads);
int mlxd_stream_read_from_tar(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* tarkey,
    const char* ikey,
    const char* okey,
    const char* prefix,
    const char* tar_prefix,
    bool from_key,
    bool nested,
    int num_threads);
int mlxd_stream_read_from_tar_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* tarkey,
    const char* ikey,
    const char* okey,
    const char* prefix,
    const char* tar_prefix,
    bool from_key,
    bool nested,
    int num_threads);

/* ---------------- fetcher-backed variants ---------------- */

/**
 * The _with_fetcher variants of the readers and read_from_tar fetch
 * remote files through `fetcher` before local processing. An EMPTY
 * fetcher handle returns status 1 with a clear message — pass NULL
 * semantics by using the fetcher-less originals instead.
 *
 * csv/line readers: same contracts as their originals.
 */

int mlxd_stream_csv_reader_with_fetcher(
    mlxd_stream* out,
    const char* filename,
    char sep,
    char quote,
    const char* local_prefix,
    mlxd_file_fetcher fetcher);

int mlxd_stream_line_reader_with_fetcher(
    mlxd_stream* out,
    const char* filename,
    const char* key,
    bool unzip,
    const char* local_prefix,
    mlxd_file_fetcher fetcher);

int mlxd_stream_csv_reader_from_key_with_fetcher(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* key,
    char sep,
    char quote,
    bool from_memory,
    const char* local_prefix,
    mlxd_file_fetcher fetcher);

int mlxd_stream_line_reader_from_key_with_fetcher(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* key,
    const char* dst_key,
    bool from_memory,
    bool unzip,
    const char* local_prefix,
    mlxd_file_fetcher fetcher);

int mlxd_buffer_read_from_tar_with_fetcher(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* tarkey,
    const char* ikey,
    const char* okey,
    const char* prefix,
    const char* tar_prefix,
    bool from_key,
    mlxd_file_fetcher fetcher,
    bool nested,
    int num_threads);

int mlxd_stream_read_from_tar_with_fetcher(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* tarkey,
    const char* ikey,
    const char* okey,
    const char* prefix,
    const char* tar_prefix,
    bool from_key,
    mlxd_file_fetcher fetcher,
    bool nested,
    int num_threads);

/* ---------------- key/value manipulation ---------------- */

int mlxd_buffer_remove_value(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* key,
    const char* size_key,
    int dim,
    double value,
    double pad);
int mlxd_buffer_remove_value_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* key,
    const char* size_key,
    int dim,
    double value,
    double pad);
int mlxd_stream_remove_value(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* key,
    const char* size_key,
    int dim,
    double value,
    double pad);
int mlxd_stream_remove_value_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* key,
    const char* size_key,
    int dim,
    double value,
    double pad);

int mlxd_buffer_replace(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* key,
    const char* old,
    const char* replacement,
    int count);
int mlxd_buffer_replace_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* key,
    const char* old,
    const char* replacement,
    int count);
int mlxd_stream_replace(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* key,
    const char* old,
    const char* replacement,
    int count);
int mlxd_stream_replace_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* key,
    const char* old,
    const char* replacement,
    int count);

/**
 * `byte_map` is a 256-entry lookup table: byte value i is replaced by
 * byte_map[i] (empty string drops the byte). Entries beyond
 * `byte_map_num` are treated as empty.
 */
int mlxd_buffer_replace_bytes(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char** byte_map,
    size_t byte_map_num,
    const char* okey);
int mlxd_buffer_replace_bytes_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char** byte_map,
    size_t byte_map_num,
    const char* okey);
int mlxd_stream_replace_bytes(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char** byte_map,
    size_t byte_map_num,
    const char* okey);
int mlxd_stream_replace_bytes_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char** byte_map,
    size_t byte_map_num,
    const char* okey);

int mlxd_buffer_rename_key(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* okey);
int mlxd_buffer_rename_key_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* okey);
int mlxd_stream_rename_key(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* okey);
int mlxd_stream_rename_key_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* okey);

int mlxd_buffer_save_image(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* image_key,
    const char* filename_key,
    const char* prefix,
    const char* filename_prefix);
int mlxd_buffer_save_image_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* image_key,
    const char* filename_key,
    const char* prefix,
    const char* filename_prefix);
int mlxd_stream_save_image(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* image_key,
    const char* filename_key,
    const char* prefix,
    const char* filename_prefix);
int mlxd_stream_save_image_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* image_key,
    const char* filename_key,
    const char* prefix,
    const char* filename_prefix);

int mlxd_buffer_shape(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* okey);
int mlxd_buffer_shape_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* okey);
int mlxd_stream_shape(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* okey);
int mlxd_stream_shape_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* okey);

/**
 * Writes only dimension `dim` of `ikey`'s shape to `okey`.
 */
int mlxd_buffer_shape_dim(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    const char* okey);
int mlxd_buffer_shape_dim_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    const char* okey);
int mlxd_stream_shape_dim(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    const char* okey);
int mlxd_stream_shape_dim_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    const char* okey);

int mlxd_buffer_shard(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int64_t n_shards,
    const char* okey);
int mlxd_buffer_shard_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int64_t n_shards,
    const char* okey);
int mlxd_stream_shard(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int64_t n_shards,
    const char* okey);
int mlxd_stream_shard_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int64_t n_shards,
    const char* okey);

int mlxd_buffer_squeeze(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* okey);
int mlxd_buffer_squeeze_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* okey);
int mlxd_stream_squeeze(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* okey);
int mlxd_stream_squeeze_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* okey);

int mlxd_buffer_squeeze_dim(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    const char* okey);
int mlxd_buffer_squeeze_dim_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    const char* okey);
int mlxd_stream_squeeze_dim(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    const char* okey);
int mlxd_stream_squeeze_dim_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    const char* okey);

int mlxd_buffer_squeeze_dims(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const char* okey);
int mlxd_buffer_squeeze_dims_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const char* okey);
int mlxd_stream_squeeze_dims(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const char* okey);
int mlxd_stream_squeeze_dims_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const char* okey);

/* ---------------- user callbacks ---------------- */

/**
 * Applies the closure to `ikey`'s array of every sample. The callback
 * contract and lifetime rules are documented in closure.h; a callback
 * returning an empty handle makes the data access fail with status 1.
 */
int mlxd_buffer_key_transform(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    mlxd_closure_array closure,
    const char* okey);
int mlxd_buffer_key_transform_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    mlxd_closure_array closure,
    const char* okey);
int mlxd_stream_key_transform(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    mlxd_closure_array closure,
    const char* okey);
int mlxd_stream_key_transform_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    mlxd_closure_array closure,
    const char* okey);

/**
 * Applies the closure to every sample (adding, removing, or replacing
 * keys).
 */
int mlxd_buffer_sample_transform(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    mlxd_closure_sample closure);
int mlxd_buffer_sample_transform_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    mlxd_closure_sample closure);
int mlxd_stream_sample_transform(
    mlxd_stream* out,
    mlxd_stream dataset,
    mlxd_closure_sample closure);
int mlxd_stream_sample_transform_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    mlxd_closure_sample closure);

/**
 * Background loading with a user refill hook: keeps up to `buffer_size`
 * samples in flight, calling the closure to refill. The callback may
 * run on worker threads (see closure.h for the thread-safety note).
 */
int mlxd_stream_buffered(
    mlxd_stream* out,
    mlxd_stream dataset,
    int64_t buffer_size,
    mlxd_closure_buffer on_refill,
    int num_thread);

/* ---------------- tokenization ---------------- */

int mlxd_buffer_tokenize(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    mlxd_trie trie,
    mlxd_tokenize_mode mode,
    bool ignore_unk,
    const double* trie_key_scores,
    size_t trie_key_scores_num,
    const char* okey);
int mlxd_buffer_tokenize_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    mlxd_trie trie,
    mlxd_tokenize_mode mode,
    bool ignore_unk,
    const double* trie_key_scores,
    size_t trie_key_scores_num,
    const char* okey);
int mlxd_stream_tokenize(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    mlxd_trie trie,
    mlxd_tokenize_mode mode,
    bool ignore_unk,
    const double* trie_key_scores,
    size_t trie_key_scores_num,
    const char* okey);
int mlxd_stream_tokenize_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    mlxd_trie trie,
    mlxd_tokenize_mode mode,
    bool ignore_unk,
    const double* trie_key_scores,
    size_t trie_key_scores_num,
    const char* okey);

int mlxd_buffer_tokenize_bpe(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    mlxd_trie symbols,
    mlxd_bpe_merges merges,
    const char* okey);
int mlxd_buffer_tokenize_bpe_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    mlxd_trie symbols,
    mlxd_bpe_merges merges,
    const char* okey);
int mlxd_stream_tokenize_bpe(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    mlxd_trie symbols,
    mlxd_bpe_merges merges,
    const char* okey);
int mlxd_stream_tokenize_bpe_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    mlxd_trie symbols,
    mlxd_bpe_merges merges,
    const char* okey);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
