#ifndef MLXD_ARRAY_H
#define MLXD_ARRAY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_array Array
 * The mlx-data Array handle: dtype + shape + contiguous buffer.
 */
/**@{*/

/**
 * Element types, mapped 1:1 to mlx::data::ArrayType.
 */
typedef enum {
  MLXD_ANY = 0,
  MLXD_UINT8 = 1,
  MLXD_INT8 = 2,
  MLXD_INT32 = 3,
  MLXD_INT64 = 4,
  MLXD_FLOAT = 5,
  MLXD_DOUBLE = 6
} mlxd_array_type;

/**
 * An array owned by the caller. Wraps a std::shared_ptr<mlx::data::Array>;
 * samples hold references to the same buffer.
 */
typedef struct mlxd_array_ {
  void* ctx;
} mlxd_array;

/**
 * Returns a new empty handle; not a usable array.
 */
mlxd_array mlxd_array_new(void);

/**
 * Free the array handle. The underlying buffer survives while other
 * references (e.g. in samples) exist. Safe no-op returning 0 on an empty
 * handle.
 */
int mlxd_array_free(mlxd_array arr);

/**
 * Shares `src`'s array into `dst` (both handles then reference the same
 * buffer).
 *
 * @returns 0 on success, 1 if `src` is an empty handle.
 */
int mlxd_array_set(mlxd_array* dst, mlxd_array src);

/**
 * Returns a new 0-dimensional array holding the scalar value.
 */
mlxd_array mlxd_array_new_scalar_uint8(uint8_t value);
mlxd_array mlxd_array_new_scalar_int8(int8_t value);
mlxd_array mlxd_array_new_scalar_int32(int32_t value);
mlxd_array mlxd_array_new_scalar_int64(int64_t value);
mlxd_array mlxd_array_new_scalar_float(float value);
mlxd_array mlxd_array_new_scalar_double(double value);

/**
 * Returns a new 1-dimensional Int8 array copying `c_str`'s bytes WITHOUT
 * the terminating `\0` (mirrors mlx::data::Array(std::string)).
 */
mlxd_array mlxd_array_new_string(const char* c_str);

/**
 * Returns a new array DEEP-COPYING `shape_num * element` bytes from `data`
 * (row-major contiguous). The caller keeps ownership of `data`.
 */
mlxd_array mlxd_array_new_data(
    mlxd_array_type type,
    const int64_t* shape,
    size_t shape_num,
    const void* data);

/**
 * Returns a new array ADOPTING `data` without copying. `dtor` is invoked
 * exactly once, with `ctx` as its argument, when the last reference to the
 * buffer drops (freeing `data` itself: pass `ctx == data`).
 *
 * Mixing this up with the copying variant is either a leak or a double
 * free: after this call, `data` must not be freed by the caller.
 */
mlxd_array mlxd_array_new_data_managed(
    mlxd_array_type type,
    const int64_t* shape,
    size_t shape_num,
    void* data,
    void (*dtor)(void*),
    void* ctx);

/**
 * Writes a pointer to the shape dimensions and the dimension count.
 *
 * The pointer is owned by `arr` and stays valid until every reference to
 * the array is gone.
 *
 * @returns 0 on success, 1 if `arr` is an empty handle.
 */
int mlxd_array_shape(const int64_t** shape, size_t* ndim, mlxd_array arr);

/**
 * Writes the number of dimensions to `out`.
 *
 * @returns 0 on success, 1 if `arr` is an empty handle.
 */
int mlxd_array_ndim(int* out, mlxd_array arr);

/**
 * Writes the total number of elements (product of the shape) to `out`.
 *
 * @returns 0 on success, 1 if `arr` is an empty handle.
 */
int mlxd_array_size(int64_t* out, mlxd_array arr);

/**
 * Writes the size in bytes of one element to `out`.
 *
 * @returns 0 on success, 1 if `arr` is an empty handle.
 */
int mlxd_array_itemsize(int64_t* out, mlxd_array arr);

/**
 * Writes the element type to `out`.
 *
 * @returns 0 on success, 1 if `arr` is an empty handle.
 */
int mlxd_array_get_type(mlxd_array_type* out, mlxd_array arr);

/**
 * Writes a pointer to the raw contiguous buffer to `out`; NULL when the
 * array holds no data. The pointer is owned by `arr` and stays valid until
 * every reference to the array is gone.
 *
 * @returns 0 on success, 1 if `arr` is an empty handle.
 */
int mlxd_array_data(const void** out, mlxd_array arr);

/**
 * Typed scalar read (mirrors Array::scalar<T>).
 *
 * @returns 0 on success, 1 if `arr` is an empty handle, holds more than one
 * element, or its type is not the requested one.
 */
int mlxd_array_item_uint8(uint8_t* out, mlxd_array arr);
int mlxd_array_item_int8(int8_t* out, mlxd_array arr);
int mlxd_array_item_int32(int32_t* out, mlxd_array arr);
int mlxd_array_item_int64(int64_t* out, mlxd_array arr);
int mlxd_array_item_float(float* out, mlxd_array arr);
int mlxd_array_item_double(double* out, mlxd_array arr);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
