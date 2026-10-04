#ifndef MLXD_CLOSURE_H
#define MLXD_CLOSURE_H

#include "mlx/data/c/array.h"
#include "mlx/data/c/buffer.h"
#include "mlx/data/c/sample.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_closure Closures
 * User C callbacks bridged into mlx-data's std::function escape hatches.
 *
 * OWNERSHIP CONTRACT: a callback receives a BORROWED input handle — it
 * must not free it (and for arrays must not modify the underlying
 * buffer). It returns a handle it OWNS; ownership of the returned
 * object transfers to the pipeline.
 *
 * LIFETIME: the closure handle shares the fn/ctx state with the
 * pipelines it was built into. Freeing the handle does NOT invalidate
 * them; the state (and the ctx destructor) survives until every
 * reference — the handle and all pipelines — is gone. Callbacks may
 * therefore run after mlxd_closure_*_free: keep the function code and
 * ctx valid for as long as the datasets live. Callbacks used by
 * background threads (buffered, prefetch paths) must be thread-safe.
 */
/**@{*/

/**
 * Per-key array transform callback: receives the borrowed input array
 * and returns a newly built output array (empty handle = failure).
 */
typedef mlxd_array (*mlxd_closure_array_fn)(mlxd_array input, void* ctx);

/**
 * Whole-sample transform callback: receives a borrowed copy of the
 * sample (its array buffers are shared) and returns the new sample.
 */
typedef mlxd_sample (*mlxd_closure_sample_fn)(mlxd_sample input, void* ctx);

/**
 * Stream refill callback: receives a borrowed buffer of samples to
 * append to and returns the refilled buffer.
 */
typedef mlxd_buffer (*mlxd_closure_buffer_fn)(mlxd_buffer input, void* ctx);

/**
 * A closure wrapping an mlxd_closure_array_fn.
 */
typedef struct mlxd_closure_array_ {
  void* ctx;
} mlxd_closure_array;

/**
 * Wraps `fn` with `ctx` (handed back on every call). When the last
 * reference to the closure state drops, `dtor` runs once on `ctx`.
 *
 * @returns 0 on success, 1 if `fn` is NULL.
 */
int mlxd_closure_array_new(
    mlxd_closure_array* out,
    mlxd_closure_array_fn fn,
    void* ctx,
    void (*dtor)(void*));

/**
 * Releases the caller's reference to the closure state. See the
 * LIFETIME note above.
 */
int mlxd_closure_array_free(mlxd_closure_array closure);

/**
 * A closure wrapping an mlxd_closure_sample_fn.
 */
typedef struct mlxd_closure_sample_ {
  void* ctx;
} mlxd_closure_sample;

int mlxd_closure_sample_new(
    mlxd_closure_sample* out,
    mlxd_closure_sample_fn fn,
    void* ctx,
    void (*dtor)(void*));
int mlxd_closure_sample_free(mlxd_closure_sample closure);

/**
 * A closure wrapping an mlxd_closure_buffer_fn.
 */
typedef struct mlxd_closure_buffer_ {
  void* ctx;
} mlxd_closure_buffer;

int mlxd_closure_buffer_new(
    mlxd_closure_buffer* out,
    mlxd_closure_buffer_fn fn,
    void* ctx,
    void (*dtor)(void*));
int mlxd_closure_buffer_free(mlxd_closure_buffer closure);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
