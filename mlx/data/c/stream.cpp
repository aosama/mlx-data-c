#include "mlx/data/c/stream.h"

#include <stdexcept>
#include <string>

#include "mlx/data/Stream.h"
#include "mlx/data/c/error.h"
#include "mlx/data/c/private/buffer.h"
#include "mlx/data/c/private/maps.h"
#include "mlx/data/c/private/sample.h"
#include "mlx/data/c/private/stream.h"

extern "C" mlxd_stream mlxd_stream_new(void) {
  return mlxd_stream_new_();
}

extern "C" int mlxd_stream_free(mlxd_stream stream) {
  try {
    mlxd_stream_free_(stream);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_set(mlxd_stream* dst, mlxd_stream src) {
  try {
    mlxd_stream_set_(*dst, mlxd_stream_get_(src));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_next(mlxd_sample* out, mlxd_stream stream) {
  try {
    mlxd_sample_set_(*out, mlxd_stream_get_(stream).next());
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_reset(mlxd_stream stream) {
  try {
    mlxd_stream_get_(stream).reset();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_batch(
    mlxd_stream* out,
    mlxd_stream stream,
    int64_t batch_size,
    const char** pad_keys,
    const double* pad_values,
    size_t pad_num,
    const char** dim_keys,
    const int* dim_values,
    size_t dim_num) {
  try {
    mlxd_stream_set_(
        *out,
        mlxd_stream_get_(stream).batch(
            batch_size,
            mlxd_pad_values_(pad_keys, pad_values, pad_num),
            mlxd_batch_dims_(dim_keys, dim_values, dim_num)));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_dynamic_batch(
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
    int num_thread) {
  try {
    if (key == nullptr) {
      throw std::invalid_argument("mlxd_stream_dynamic_batch: key is NULL");
    }
    mlxd_stream_set_(
        *out,
        mlxd_stream_get_(stream).dynamic_batch(
            buffer_size,
            std::string(key),
            min_data_size,
            max_data_size,
            mlxd_pad_values_(pad_keys, pad_values, pad_num),
            mlxd_batch_dims_(dim_keys, dim_values, dim_num),
            shuffle,
            drop_outliers,
            max_skipped_samples,
            num_thread));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_csv_reader_from_key(
    mlxd_stream* out,
    mlxd_stream stream,
    const char* key,
    char sep,
    char quote,
    bool from_memory,
    const char* local_prefix) {
  try {
    if (key == nullptr) {
      throw std::invalid_argument("mlxd_stream_csv_reader_from_key: key is NULL");
    }
    mlxd_stream_set_(
        *out,
        mlxd_stream_get_(stream).csv_reader_from_key(
            std::string(key),
            sep,
            quote,
            from_memory,
            local_prefix ? local_prefix : ""));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_line_reader_from_key(
    mlxd_stream* out,
    mlxd_stream stream,
    const char* key,
    const char* dst_key,
    bool from_memory,
    bool unzip,
    const char* local_prefix) {
  try {
    if (key == nullptr || dst_key == nullptr) {
      throw std::invalid_argument("mlxd_stream_line_reader_from_key: NULL key");
    }
    mlxd_stream_set_(
        *out,
        mlxd_stream_get_(stream).line_reader_from_key(
            std::string(key),
            std::string(dst_key),
            from_memory,
            unzip,
            local_prefix ? local_prefix : ""));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_partition(
    mlxd_stream* out,
    mlxd_stream stream,
    int64_t num_partitions,
    int64_t partition) {
  try {
    mlxd_stream_set_(
        *out, mlxd_stream_get_(stream).partition(num_partitions, partition));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_partition_if(
    mlxd_stream* out,
    mlxd_stream stream,
    bool cond,
    int64_t num_partitions,
    int64_t partition) {
  try {
    mlxd_stream_set_(
        *out,
        mlxd_stream_get_(stream).partition_if(cond, num_partitions, partition));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_prefetch(
    mlxd_stream* out,
    mlxd_stream stream,
    int prefetch_size,
    int num_thread) {
  try {
    mlxd_stream_set_(
        *out, mlxd_stream_get_(stream).prefetch(prefetch_size, num_thread));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_prefetch_if(
    mlxd_stream* out,
    mlxd_stream stream,
    bool cond,
    int prefetch_size,
    int num_thread) {
  try {
    mlxd_stream_set_(
        *out,
        mlxd_stream_get_(stream).prefetch_if(cond, prefetch_size, num_thread));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int
mlxd_stream_repeat(mlxd_stream* out, mlxd_stream stream, int64_t num_time) {
  try {
    mlxd_stream_set_(*out, mlxd_stream_get_(stream).repeat(num_time));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int
mlxd_stream_shuffle(mlxd_stream* out, mlxd_stream stream, int64_t buffer_size) {
  try {
    mlxd_stream_set_(*out, mlxd_stream_get_(stream).shuffle(buffer_size));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_shuffle_if(
    mlxd_stream* out,
    mlxd_stream stream,
    bool cond,
    int64_t buffer_size) {
  try {
    mlxd_stream_set_(
        *out, mlxd_stream_get_(stream).shuffle_if(cond, buffer_size));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_sliding_window(
    mlxd_stream* out,
    mlxd_stream stream,
    const char* key,
    int64_t size,
    int64_t stride,
    int dim,
    const char* index_key) {
  try {
    if (key == nullptr) {
      throw std::invalid_argument("mlxd_stream_sliding_window: key is NULL");
    }
    mlxd_stream_set_(
        *out,
        mlxd_stream_get_(stream).sliding_window(
            std::string(key),
            size,
            stride,
            dim,
            index_key ? std::string(index_key) : std::string()));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_to_buffer(mlxd_buffer* out, mlxd_stream stream) {
  try {
    mlxd_buffer_set_(*out, mlxd_stream_get_(stream).to_buffer());
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_csv_reader(
    mlxd_stream* out,
    const char* filename,
    char sep,
    char quote,
    const char* local_prefix) {
  try {
    if (filename == nullptr) {
      throw std::invalid_argument("mlxd_stream_csv_reader: filename is NULL");
    }
    mlxd_stream_set_(
        *out,
        mlx::data::stream_csv_reader(
            filename,
            sep,
            quote,
            local_prefix ? local_prefix : ""));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_csv_reader_from_string(
    mlxd_stream* out,
    const char* contents,
    char sep,
    char quote) {
  try {
    if (contents == nullptr) {
      throw std::invalid_argument(
          "mlxd_stream_csv_reader_from_string: contents is NULL");
    }
    mlxd_stream_set_(
        *out, mlx::data::stream_csv_reader_from_string(contents, sep, quote));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_stream_line_reader(
    mlxd_stream* out,
    const char* filename,
    const char* key,
    bool unzip,
    const char* local_prefix) {
  try {
    if (filename == nullptr || key == nullptr) {
      throw std::invalid_argument("mlxd_stream_line_reader: NULL filename/key");
    }
    mlxd_stream_set_(
        *out,
        mlx::data::stream_line_reader(
            filename,
            std::string(key),
            unzip,
            local_prefix ? local_prefix : ""));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}
