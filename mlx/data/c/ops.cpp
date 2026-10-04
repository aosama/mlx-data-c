#include "mlx/data/c/ops.h"

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "mlx/data/Dataset.h"
#include "mlx/data/c/error.h"
#include "mlx/data/c/private/bpe.h"
#include "mlx/data/c/private/buffer.h"
#include "mlx/data/c/private/closure.h"
#include "mlx/data/c/private/stream.h"
#include "mlx/data/c/private/trie.h"

// The C enums must stay 1:1 with the C++ enums; verified at compile time.
static_assert(
    (int)MLXD_TOKENIZE_SHORTEST ==
    (int)mlx::data::op::TokenizeMode::shortest);
static_assert((int)MLXD_TOKENIZE_RAND == (int)mlx::data::op::TokenizeMode::rand);
static_assert(
    (int)MLXD_LOAD_AUDIO_INFO_ALL ==
    (int)mlx::data::op::LoadAudioInfo::All);
static_assert(
    (int)MLXD_LOAD_AUDIO_INFO_NUM_FRAMES ==
    (int)mlx::data::op::LoadAudioInfo::NumFrames);
static_assert(
    (int)MLXD_LOAD_AUDIO_INFO_NUM_CHANNELS ==
    (int)mlx::data::op::LoadAudioInfo::NumChannels);
static_assert(
    (int)MLXD_LOAD_AUDIO_INFO_SAMPLE_RATE ==
    (int)mlx::data::op::LoadAudioInfo::SampleRate);
static_assert(
    (int)MLXD_LOAD_AUDIO_INFO_NUM_SECONDS ==
    (int)mlx::data::op::LoadAudioInfo::NumSeconds);
static_assert(
    (int)MLXD_RESAMPLING_SINC_BEST ==
    (int)mlx::data::op::LoadAudioResamplingQuality::SincBest);
static_assert(
    (int)MLXD_RESAMPLING_SINC_MEDIUM ==
    (int)mlx::data::op::LoadAudioResamplingQuality::SincMedium);
static_assert(
    (int)MLXD_RESAMPLING_SINC_FASTEST ==
    (int)mlx::data::op::LoadAudioResamplingQuality::SincFastest);
static_assert(
    (int)MLXD_RESAMPLING_ZERO_ORDER_HOLD ==
    (int)mlx::data::op::LoadAudioResamplingQuality::ZeroOrderHold);
static_assert(
    (int)MLXD_RESAMPLING_LINEAR ==
    (int)mlx::data::op::LoadAudioResamplingQuality::Linear);

namespace {

// Required string parameter: NULL is a caller bug, fail loudly.
std::string mlxd_key_(const char* key, const char* caller) {
  if (key == nullptr) {
    throw std::invalid_argument(std::string(caller) + ": key is NULL");
  }
  return std::string(key);
}

// Optional string parameter: NULL means the upstream default value.
std::string mlxd_opt_(const char* value, const char* fallback) {
  return std::string(value ? value : fallback);
}

// Optional path parameter: NULL means the upstream default ("").
std::filesystem::path mlxd_path_(const char* path) {
  return std::filesystem::path(path ? path : "");
}

template <typename F>
int mlxd_dataset_op_(mlxd_buffer* out, mlxd_buffer dataset, F fn) {
  try {
    mlxd_buffer_set_(*out, fn(mlxd_buffer_get_(dataset)));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

template <typename F>
int mlxd_dataset_op_(mlxd_stream* out, mlxd_stream dataset, F fn) {
  try {
    mlxd_stream_set_(*out, fn(mlxd_stream_get_(dataset)));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

} // namespace

/* ---------------- filtering ---------------- */

extern "C" int mlxd_buffer_filter_by_shape(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* key,
    int dim,
    int64_t low,
    int64_t high) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.filter_by_shape(mlxd_key_(key, "filter_by_shape"), dim, low, high);
  });
}

extern "C" int mlxd_buffer_filter_by_shape_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* key,
    int dim,
    int64_t low,
    int64_t high) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.filter_by_shape_if(
        cond, mlxd_key_(key, "filter_by_shape"), dim, low, high);
  });
}

extern "C" int mlxd_stream_filter_by_shape(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* key,
    int dim,
    int64_t low,
    int64_t high) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.filter_by_shape(mlxd_key_(key, "filter_by_shape"), dim, low, high);
  });
}

extern "C" int mlxd_stream_filter_by_shape_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* key,
    int dim,
    int64_t low,
    int64_t high) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.filter_by_shape_if(
        cond, mlxd_key_(key, "filter_by_shape"), dim, low, high);
  });
}

extern "C" int mlxd_buffer_filter_key(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* key,
    bool remove) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.filter_key(mlxd_key_(key, "filter_key"), remove);
  });
}

extern "C" int mlxd_buffer_filter_key_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* key,
    bool remove) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.filter_key_if(cond, mlxd_key_(key, "filter_key"), remove);
  });
}

extern "C" int mlxd_stream_filter_key(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* key,
    bool remove) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.filter_key(mlxd_key_(key, "filter_key"), remove);
  });
}

extern "C" int mlxd_stream_filter_key_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* key,
    bool remove) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.filter_key_if(cond, mlxd_key_(key, "filter_key"), remove);
  });
}

/* ---------------- image transforms ---------------- */

extern "C" int mlxd_buffer_image_center_crop(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_center_crop(
        mlxd_key_(ikey, "image_center_crop"), w, h,
        mlxd_key_(okey, "image_center_crop"));
  });
}

extern "C" int mlxd_buffer_image_center_crop_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_center_crop_if(
        cond, mlxd_key_(ikey, "image_center_crop"), w, h,
        mlxd_key_(okey, "image_center_crop"));
  });
}

extern "C" int mlxd_stream_image_center_crop(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_center_crop(
        mlxd_key_(ikey, "image_center_crop"), w, h,
        mlxd_key_(okey, "image_center_crop"));
  });
}

extern "C" int mlxd_stream_image_center_crop_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_center_crop_if(
        cond, mlxd_key_(ikey, "image_center_crop"), w, h,
        mlxd_key_(okey, "image_center_crop"));
  });
}

extern "C" int mlxd_buffer_image_channel_reduction(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* preset,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_channel_reduction(
        mlxd_key_(ikey, "image_channel_reduction"),
        mlxd_opt_(preset, "default"),
        mlxd_key_(okey, "image_channel_reduction"));
  });
}

extern "C" int mlxd_buffer_image_channel_reduction_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* preset,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_channel_reduction_if(
        cond, mlxd_key_(ikey, "image_channel_reduction"),
        mlxd_opt_(preset, "default"),
        mlxd_key_(okey, "image_channel_reduction"));
  });
}

extern "C" int mlxd_stream_image_channel_reduction(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* preset,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_channel_reduction(
        mlxd_key_(ikey, "image_channel_reduction"),
        mlxd_opt_(preset, "default"),
        mlxd_key_(okey, "image_channel_reduction"));
  });
}

extern "C" int mlxd_stream_image_channel_reduction_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* preset,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_channel_reduction_if(
        cond, mlxd_key_(ikey, "image_channel_reduction"),
        mlxd_opt_(preset, "default"),
        mlxd_key_(okey, "image_channel_reduction"));
  });
}

extern "C" int mlxd_buffer_image_random_area_crop(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    float area_low,
    float area_high,
    float aspect_low,
    float aspect_high,
    int num_trial,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_random_area_crop(
        mlxd_key_(ikey, "image_random_area_crop"),
        std::make_pair(area_low, area_high),
        std::make_pair(aspect_low, aspect_high), num_trial,
        mlxd_key_(okey, "image_random_area_crop"));
  });
}

extern "C" int mlxd_buffer_image_random_area_crop_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    float area_low,
    float area_high,
    float aspect_low,
    float aspect_high,
    int num_trial,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_random_area_crop_if(
        cond, mlxd_key_(ikey, "image_random_area_crop"),
        std::make_pair(area_low, area_high),
        std::make_pair(aspect_low, aspect_high), num_trial,
        mlxd_key_(okey, "image_random_area_crop"));
  });
}

extern "C" int mlxd_stream_image_random_area_crop(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    float area_low,
    float area_high,
    float aspect_low,
    float aspect_high,
    int num_trial,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_random_area_crop(
        mlxd_key_(ikey, "image_random_area_crop"),
        std::make_pair(area_low, area_high),
        std::make_pair(aspect_low, aspect_high), num_trial,
        mlxd_key_(okey, "image_random_area_crop"));
  });
}

extern "C" int mlxd_stream_image_random_area_crop_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    float area_low,
    float area_high,
    float aspect_low,
    float aspect_high,
    int num_trial,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_random_area_crop_if(
        cond, mlxd_key_(ikey, "image_random_area_crop"),
        std::make_pair(area_low, area_high),
        std::make_pair(aspect_low, aspect_high), num_trial,
        mlxd_key_(okey, "image_random_area_crop"));
  });
}

extern "C" int mlxd_buffer_image_random_crop(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_random_crop(
        mlxd_key_(ikey, "image_random_crop"), w, h,
        mlxd_key_(okey, "image_random_crop"));
  });
}

extern "C" int mlxd_buffer_image_random_crop_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_random_crop_if(
        cond, mlxd_key_(ikey, "image_random_crop"), w, h,
        mlxd_key_(okey, "image_random_crop"));
  });
}

extern "C" int mlxd_stream_image_random_crop(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_random_crop(
        mlxd_key_(ikey, "image_random_crop"), w, h,
        mlxd_key_(okey, "image_random_crop"));
  });
}

extern "C" int mlxd_stream_image_random_crop_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_random_crop_if(
        cond, mlxd_key_(ikey, "image_random_crop"), w, h,
        mlxd_key_(okey, "image_random_crop"));
  });
}

extern "C" int mlxd_buffer_image_random_h_flip(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    float prob,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_random_h_flip(
        mlxd_key_(ikey, "image_random_h_flip"), prob,
        mlxd_key_(okey, "image_random_h_flip"));
  });
}

extern "C" int mlxd_buffer_image_random_h_flip_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    float prob,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_random_h_flip_if(
        cond, mlxd_key_(ikey, "image_random_h_flip"), prob,
        mlxd_key_(okey, "image_random_h_flip"));
  });
}

extern "C" int mlxd_stream_image_random_h_flip(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    float prob,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_random_h_flip(
        mlxd_key_(ikey, "image_random_h_flip"), prob,
        mlxd_key_(okey, "image_random_h_flip"));
  });
}

extern "C" int mlxd_stream_image_random_h_flip_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    float prob,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_random_h_flip_if(
        cond, mlxd_key_(ikey, "image_random_h_flip"), prob,
        mlxd_key_(okey, "image_random_h_flip"));
  });
}

extern "C" int mlxd_buffer_image_resize(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_resize(
        mlxd_key_(ikey, "image_resize"), w, h, mlxd_key_(okey, "image_resize"));
  });
}

extern "C" int mlxd_buffer_image_resize_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_resize_if(
        cond, mlxd_key_(ikey, "image_resize"), w, h,
        mlxd_key_(okey, "image_resize"));
  });
}

extern "C" int mlxd_stream_image_resize(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_resize(
        mlxd_key_(ikey, "image_resize"), w, h, mlxd_key_(okey, "image_resize"));
  });
}

extern "C" int mlxd_stream_image_resize_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int64_t w,
    int64_t h,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_resize_if(
        cond, mlxd_key_(ikey, "image_resize"), w, h,
        mlxd_key_(okey, "image_resize"));
  });
}

extern "C" int mlxd_buffer_image_resize_smallest_side(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int64_t size,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_resize_smallest_side(
        mlxd_key_(ikey, "image_resize_smallest_side"), size,
        mlxd_key_(okey, "image_resize_smallest_side"));
  });
}

extern "C" int mlxd_buffer_image_resize_smallest_side_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int64_t size,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_resize_smallest_side_if(
        cond, mlxd_key_(ikey, "image_resize_smallest_side"), size,
        mlxd_key_(okey, "image_resize_smallest_side"));
  });
}

extern "C" int mlxd_stream_image_resize_smallest_side(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int64_t size,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_resize_smallest_side(
        mlxd_key_(ikey, "image_resize_smallest_side"), size,
        mlxd_key_(okey, "image_resize_smallest_side"));
  });
}

extern "C" int mlxd_stream_image_resize_smallest_side_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int64_t size,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_resize_smallest_side_if(
        cond, mlxd_key_(ikey, "image_resize_smallest_side"), size,
        mlxd_key_(okey, "image_resize_smallest_side"));
  });
}

extern "C" int mlxd_buffer_image_rotate(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    double angle,
    bool crop,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_rotate(
        mlxd_key_(ikey, "image_rotate"), angle, crop,
        mlxd_key_(okey, "image_rotate"));
  });
}

extern "C" int mlxd_buffer_image_rotate_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    double angle,
    bool crop,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_rotate_if(
        cond, mlxd_key_(ikey, "image_rotate"), angle, crop,
        mlxd_key_(okey, "image_rotate"));
  });
}

extern "C" int mlxd_stream_image_rotate(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    double angle,
    bool crop,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_rotate(
        mlxd_key_(ikey, "image_rotate"), angle, crop,
        mlxd_key_(okey, "image_rotate"));
  });
}

extern "C" int mlxd_stream_image_rotate_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    double angle,
    bool crop,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.image_rotate_if(
        cond, mlxd_key_(ikey, "image_rotate"), angle, crop,
        mlxd_key_(okey, "image_rotate"));
  });
}

/* ---------------- media loading ---------------- */

extern "C" int mlxd_buffer_load_audio(
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
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_audio(
        mlxd_key_(ikey, "load_audio"), mlxd_path_(prefix), info, from_memory,
        static_cast<mlx::data::op::LoadAudioInfo>(info_type), sample_rate,
        static_cast<mlx::data::op::LoadAudioResamplingQuality>(
            resampling_quality),
        mlxd_opt_(info_key, ""),
        mlxd_key_(okey, "load_audio"));
  });
}

extern "C" int mlxd_buffer_load_audio_if(
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
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_audio_if(
        cond, mlxd_key_(ikey, "load_audio"), mlxd_path_(prefix), info,
        from_memory,
        static_cast<mlx::data::op::LoadAudioInfo>(info_type), sample_rate,
        static_cast<mlx::data::op::LoadAudioResamplingQuality>(
            resampling_quality),
        mlxd_opt_(info_key, ""),
        mlxd_key_(okey, "load_audio"));
  });
}

extern "C" int mlxd_stream_load_audio(
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
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_audio(
        mlxd_key_(ikey, "load_audio"), mlxd_path_(prefix), info, from_memory,
        static_cast<mlx::data::op::LoadAudioInfo>(info_type), sample_rate,
        static_cast<mlx::data::op::LoadAudioResamplingQuality>(
            resampling_quality),
        mlxd_opt_(info_key, ""),
        mlxd_key_(okey, "load_audio"));
  });
}

extern "C" int mlxd_stream_load_audio_if(
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
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_audio_if(
        cond, mlxd_key_(ikey, "load_audio"), mlxd_path_(prefix), info,
        from_memory,
        static_cast<mlx::data::op::LoadAudioInfo>(info_type), sample_rate,
        static_cast<mlx::data::op::LoadAudioResamplingQuality>(
            resampling_quality),
        mlxd_opt_(info_key, ""),
        mlxd_key_(okey, "load_audio"));
  });
}

extern "C" int mlxd_buffer_resample_audio(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int output_sample_rate,
    int input_sample_rate,
    const char* info_key,
    mlxd_load_audio_resampling_quality resampling_quality,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.resample_audio(
        mlxd_key_(ikey, "resample_audio"), output_sample_rate,
        input_sample_rate,
        mlxd_opt_(info_key, ""),
        static_cast<mlx::data::op::LoadAudioResamplingQuality>(
            resampling_quality),
        mlxd_key_(okey, "resample_audio"));
  });
}

extern "C" int mlxd_buffer_resample_audio_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int output_sample_rate,
    int input_sample_rate,
    const char* info_key,
    mlxd_load_audio_resampling_quality resampling_quality,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.resample_audio_if(
        cond, mlxd_key_(ikey, "resample_audio"), output_sample_rate,
        input_sample_rate,
        mlxd_opt_(info_key, ""),
        static_cast<mlx::data::op::LoadAudioResamplingQuality>(
            resampling_quality),
        mlxd_key_(okey, "resample_audio"));
  });
}

extern "C" int mlxd_stream_resample_audio(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int output_sample_rate,
    int input_sample_rate,
    const char* info_key,
    mlxd_load_audio_resampling_quality resampling_quality,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.resample_audio(
        mlxd_key_(ikey, "resample_audio"), output_sample_rate,
        input_sample_rate,
        mlxd_opt_(info_key, ""),
        static_cast<mlx::data::op::LoadAudioResamplingQuality>(
            resampling_quality),
        mlxd_key_(okey, "resample_audio"));
  });
}

extern "C" int mlxd_stream_resample_audio_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int output_sample_rate,
    int input_sample_rate,
    const char* info_key,
    mlxd_load_audio_resampling_quality resampling_quality,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.resample_audio_if(
        cond, mlxd_key_(ikey, "resample_audio"), output_sample_rate,
        input_sample_rate,
        mlxd_opt_(info_key, ""),
        static_cast<mlx::data::op::LoadAudioResamplingQuality>(
            resampling_quality),
        mlxd_key_(okey, "resample_audio"));
  });
}

extern "C" int mlxd_buffer_load_file(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* prefix,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_file(
        mlxd_key_(ikey, "load_file"), mlxd_path_(prefix),
        mlxd_key_(okey, "load_file"));
  });
}

extern "C" int mlxd_buffer_load_file_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_file_if(
        cond, mlxd_key_(ikey, "load_file"), mlxd_path_(prefix),
        mlxd_key_(okey, "load_file"));
  });
}

extern "C" int mlxd_stream_load_file(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* prefix,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_file(
        mlxd_key_(ikey, "load_file"), mlxd_path_(prefix),
        mlxd_key_(okey, "load_file"));
  });
}

extern "C" int mlxd_stream_load_file_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_file_if(
        cond, mlxd_key_(ikey, "load_file"), mlxd_path_(prefix),
        mlxd_key_(okey, "load_file"));
  });
}

extern "C" int mlxd_buffer_load_image(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* prefix,
    bool info,
    const char* format,
    bool from_memory,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_image(
        mlxd_key_(ikey, "load_image"), mlxd_path_(prefix), info,
        mlxd_opt_(format, "RGB"), from_memory,
        mlxd_key_(okey, "load_image"));
  });
}

extern "C" int mlxd_buffer_load_image_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    bool info,
    const char* format,
    bool from_memory,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_image_if(
        cond, mlxd_key_(ikey, "load_image"), mlxd_path_(prefix), info,
        mlxd_opt_(format, "RGB"), from_memory,
        mlxd_key_(okey, "load_image"));
  });
}

extern "C" int mlxd_stream_load_image(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* prefix,
    bool info,
    const char* format,
    bool from_memory,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_image(
        mlxd_key_(ikey, "load_image"), mlxd_path_(prefix), info,
        mlxd_opt_(format, "RGB"), from_memory,
        mlxd_key_(okey, "load_image"));
  });
}

extern "C" int mlxd_stream_load_image_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    bool info,
    const char* format,
    bool from_memory,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_image_if(
        cond, mlxd_key_(ikey, "load_image"), mlxd_path_(prefix), info,
        mlxd_opt_(format, "RGB"), from_memory,
        mlxd_key_(okey, "load_image"));
  });
}

extern "C" int mlxd_buffer_load_numpy(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* prefix,
    bool from_memory,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_numpy(
        mlxd_key_(ikey, "load_numpy"), mlxd_path_(prefix), from_memory,
        mlxd_key_(okey, "load_numpy"));
  });
}

extern "C" int mlxd_buffer_load_numpy_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    bool from_memory,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_numpy_if(
        cond, mlxd_key_(ikey, "load_numpy"), mlxd_path_(prefix), from_memory,
        mlxd_key_(okey, "load_numpy"));
  });
}

extern "C" int mlxd_stream_load_numpy(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* prefix,
    bool from_memory,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_numpy(
        mlxd_key_(ikey, "load_numpy"), mlxd_path_(prefix), from_memory,
        mlxd_key_(okey, "load_numpy"));
  });
}

extern "C" int mlxd_stream_load_numpy_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    bool from_memory,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_numpy_if(
        cond, mlxd_key_(ikey, "load_numpy"), mlxd_path_(prefix), from_memory,
        mlxd_key_(okey, "load_numpy"));
  });
}

extern "C" int mlxd_buffer_load_video(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* prefix,
    bool info,
    bool from_memory,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_video(
        mlxd_key_(ikey, "load_video"), mlxd_path_(prefix), info, from_memory,
        mlxd_key_(okey, "load_video"));
  });
}

extern "C" int mlxd_buffer_load_video_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    bool info,
    bool from_memory,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_video_if(
        cond, mlxd_key_(ikey, "load_video"), mlxd_path_(prefix), info,
        from_memory, mlxd_key_(okey, "load_video"));
  });
}

extern "C" int mlxd_stream_load_video(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* prefix,
    bool info,
    bool from_memory,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_video(
        mlxd_key_(ikey, "load_video"), mlxd_path_(prefix), info, from_memory,
        mlxd_key_(okey, "load_video"));
  });
}

extern "C" int mlxd_stream_load_video_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* prefix,
    bool info,
    bool from_memory,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.load_video_if(
        cond, mlxd_key_(ikey, "load_video"), mlxd_path_(prefix), info,
        from_memory, mlxd_key_(okey, "load_video"));
  });
}

/* ---------------- padding ---------------- */

extern "C" int mlxd_buffer_pad(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    int64_t lpad,
    int64_t rpad,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.pad(
        mlxd_key_(ikey, "pad"), dim, lpad, rpad, value,
        mlxd_key_(okey, "pad"));
  });
}

extern "C" int mlxd_buffer_pad_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t lpad,
    int64_t rpad,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.pad_if(
        cond, mlxd_key_(ikey, "pad"), dim, lpad, rpad, value,
        mlxd_key_(okey, "pad"));
  });
}

extern "C" int mlxd_stream_pad(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    int64_t lpad,
    int64_t rpad,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.pad(
        mlxd_key_(ikey, "pad"), dim, lpad, rpad, value,
        mlxd_key_(okey, "pad"));
  });
}

extern "C" int mlxd_stream_pad_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t lpad,
    int64_t rpad,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.pad_if(
        cond, mlxd_key_(ikey, "pad"), dim, lpad, rpad, value,
        mlxd_key_(okey, "pad"));
  });
}

extern "C" int mlxd_buffer_pad_to_multiple(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.pad_to_multiple(
        mlxd_key_(ikey, "pad_to_multiple"), dim, size, value,
        mlxd_key_(okey, "pad_to_multiple"));
  });
}

extern "C" int mlxd_buffer_pad_to_multiple_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.pad_to_multiple_if(
        cond, mlxd_key_(ikey, "pad_to_multiple"), dim, size, value,
        mlxd_key_(okey, "pad_to_multiple"));
  });
}

extern "C" int mlxd_stream_pad_to_multiple(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.pad_to_multiple(
        mlxd_key_(ikey, "pad_to_multiple"), dim, size, value,
        mlxd_key_(okey, "pad_to_multiple"));
  });
}

extern "C" int mlxd_stream_pad_to_multiple_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.pad_to_multiple_if(
        cond, mlxd_key_(ikey, "pad_to_multiple"), dim, size, value,
        mlxd_key_(okey, "pad_to_multiple"));
  });
}

extern "C" int mlxd_buffer_pad_to_size(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.pad_to_size(
        mlxd_key_(ikey, "pad_to_size"), dim, size, value,
        mlxd_key_(okey, "pad_to_size"));
  });
}

extern "C" int mlxd_buffer_pad_to_size_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.pad_to_size_if(
        cond, mlxd_key_(ikey, "pad_to_size"), dim, size, value,
        mlxd_key_(okey, "pad_to_size"));
  });
}

extern "C" int mlxd_stream_pad_to_size(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.pad_to_size(
        mlxd_key_(ikey, "pad_to_size"), dim, size, value,
        mlxd_key_(okey, "pad_to_size"));
  });
}

extern "C" int mlxd_stream_pad_to_size_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t size,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.pad_to_size_if(
        cond, mlxd_key_(ikey, "pad_to_size"), dim, size, value,
        mlxd_key_(okey, "pad_to_size"));
  });
}

extern "C" int mlxd_buffer_pad_to_sizes(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    const int64_t* sizes,
    size_t sizes_num,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if (sizes_num > 0 && sizes == nullptr) {
      throw std::invalid_argument("pad_to_sizes: sizes is NULL");
    }
    return ds.pad_to_size(
        mlxd_key_(ikey, "pad_to_sizes"), dim,
        std::vector<int64_t>(sizes, sizes + sizes_num), value,
        mlxd_key_(okey, "pad_to_sizes"));
  });
}

extern "C" int mlxd_buffer_pad_to_sizes_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    const int64_t* sizes,
    size_t sizes_num,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if (sizes_num > 0 && sizes == nullptr) {
      throw std::invalid_argument("pad_to_sizes: sizes is NULL");
    }
    return ds.pad_to_size_if(
        cond, mlxd_key_(ikey, "pad_to_sizes"), dim,
        std::vector<int64_t>(sizes, sizes + sizes_num), value,
        mlxd_key_(okey, "pad_to_sizes"));
  });
}

extern "C" int mlxd_stream_pad_to_sizes(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    const int64_t* sizes,
    size_t sizes_num,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if (sizes_num > 0 && sizes == nullptr) {
      throw std::invalid_argument("pad_to_sizes: sizes is NULL");
    }
    return ds.pad_to_size(
        mlxd_key_(ikey, "pad_to_sizes"), dim,
        std::vector<int64_t>(sizes, sizes + sizes_num), value,
        mlxd_key_(okey, "pad_to_sizes"));
  });
}

extern "C" int mlxd_stream_pad_to_sizes_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    const int64_t* sizes,
    size_t sizes_num,
    double value,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if (sizes_num > 0 && sizes == nullptr) {
      throw std::invalid_argument("pad_to_sizes: sizes is NULL");
    }
    return ds.pad_to_size_if(
        cond, mlxd_key_(ikey, "pad_to_sizes"), dim,
        std::vector<int64_t>(sizes, sizes + sizes_num), value,
        mlxd_key_(okey, "pad_to_sizes"));
  });
}

/* ---------------- slicing ---------------- */

extern "C" int mlxd_buffer_random_slice(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    int64_t size,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.random_slice(
        mlxd_key_(ikey, "random_slice"), dim, size,
        mlxd_key_(okey, "random_slice"));
  });
}

extern "C" int mlxd_buffer_random_slice_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t size,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.random_slice_if(
        cond, mlxd_key_(ikey, "random_slice"), dim, size,
        mlxd_key_(okey, "random_slice"));
  });
}

extern "C" int mlxd_stream_random_slice(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    int64_t size,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.random_slice(
        mlxd_key_(ikey, "random_slice"), dim, size,
        mlxd_key_(okey, "random_slice"));
  });
}

extern "C" int mlxd_stream_random_slice_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t size,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.random_slice_if(
        cond, mlxd_key_(ikey, "random_slice"), dim, size,
        mlxd_key_(okey, "random_slice"));
  });
}

extern "C" int mlxd_buffer_random_slice_dims(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const int64_t* sizes,
    size_t sizes_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if ((dims_num > 0 && dims == nullptr) ||
        (sizes_num > 0 && sizes == nullptr)) {
      throw std::invalid_argument("random_slice_dims: NULL list");
    }
    return ds.random_slice(
        mlxd_key_(ikey, "random_slice_dims"),
        std::vector<int>(dims, dims + dims_num),
        std::vector<int64_t>(sizes, sizes + sizes_num),
        mlxd_key_(okey, "random_slice_dims"));
  });
}

extern "C" int mlxd_buffer_random_slice_dims_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const int64_t* sizes,
    size_t sizes_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if ((dims_num > 0 && dims == nullptr) ||
        (sizes_num > 0 && sizes == nullptr)) {
      throw std::invalid_argument("random_slice_dims: NULL list");
    }
    return ds.random_slice_if(
        cond, mlxd_key_(ikey, "random_slice_dims"),
        std::vector<int>(dims, dims + dims_num),
        std::vector<int64_t>(sizes, sizes + sizes_num),
        mlxd_key_(okey, "random_slice_dims"));
  });
}

extern "C" int mlxd_stream_random_slice_dims(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const int64_t* sizes,
    size_t sizes_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if ((dims_num > 0 && dims == nullptr) ||
        (sizes_num > 0 && sizes == nullptr)) {
      throw std::invalid_argument("random_slice_dims: NULL list");
    }
    return ds.random_slice(
        mlxd_key_(ikey, "random_slice_dims"),
        std::vector<int>(dims, dims + dims_num),
        std::vector<int64_t>(sizes, sizes + sizes_num),
        mlxd_key_(okey, "random_slice_dims"));
  });
}

extern "C" int mlxd_stream_random_slice_dims_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const int64_t* sizes,
    size_t sizes_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if ((dims_num > 0 && dims == nullptr) ||
        (sizes_num > 0 && sizes == nullptr)) {
      throw std::invalid_argument("random_slice_dims: NULL list");
    }
    return ds.random_slice_if(
        cond, mlxd_key_(ikey, "random_slice_dims"),
        std::vector<int>(dims, dims + dims_num),
        std::vector<int64_t>(sizes, sizes + sizes_num),
        mlxd_key_(okey, "random_slice_dims"));
  });
}

extern "C" int mlxd_buffer_slice(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    int64_t start,
    int64_t end,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.slice(
        mlxd_key_(ikey, "slice"), dim, start, end, mlxd_key_(okey, "slice"));
  });
}

extern "C" int mlxd_buffer_slice_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t start,
    int64_t end,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.slice_if(
        cond, mlxd_key_(ikey, "slice"), dim, start, end,
        mlxd_key_(okey, "slice"));
  });
}

extern "C" int mlxd_stream_slice(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    int64_t start,
    int64_t end,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.slice(
        mlxd_key_(ikey, "slice"), dim, start, end, mlxd_key_(okey, "slice"));
  });
}

extern "C" int mlxd_stream_slice_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    int64_t start,
    int64_t end,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.slice_if(
        cond, mlxd_key_(ikey, "slice"), dim, start, end,
        mlxd_key_(okey, "slice"));
  });
}

extern "C" int mlxd_buffer_slice_dims(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const int64_t* starts,
    size_t starts_num,
    const int64_t* ends,
    size_t ends_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if ((dims_num > 0 && dims == nullptr) ||
        (starts_num > 0 && starts == nullptr) ||
        (ends_num > 0 && ends == nullptr)) {
      throw std::invalid_argument("slice_dims: NULL list");
    }
    return ds.slice(
        mlxd_key_(ikey, "slice_dims"),
        std::vector<int>(dims, dims + dims_num),
        std::vector<int64_t>(starts, starts + starts_num),
        std::vector<int64_t>(ends, ends + ends_num),
        mlxd_key_(okey, "slice_dims"));
  });
}

extern "C" int mlxd_buffer_slice_dims_if(
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
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if ((dims_num > 0 && dims == nullptr) ||
        (starts_num > 0 && starts == nullptr) ||
        (ends_num > 0 && ends == nullptr)) {
      throw std::invalid_argument("slice_dims: NULL list");
    }
    return ds.slice_if(
        cond, mlxd_key_(ikey, "slice_dims"),
        std::vector<int>(dims, dims + dims_num),
        std::vector<int64_t>(starts, starts + starts_num),
        std::vector<int64_t>(ends, ends + ends_num),
        mlxd_key_(okey, "slice_dims"));
  });
}

extern "C" int mlxd_stream_slice_dims(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const int64_t* starts,
    size_t starts_num,
    const int64_t* ends,
    size_t ends_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if ((dims_num > 0 && dims == nullptr) ||
        (starts_num > 0 && starts == nullptr) ||
        (ends_num > 0 && ends == nullptr)) {
      throw std::invalid_argument("slice_dims: NULL list");
    }
    return ds.slice(
        mlxd_key_(ikey, "slice_dims"),
        std::vector<int>(dims, dims + dims_num),
        std::vector<int64_t>(starts, starts + starts_num),
        std::vector<int64_t>(ends, ends + ends_num),
        mlxd_key_(okey, "slice_dims"));
  });
}

extern "C" int mlxd_stream_slice_dims_if(
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
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if ((dims_num > 0 && dims == nullptr) ||
        (starts_num > 0 && starts == nullptr) ||
        (ends_num > 0 && ends == nullptr)) {
      throw std::invalid_argument("slice_dims: NULL list");
    }
    return ds.slice_if(
        cond, mlxd_key_(ikey, "slice_dims"),
        std::vector<int>(dims, dims + dims_num),
        std::vector<int64_t>(starts, starts + starts_num),
        std::vector<int64_t>(ends, ends + ends_num),
        mlxd_key_(okey, "slice_dims"));
  });
}

/* ---------------- archives ---------------- */

extern "C" int mlxd_buffer_read_from_tar(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* tarkey,
    const char* ikey,
    const char* okey,
    const char* prefix,
    const char* tar_prefix,
    bool from_key,
    bool nested,
    int num_threads) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.read_from_tar(
        mlxd_key_(tarkey, "read_from_tar"), mlxd_key_(ikey, "read_from_tar"),
        mlxd_key_(okey, "read_from_tar"), mlxd_path_(prefix),
        mlxd_path_(tar_prefix), from_key, nullptr, nested, num_threads);
  });
}

extern "C" int mlxd_buffer_read_from_tar_if(
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
    int num_threads) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.read_from_tar_if(
        cond, mlxd_key_(tarkey, "read_from_tar"),
        mlxd_key_(ikey, "read_from_tar"), mlxd_key_(okey, "read_from_tar"),
        mlxd_path_(prefix), mlxd_path_(tar_prefix), from_key, nullptr, nested,
        num_threads);
  });
}

extern "C" int mlxd_stream_read_from_tar(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* tarkey,
    const char* ikey,
    const char* okey,
    const char* prefix,
    const char* tar_prefix,
    bool from_key,
    bool nested,
    int num_threads) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.read_from_tar(
        mlxd_key_(tarkey, "read_from_tar"), mlxd_key_(ikey, "read_from_tar"),
        mlxd_key_(okey, "read_from_tar"), mlxd_path_(prefix),
        mlxd_path_(tar_prefix), from_key, nullptr, nested, num_threads);
  });
}

extern "C" int mlxd_stream_read_from_tar_if(
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
    int num_threads) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.read_from_tar_if(
        cond, mlxd_key_(tarkey, "read_from_tar"),
        mlxd_key_(ikey, "read_from_tar"), mlxd_key_(okey, "read_from_tar"),
        mlxd_path_(prefix), mlxd_path_(tar_prefix), from_key, nullptr, nested,
        num_threads);
  });
}

/* ---------------- key/value manipulation ---------------- */

extern "C" int mlxd_buffer_remove_value(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* key,
    const char* size_key,
    int dim,
    double value,
    double pad) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.remove_value(
        mlxd_key_(key, "remove_value"), mlxd_key_(size_key, "remove_value"),
        dim, value, pad);
  });
}

extern "C" int mlxd_buffer_remove_value_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* key,
    const char* size_key,
    int dim,
    double value,
    double pad) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.remove_value_if(
        cond, mlxd_key_(key, "remove_value"),
        mlxd_key_(size_key, "remove_value"), dim, value, pad);
  });
}

extern "C" int mlxd_stream_remove_value(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* key,
    const char* size_key,
    int dim,
    double value,
    double pad) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.remove_value(
        mlxd_key_(key, "remove_value"), mlxd_key_(size_key, "remove_value"),
        dim, value, pad);
  });
}

extern "C" int mlxd_stream_remove_value_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* key,
    const char* size_key,
    int dim,
    double value,
    double pad) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.remove_value_if(
        cond, mlxd_key_(key, "remove_value"),
        mlxd_key_(size_key, "remove_value"), dim, value, pad);
  });
}

extern "C" int mlxd_buffer_replace(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* key,
    const char* old,
    const char* replacement,
    int count) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.replace(
        mlxd_key_(key, "replace"), mlxd_key_(old, "replace"),
        mlxd_key_(replacement, "replace"), count);
  });
}

extern "C" int mlxd_buffer_replace_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* key,
    const char* old,
    const char* replacement,
    int count) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.replace_if(
        cond, mlxd_key_(key, "replace"), mlxd_key_(old, "replace"),
        mlxd_key_(replacement, "replace"), count);
  });
}

extern "C" int mlxd_stream_replace(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* key,
    const char* old,
    const char* replacement,
    int count) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.replace(
        mlxd_key_(key, "replace"), mlxd_key_(old, "replace"),
        mlxd_key_(replacement, "replace"), count);
  });
}

extern "C" int mlxd_stream_replace_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* key,
    const char* old,
    const char* replacement,
    int count) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.replace_if(
        cond, mlxd_key_(key, "replace"), mlxd_key_(old, "replace"),
        mlxd_key_(replacement, "replace"), count);
  });
}

extern "C" int mlxd_buffer_replace_bytes(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char** byte_map,
    size_t byte_map_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if (byte_map_num > 0 && byte_map == nullptr) {
      throw std::invalid_argument("replace_bytes: byte_map is NULL");
    }
    return ds.replace_bytes(
        mlxd_key_(ikey, "replace_bytes"),
        std::vector<std::string>(byte_map, byte_map + byte_map_num),
        mlxd_key_(okey, "replace_bytes"));
  });
}

extern "C" int mlxd_buffer_replace_bytes_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char** byte_map,
    size_t byte_map_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if (byte_map_num > 0 && byte_map == nullptr) {
      throw std::invalid_argument("replace_bytes: byte_map is NULL");
    }
    return ds.replace_bytes_if(
        cond, mlxd_key_(ikey, "replace_bytes"),
        std::vector<std::string>(byte_map, byte_map + byte_map_num),
        mlxd_key_(okey, "replace_bytes"));
  });
}

extern "C" int mlxd_stream_replace_bytes(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char** byte_map,
    size_t byte_map_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if (byte_map_num > 0 && byte_map == nullptr) {
      throw std::invalid_argument("replace_bytes: byte_map is NULL");
    }
    return ds.replace_bytes(
        mlxd_key_(ikey, "replace_bytes"),
        std::vector<std::string>(byte_map, byte_map + byte_map_num),
        mlxd_key_(okey, "replace_bytes"));
  });
}

extern "C" int mlxd_stream_replace_bytes_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char** byte_map,
    size_t byte_map_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if (byte_map_num > 0 && byte_map == nullptr) {
      throw std::invalid_argument("replace_bytes: byte_map is NULL");
    }
    return ds.replace_bytes_if(
        cond, mlxd_key_(ikey, "replace_bytes"),
        std::vector<std::string>(byte_map, byte_map + byte_map_num),
        mlxd_key_(okey, "replace_bytes"));
  });
}

extern "C" int mlxd_buffer_rename_key(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.rename_key(
        mlxd_key_(ikey, "rename_key"), mlxd_key_(okey, "rename_key"));
  });
}

extern "C" int mlxd_buffer_rename_key_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.rename_key_if(
        cond, mlxd_key_(ikey, "rename_key"), mlxd_key_(okey, "rename_key"));
  });
}

extern "C" int mlxd_stream_rename_key(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.rename_key(
        mlxd_key_(ikey, "rename_key"), mlxd_key_(okey, "rename_key"));
  });
}

extern "C" int mlxd_stream_rename_key_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.rename_key_if(
        cond, mlxd_key_(ikey, "rename_key"), mlxd_key_(okey, "rename_key"));
  });
}

extern "C" int mlxd_buffer_save_image(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* image_key,
    const char* filename_key,
    const char* prefix,
    const char* filename_prefix) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.save_image(
        mlxd_key_(image_key, "save_image"),
        mlxd_key_(filename_key, "save_image"),
        mlxd_opt_(prefix, ""),
        mlxd_opt_(filename_prefix, ""));
  });
}

extern "C" int mlxd_buffer_save_image_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* image_key,
    const char* filename_key,
    const char* prefix,
    const char* filename_prefix) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.save_image_if(
        cond, mlxd_key_(image_key, "save_image"),
        mlxd_key_(filename_key, "save_image"),
        mlxd_opt_(prefix, ""),
        mlxd_opt_(filename_prefix, ""));
  });
}

extern "C" int mlxd_stream_save_image(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* image_key,
    const char* filename_key,
    const char* prefix,
    const char* filename_prefix) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.save_image(
        mlxd_key_(image_key, "save_image"),
        mlxd_key_(filename_key, "save_image"),
        mlxd_opt_(prefix, ""),
        mlxd_opt_(filename_prefix, ""));
  });
}

extern "C" int mlxd_stream_save_image_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* image_key,
    const char* filename_key,
    const char* prefix,
    const char* filename_prefix) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.save_image_if(
        cond, mlxd_key_(image_key, "save_image"),
        mlxd_key_(filename_key, "save_image"),
        mlxd_opt_(prefix, ""),
        mlxd_opt_(filename_prefix, ""));
  });
}

extern "C" int mlxd_buffer_shape(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.shape(mlxd_key_(ikey, "shape"), mlxd_key_(okey, "shape"));
  });
}

extern "C" int mlxd_buffer_shape_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.shape_if(
        cond, mlxd_key_(ikey, "shape"), mlxd_key_(okey, "shape"));
  });
}

extern "C" int mlxd_stream_shape(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.shape(mlxd_key_(ikey, "shape"), mlxd_key_(okey, "shape"));
  });
}

extern "C" int mlxd_stream_shape_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.shape_if(
        cond, mlxd_key_(ikey, "shape"), mlxd_key_(okey, "shape"));
  });
}

extern "C" int mlxd_buffer_shape_dim(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.shape(
        mlxd_key_(ikey, "shape_dim"), dim, mlxd_key_(okey, "shape_dim"));
  });
}

extern "C" int mlxd_buffer_shape_dim_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.shape_if(
        cond, mlxd_key_(ikey, "shape_dim"), dim, mlxd_key_(okey, "shape_dim"));
  });
}

extern "C" int mlxd_stream_shape_dim(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.shape(
        mlxd_key_(ikey, "shape_dim"), dim, mlxd_key_(okey, "shape_dim"));
  });
}

extern "C" int mlxd_stream_shape_dim_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.shape_if(
        cond, mlxd_key_(ikey, "shape_dim"), dim, mlxd_key_(okey, "shape_dim"));
  });
}

extern "C" int mlxd_buffer_shard(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int64_t n_shards,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.shard(
        mlxd_key_(ikey, "shard"), n_shards, mlxd_key_(okey, "shard"));
  });
}

extern "C" int mlxd_buffer_shard_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int64_t n_shards,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.shard_if(
        cond, mlxd_key_(ikey, "shard"), n_shards, mlxd_key_(okey, "shard"));
  });
}

extern "C" int mlxd_stream_shard(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int64_t n_shards,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.shard(
        mlxd_key_(ikey, "shard"), n_shards, mlxd_key_(okey, "shard"));
  });
}

extern "C" int mlxd_stream_shard_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int64_t n_shards,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.shard_if(
        cond, mlxd_key_(ikey, "shard"), n_shards, mlxd_key_(okey, "shard"));
  });
}

extern "C" int mlxd_buffer_squeeze(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.squeeze(
        mlxd_key_(ikey, "squeeze"), mlxd_key_(okey, "squeeze"));
  });
}

extern "C" int mlxd_buffer_squeeze_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.squeeze_if(
        cond, mlxd_key_(ikey, "squeeze"), mlxd_key_(okey, "squeeze"));
  });
}

extern "C" int mlxd_stream_squeeze(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.squeeze(
        mlxd_key_(ikey, "squeeze"), mlxd_key_(okey, "squeeze"));
  });
}

extern "C" int mlxd_stream_squeeze_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.squeeze_if(
        cond, mlxd_key_(ikey, "squeeze"), mlxd_key_(okey, "squeeze"));
  });
}

extern "C" int mlxd_buffer_squeeze_dim(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    int dim,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.squeeze(
        mlxd_key_(ikey, "squeeze_dim"), dim, mlxd_key_(okey, "squeeze_dim"));
  });
}

extern "C" int mlxd_buffer_squeeze_dim_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    int dim,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.squeeze_if(
        cond, mlxd_key_(ikey, "squeeze_dim"), dim,
        mlxd_key_(okey, "squeeze_dim"));
  });
}

extern "C" int mlxd_stream_squeeze_dim(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    int dim,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.squeeze(
        mlxd_key_(ikey, "squeeze_dim"), dim, mlxd_key_(okey, "squeeze_dim"));
  });
}

extern "C" int mlxd_stream_squeeze_dim_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    int dim,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.squeeze_if(
        cond, mlxd_key_(ikey, "squeeze_dim"), dim,
        mlxd_key_(okey, "squeeze_dim"));
  });
}

extern "C" int mlxd_buffer_squeeze_dims(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if (dims_num > 0 && dims == nullptr) {
      throw std::invalid_argument("squeeze_dims: dims is NULL");
    }
    return ds.squeeze(
        mlxd_key_(ikey, "squeeze_dims"),
        std::vector<int>(dims, dims + dims_num),
        mlxd_key_(okey, "squeeze_dims"));
  });
}

extern "C" int mlxd_buffer_squeeze_dims_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if (dims_num > 0 && dims == nullptr) {
      throw std::invalid_argument("squeeze_dims: dims is NULL");
    }
    return ds.squeeze_if(
        cond, mlxd_key_(ikey, "squeeze_dims"),
        std::vector<int>(dims, dims + dims_num),
        mlxd_key_(okey, "squeeze_dims"));
  });
}

extern "C" int mlxd_stream_squeeze_dims(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if (dims_num > 0 && dims == nullptr) {
      throw std::invalid_argument("squeeze_dims: dims is NULL");
    }
    return ds.squeeze(
        mlxd_key_(ikey, "squeeze_dims"),
        std::vector<int>(dims, dims + dims_num),
        mlxd_key_(okey, "squeeze_dims"));
  });
}

extern "C" int mlxd_stream_squeeze_dims_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    const int* dims,
    size_t dims_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    if (dims_num > 0 && dims == nullptr) {
      throw std::invalid_argument("squeeze_dims: dims is NULL");
    }
    return ds.squeeze_if(
        cond, mlxd_key_(ikey, "squeeze_dims"),
        std::vector<int>(dims, dims + dims_num),
        mlxd_key_(okey, "squeeze_dims"));
  });
}

/* ---------------- user callbacks ---------------- */

extern "C" int mlxd_buffer_key_transform(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    mlxd_closure_array closure,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.key_transform(
        mlxd_key_(ikey, "key_transform"),
        mlxd_key_transform_fn_(mlxd_closure_array_state_(closure)),
        mlxd_key_(okey, "key_transform"));
  });
}

extern "C" int mlxd_buffer_key_transform_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    mlxd_closure_array closure,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.key_transform_if(
        cond, mlxd_key_(ikey, "key_transform"),
        mlxd_key_transform_fn_(mlxd_closure_array_state_(closure)),
        mlxd_key_(okey, "key_transform"));
  });
}

extern "C" int mlxd_stream_key_transform(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    mlxd_closure_array closure,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.key_transform(
        mlxd_key_(ikey, "key_transform"),
        mlxd_key_transform_fn_(mlxd_closure_array_state_(closure)),
        mlxd_key_(okey, "key_transform"));
  });
}

extern "C" int mlxd_stream_key_transform_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    mlxd_closure_array closure,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.key_transform_if(
        cond, mlxd_key_(ikey, "key_transform"),
        mlxd_key_transform_fn_(mlxd_closure_array_state_(closure)),
        mlxd_key_(okey, "key_transform"));
  });
}

extern "C" int mlxd_buffer_sample_transform(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    mlxd_closure_sample closure) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.sample_transform(
        mlxd_sample_transform_fn_(mlxd_closure_sample_state_(closure)));
  });
}

extern "C" int mlxd_buffer_sample_transform_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    mlxd_closure_sample closure) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.sample_transform_if(
        cond,
        mlxd_sample_transform_fn_(mlxd_closure_sample_state_(closure)));
  });
}

extern "C" int mlxd_stream_sample_transform(
    mlxd_stream* out,
    mlxd_stream dataset,
    mlxd_closure_sample closure) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.sample_transform(
        mlxd_sample_transform_fn_(mlxd_closure_sample_state_(closure)));
  });
}

extern "C" int mlxd_stream_sample_transform_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    mlxd_closure_sample closure) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.sample_transform_if(
        cond,
        mlxd_sample_transform_fn_(mlxd_closure_sample_state_(closure)));
  });
}

extern "C" int mlxd_stream_buffered(
    mlxd_stream* out,
    mlxd_stream dataset,
    int64_t buffer_size,
    mlxd_closure_buffer on_refill,
    int num_thread) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.buffered(
        buffer_size,
        mlxd_buffer_transform_fn_(mlxd_closure_buffer_state_(on_refill)),
        num_thread);
  });
}

/* ---------------- tokenization ---------------- */

extern "C" int mlxd_buffer_tokenize(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    mlxd_trie trie,
    mlxd_tokenize_mode mode,
    bool ignore_unk,
    const double* trie_key_scores,
    size_t trie_key_scores_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    std::vector<double> scores;
    if (trie_key_scores != nullptr) {
      scores.assign(trie_key_scores, trie_key_scores + trie_key_scores_num);
    }
    return ds.tokenize(
        mlxd_key_(ikey, "tokenize"), mlxd_trie_get_(trie),
        static_cast<mlx::data::op::TokenizeMode>(mode), ignore_unk, scores,
        mlxd_key_(okey, "tokenize"));
  });
}

extern "C" int mlxd_buffer_tokenize_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    mlxd_trie trie,
    mlxd_tokenize_mode mode,
    bool ignore_unk,
    const double* trie_key_scores,
    size_t trie_key_scores_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    std::vector<double> scores;
    if (trie_key_scores != nullptr) {
      scores.assign(trie_key_scores, trie_key_scores + trie_key_scores_num);
    }
    return ds.tokenize_if(
        cond, mlxd_key_(ikey, "tokenize"), mlxd_trie_get_(trie),
        static_cast<mlx::data::op::TokenizeMode>(mode), ignore_unk, scores,
        mlxd_key_(okey, "tokenize"));
  });
}

extern "C" int mlxd_stream_tokenize(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    mlxd_trie trie,
    mlxd_tokenize_mode mode,
    bool ignore_unk,
    const double* trie_key_scores,
    size_t trie_key_scores_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    std::vector<double> scores;
    if (trie_key_scores != nullptr) {
      scores.assign(trie_key_scores, trie_key_scores + trie_key_scores_num);
    }
    return ds.tokenize(
        mlxd_key_(ikey, "tokenize"), mlxd_trie_get_(trie),
        static_cast<mlx::data::op::TokenizeMode>(mode), ignore_unk, scores,
        mlxd_key_(okey, "tokenize"));
  });
}

extern "C" int mlxd_stream_tokenize_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    mlxd_trie trie,
    mlxd_tokenize_mode mode,
    bool ignore_unk,
    const double* trie_key_scores,
    size_t trie_key_scores_num,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    std::vector<double> scores;
    if (trie_key_scores != nullptr) {
      scores.assign(trie_key_scores, trie_key_scores + trie_key_scores_num);
    }
    return ds.tokenize_if(
        cond, mlxd_key_(ikey, "tokenize"), mlxd_trie_get_(trie),
        static_cast<mlx::data::op::TokenizeMode>(mode), ignore_unk, scores,
        mlxd_key_(okey, "tokenize"));
  });
}

extern "C" int mlxd_buffer_tokenize_bpe(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    const char* ikey,
    mlxd_trie symbols,
    mlxd_bpe_merges merges,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.tokenize_bpe(
        mlxd_key_(ikey, "tokenize_bpe"), mlxd_trie_get_(symbols),
        mlxd_bpe_merges_get_(merges), mlxd_key_(okey, "tokenize_bpe"));
  });
}

extern "C" int mlxd_buffer_tokenize_bpe_if(
    mlxd_buffer* out,
    mlxd_buffer dataset,
    bool cond,
    const char* ikey,
    mlxd_trie symbols,
    mlxd_bpe_merges merges,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.tokenize_bpe_if(
        cond, mlxd_key_(ikey, "tokenize_bpe"), mlxd_trie_get_(symbols),
        mlxd_bpe_merges_get_(merges), mlxd_key_(okey, "tokenize_bpe"));
  });
}

extern "C" int mlxd_stream_tokenize_bpe(
    mlxd_stream* out,
    mlxd_stream dataset,
    const char* ikey,
    mlxd_trie symbols,
    mlxd_bpe_merges merges,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.tokenize_bpe(
        mlxd_key_(ikey, "tokenize_bpe"), mlxd_trie_get_(symbols),
        mlxd_bpe_merges_get_(merges), mlxd_key_(okey, "tokenize_bpe"));
  });
}

extern "C" int mlxd_stream_tokenize_bpe_if(
    mlxd_stream* out,
    mlxd_stream dataset,
    bool cond,
    const char* ikey,
    mlxd_trie symbols,
    mlxd_bpe_merges merges,
    const char* okey) {
  return mlxd_dataset_op_(out, dataset, [&](auto& ds) {
    return ds.tokenize_bpe_if(
        cond, mlxd_key_(ikey, "tokenize_bpe"), mlxd_trie_get_(symbols),
        mlxd_bpe_merges_get_(merges), mlxd_key_(okey, "tokenize_bpe"));
  });
}
