#ifndef MLXD_CORE_H
#define MLXD_CORE_H

#include <stdint.h>

#include "mlx/data/c/array.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_core Misc core utilities
 * RNG state seeding and array utilities from mlx::data::core.
 */
/**@{*/

/**
 * Seeds the thread-local RNG behind the random operations
 * (image_random_*, random_slice, shuffle, tokenize_rand). Call from the
 * main thread before building pipelines; identically-seeded runs then
 * produce identical results.
 *
 * @returns 0 on success, 1 on error.
 */
int mlxd_set_state(int64_t seed);

/**
 * Edit-distance counts between 1-D arrays `a` and `b`, with effective
 * lengths taken from the Int64 1-D arrays `a_length` and `b_length`
 * (single-element for a plain comparison). Writes an Int64 3-element
 * array to `out`: {deletion, insertion, substitution} counts — the
 * classic edit distance is their sum. Dtype rules mirror the C++
 * implementation (both arrays same type; lengths Int64) and violations
 * return status 1.
 *
 * @returns 0 on success, 1 on an empty handle, NULL-adjacent misuse, or
 * a rule violation from the C++ core.
 */
int mlxd_levenshtein(
    mlxd_array* out,
    mlxd_array a,
    mlxd_array a_length,
    mlxd_array b,
    mlxd_array b_length);

/**
 * Removes CONSECUTIVE duplicate elements along `dim` of `src`
 * (`src_length`: Int64 array with the valid length of every outer
 * element; `pad`: the fill value). Writes the compacted array and its
 * new Int64 lengths.
 *
 * @returns 0 on success, 1 on an empty handle or a rule violation
 * (length-array size/type mismatches) from the C++ core.
 */
int mlxd_uniq(
    mlxd_array* out,
    mlxd_array* out_length,
    mlxd_array src,
    mlxd_array src_length,
    int dim,
    double pad);

/**
 * Strips trailing elements equal to `value` along `dim` of `src` (up to
 * the lengths in `src_length`) and refills with `pad`; writes the
 * compacted array and its new Int64 lengths.
 *
 * @returns 0 on success, 1 on an empty handle or a rule violation from
 * the C++ core.
 */
int mlxd_remove(
    mlxd_array* out,
    mlxd_array* out_length,
    mlxd_array src,
    mlxd_array src_length,
    int dim,
    double value,
    double pad);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
