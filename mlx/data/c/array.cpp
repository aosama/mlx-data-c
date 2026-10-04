#include "mlx/data/c/array.h"

#include <cstring>
#include <vector>

#include "mlx/data/Array.h"
#include "mlx/data/c/error.h"
#include "mlx/data/c/private/array.h"

// The C enum must stay 1:1 with the C++ enum; verified at compile time.
static_assert((int)MLXD_ANY == (int)mlx::data::ArrayType::Any);
static_assert((int)MLXD_UINT8 == (int)mlx::data::ArrayType::UInt8);
static_assert((int)MLXD_INT8 == (int)mlx::data::ArrayType::Int8);
static_assert((int)MLXD_INT32 == (int)mlx::data::ArrayType::Int32);
static_assert((int)MLXD_INT64 == (int)mlx::data::ArrayType::Int64);
static_assert((int)MLXD_FLOAT == (int)mlx::data::ArrayType::Float);
static_assert((int)MLXD_DOUBLE == (int)mlx::data::ArrayType::Double);

namespace {

std::vector<int64_t> mlxd_shape_to_vector_(
    const int64_t* shape,
    size_t shape_num) {
  return std::vector<int64_t>(shape, shape + shape_num);
}

} // namespace

extern "C" mlxd_array mlxd_array_new(void) {
  return mlxd_array_new_();
}

extern "C" int mlxd_array_free(mlxd_array arr) {
  try {
    mlxd_array_free_(arr);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_array_set(mlxd_array* dst, mlxd_array src) {
  try {
    mlxd_array_set_(*dst, mlxd_array_get_(src));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" mlxd_array mlxd_array_new_scalar_uint8(uint8_t value) {
  try {
    return mlxd_array_new_(
        std::make_shared<mlx::data::Array>(static_cast<unsigned char>(value)));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_array_new_();
  }
}

extern "C" mlxd_array mlxd_array_new_scalar_int8(int8_t value) {
  try {
    return mlxd_array_new_(
        std::make_shared<mlx::data::Array>(static_cast<char>(value)));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_array_new_();
  }
}

extern "C" mlxd_array mlxd_array_new_scalar_int32(int32_t value) {
  try {
    return mlxd_array_new_(std::make_shared<mlx::data::Array>(value));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_array_new_();
  }
}

extern "C" mlxd_array mlxd_array_new_scalar_int64(int64_t value) {
  try {
    return mlxd_array_new_(std::make_shared<mlx::data::Array>(value));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_array_new_();
  }
}

extern "C" mlxd_array mlxd_array_new_scalar_float(float value) {
  try {
    return mlxd_array_new_(std::make_shared<mlx::data::Array>(value));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_array_new_();
  }
}

extern "C" mlxd_array mlxd_array_new_scalar_double(double value) {
  try {
    return mlxd_array_new_(std::make_shared<mlx::data::Array>(value));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_array_new_();
  }
}

extern "C" mlxd_array mlxd_array_new_string(const char* c_str) {
  try {
    return mlxd_array_new_(std::make_shared<mlx::data::Array>(
        std::string(c_str ? c_str : "")));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_array_new_();
  }
}

extern "C" mlxd_array mlxd_array_new_data(
    mlxd_array_type type,
    const int64_t* shape,
    size_t shape_num,
    const void* data) {
  try {
    auto arr = std::make_shared<mlx::data::Array>(
        static_cast<mlx::data::ArrayType>(type),
        mlxd_shape_to_vector_(shape, shape_num));
    int64_t bytes = arr->size() * arr->itemsize();
    if (bytes > 0) {
      if (data == nullptr) {
        throw std::runtime_error(
            "mlxd_array_new_data: NULL data for a non-empty array");
      }
      std::memcpy(arr->data(), data, bytes);
    }
    return mlxd_array_new_(std::move(arr));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_array_new_();
  }
}

extern "C" mlxd_array mlxd_array_new_data_managed(
    mlxd_array_type type,
    const int64_t* shape,
    size_t shape_num,
    void* data,
    void (*dtor)(void*),
    void* ctx) {
  try {
    if (dtor == nullptr) {
      throw std::runtime_error(
          "mlxd_array_new_data_managed: dtor must not be NULL");
    }
    // The deleter ignores the watched pointer and hands `ctx` to `dtor`,
    // mirroring mlx-c's managed-payload variant.
    std::shared_ptr<void> owned(data, [dtor, ctx](void*) { dtor(ctx); });
    return mlxd_array_new_(std::make_shared<mlx::data::Array>(
        static_cast<mlx::data::ArrayType>(type),
        mlxd_shape_to_vector_(shape, shape_num),
        std::move(owned)));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_array_new_();
  }
}

extern "C" int mlxd_array_shape(
    const int64_t** shape,
    size_t* ndim,
    mlxd_array arr) {
  try {
    const std::vector<int64_t>& cpp_shape = mlxd_array_get_(arr)->shape();
    *shape = cpp_shape.data();
    *ndim = cpp_shape.size();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_array_ndim(int* out, mlxd_array arr) {
  try {
    *out = mlxd_array_get_(arr)->ndim();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_array_size(int64_t* out, mlxd_array arr) {
  try {
    *out = mlxd_array_get_(arr)->size();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_array_itemsize(int64_t* out, mlxd_array arr) {
  try {
    *out = mlxd_array_get_(arr)->itemsize();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_array_get_type(mlxd_array_type* out, mlxd_array arr) {
  try {
    *out = static_cast<mlxd_array_type>(mlxd_array_get_(arr)->type());
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_array_data(const void** out, mlxd_array arr) {
  try {
    *out = mlxd_array_get_(arr)->data();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_array_item_uint8(uint8_t* out, mlxd_array arr) {
  try {
    *out = mlxd_array_get_(arr)->scalar<unsigned char>();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_array_item_int8(int8_t* out, mlxd_array arr) {
  try {
    *out = mlxd_array_get_(arr)->scalar<char>();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_array_item_int32(int32_t* out, mlxd_array arr) {
  try {
    *out = mlxd_array_get_(arr)->scalar<int32_t>();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_array_item_int64(int64_t* out, mlxd_array arr) {
  try {
    *out = mlxd_array_get_(arr)->scalar<int64_t>();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_array_item_float(float* out, mlxd_array arr) {
  try {
    *out = mlxd_array_get_(arr)->scalar<float>();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_array_item_double(double* out, mlxd_array arr) {
  try {
    *out = mlxd_array_get_(arr)->scalar<double>();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}
