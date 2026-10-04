#include "mlx/data/c/buffer.h"

#include <stdexcept>
#include <string>
#include <vector>

#include "mlx/data/Buffer.h"
#include "mlx/data/c/error.h"
#include "mlx/data/c/private/buffer.h"
#include "mlx/data/c/private/maps.h"
#include "mlx/data/c/private/sample.h"
#include "mlx/data/c/private/stream.h"

extern "C" mlxd_buffer mlxd_buffer_new(void) {
  return mlxd_buffer_new_();
}

extern "C" int mlxd_buffer_free(mlxd_buffer buffer) {
  try {
    mlxd_buffer_free_(buffer);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_set(mlxd_buffer* dst, mlxd_buffer src) {
  try {
    mlxd_buffer_set_(*dst, mlxd_buffer_get_(src));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_get(mlxd_sample* out, mlxd_buffer buffer, int64_t idx) {
  try {
    mlxd_sample_set_(*out, mlxd_buffer_get_(buffer).get(idx));
  } catch (std::out_of_range&) {
    return 2;
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_size(int64_t* out, mlxd_buffer buffer) {
  try {
    *out = mlxd_buffer_get_(buffer).size();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_batch(
    mlxd_buffer* out,
    mlxd_buffer buffer,
    int64_t batch_size,
    const char** pad_keys,
    const double* pad_values,
    size_t pad_num,
    const char** dim_keys,
    const int* dim_values,
    size_t dim_num) {
  try {
    mlxd_buffer_set_(
        *out,
        mlxd_buffer_get_(buffer).batch(
            batch_size,
            mlxd_pad_values_(pad_keys, pad_values, pad_num),
            mlxd_batch_dims_(dim_keys, dim_values, dim_num)));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_batch_sizes(
    mlxd_buffer* out,
    mlxd_buffer buffer,
    const int64_t* batch_sizes,
    size_t batch_sizes_num,
    const char** pad_keys,
    const double* pad_values,
    size_t pad_num,
    const char** dim_keys,
    const int* dim_values,
    size_t dim_num) {
  try {
    mlxd_buffer_set_(
        *out,
        mlxd_buffer_get_(buffer).batch(
            std::vector<int64_t>(batch_sizes, batch_sizes + batch_sizes_num),
            mlxd_pad_values_(pad_keys, pad_values, pad_num),
            mlxd_batch_dims_(dim_keys, dim_values, dim_num)));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_dynamic_batch(
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
    bool drop_outliers) {
  try {
    if (key == nullptr) {
      throw std::invalid_argument("mlxd_buffer_dynamic_batch: key is NULL");
    }
    mlxd_buffer_set_(
        *out,
        mlxd_buffer_get_(buffer).dynamic_batch(
            std::string(key),
            min_data_size,
            max_data_size,
            mlxd_pad_values_(pad_keys, pad_values, pad_num),
            mlxd_batch_dims_(dim_keys, dim_values, dim_num),
            drop_outliers));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_dynamic_batch_from_size_buffer(
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
    bool drop_outliers) {
  try {
    if (key == nullptr) {
      throw std::invalid_argument(
          "mlxd_buffer_dynamic_batch_from_size_buffer: key is NULL");
    }
    mlxd_buffer_set_(
        *out,
        mlxd_buffer_get_(buffer).dynamic_batch(
            mlxd_buffer_get_(size_buffer),
            std::string(key),
            min_data_size,
            max_data_size,
            mlxd_pad_values_(pad_keys, pad_values, pad_num),
            mlxd_batch_dims_(dim_keys, dim_values, dim_num),
            drop_outliers));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_ordered_prefetch(
    mlxd_stream* out,
    mlxd_buffer buffer,
    int prefetch_size,
    int num_thread) {
  try {
    mlxd_stream_set_(
        *out,
        mlxd_buffer_get_(buffer).ordered_prefetch(prefetch_size, num_thread));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_partition(
    mlxd_buffer* out,
    mlxd_buffer buffer,
    int64_t num_partitions,
    int64_t partition) {
  try {
    mlxd_buffer_set_(
        *out, mlxd_buffer_get_(buffer).partition(num_partitions, partition));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_partition_if(
    mlxd_buffer* out,
    mlxd_buffer buffer,
    bool cond,
    int64_t num_partitions,
    int64_t partition) {
  try {
    mlxd_buffer_set_(
        *out,
        mlxd_buffer_get_(buffer).partition_if(cond, num_partitions, partition));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int
mlxd_buffer_append(mlxd_buffer* out, mlxd_buffer buffer, mlxd_buffer other) {
  try {
    mlxd_buffer_set_(
        *out, mlxd_buffer_get_(buffer).append(mlxd_buffer_get_(other)));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_perm(
    mlxd_buffer* out,
    mlxd_buffer buffer,
    const int64_t* perm,
    size_t perm_num) {
  try {
    mlxd_buffer_set_(
        *out,
        mlxd_buffer_get_(buffer).perm(
            std::vector<int64_t>(perm, perm + perm_num)));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_shuffle(mlxd_buffer* out, mlxd_buffer buffer) {
  try {
    mlxd_buffer_set_(*out, mlxd_buffer_get_(buffer).shuffle());
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int
mlxd_buffer_shuffle_if(mlxd_buffer* out, mlxd_buffer buffer, bool cond) {
  try {
    mlxd_buffer_set_(*out, mlxd_buffer_get_(buffer).shuffle_if(cond));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_to_stream(mlxd_stream* out, mlxd_buffer buffer) {
  try {
    mlxd_stream_set_(*out, mlxd_buffer_get_(buffer).to_stream());
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_concretize(mlxd_buffer* out, mlxd_buffer buffer) {
  try {
    mlxd_buffer_set_(*out, mlxd_buffer_get_(buffer).concretize());
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_from_samples(
    mlxd_buffer* out,
    const mlxd_sample* samples,
    size_t num) {
  try {
    if (num > 0 && samples == nullptr) {
      throw std::runtime_error(
          "mlxd_buffer_from_samples: NULL samples for a non-empty list");
    }
    std::vector<mlx::data::Sample> cpp_samples;
    cpp_samples.reserve(num);
    for (size_t i = 0; i < num; i++) {
      cpp_samples.push_back(mlxd_sample_get_(samples[i]));
    }
    mlxd_buffer_set_(*out, mlx::data::buffer_from_vector(std::move(cpp_samples)));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_buffer_files_from_tar(
    mlxd_buffer* out,
    const char* tarfile,
    bool nested,
    int num_threads) {
  try {
    if (tarfile == nullptr) {
      throw std::invalid_argument("mlxd_buffer_files_from_tar: tarfile is NULL");
    }
    mlxd_buffer_set_(
        *out,
        mlx::data::files_from_tar(tarfile, nested, num_threads));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}
