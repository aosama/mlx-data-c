#ifndef MLXD_STRING_H
#define MLXD_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_string String
 * Owned string handle.
 */
/**@{*/

/**
 * A string owned by the caller. Wraps a C++ std::string.
 */
typedef struct mlxd_string_ {
  void* ctx;
} mlxd_string;

/**
 * Returns a new string copying `c_str` until its terminating `\0`.
 * Passing NULL produces an empty string.
 */
mlxd_string mlxd_string_new(const char* c_str);

/**
 * Free the string. Safe no-op returning 0 on an empty handle.
 */
int mlxd_string_free(mlxd_string str);

/**
 * Writes a pointer to the string contents to `out`.
 *
 * The pointer is owned by `str` and stays valid until the handle is freed
 * or its contents replaced.
 *
 * @returns 0 on success, 1 if `str` is an empty handle.
 */
int mlxd_string_c_str(const char** out, mlxd_string str);

/**
 * Writes the number of bytes in the string (excluding `\0`) to `out`.
 *
 * @returns 0 on success, 1 if `str` is an empty handle.
 */
int mlxd_string_size(size_t* out, mlxd_string str);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
