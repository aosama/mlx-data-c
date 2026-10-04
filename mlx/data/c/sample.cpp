#include "mlx/data/c/sample.h"

#include "mlx/data/Sample.h"
#include "mlx/data/c/error.h"
#include "mlx/data/c/private/array.h"
#include "mlx/data/c/private/sample.h"
#include "mlx/data/c/private/vector_string.h"

extern "C" mlxd_sample mlxd_sample_new(void) {
  try {
    return mlxd_sample_new_(mlx::data::Sample());
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_sample_new_();
  }
}

extern "C" int mlxd_sample_free(mlxd_sample sample) {
  try {
    mlxd_sample_free_(sample);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_sample_set(mlxd_sample* dst, mlxd_sample src) {
  try {
    mlxd_sample_set_(*dst, mlxd_sample_get_(src));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_sample_size(int64_t* out, mlxd_sample sample) {
  try {
    *out = static_cast<int64_t>(mlxd_sample_get_(sample).size());
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_sample_keys(mlxd_vector_string* out, mlxd_sample sample) {
  try {
    mlxd_vector_string_set_(*out, mlx::data::sample::keys(mlxd_sample_get_(sample)));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int
mlxd_sample_get(mlxd_array* out, mlxd_sample sample, const char* key) {
  try {
    if (key == nullptr) {
      throw std::runtime_error("mlxd_sample_get: key must not be NULL");
    }
    mlx::data::Sample& dict = mlxd_sample_get_(sample);
    auto it = dict.find(key);
    if (it == dict.end()) {
      return 2;
    }
    mlxd_array_set_(*out, it->second);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int
mlxd_sample_set_key(mlxd_sample sample, const char* key, mlxd_array value) {
  try {
    if (key == nullptr) {
      throw std::runtime_error("mlxd_sample_set_key: key must not be NULL");
    }
    mlxd_sample_get_(sample)[key] = mlxd_array_get_(value);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_sample_erase(mlxd_sample sample, const char* key) {
  try {
    if (key == nullptr) {
      throw std::runtime_error("mlxd_sample_erase: key must not be NULL");
    }
    mlx::data::Sample& dict = mlxd_sample_get_(sample);
    if (dict.erase(key) == 0) {
      return 2;
    }
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}
