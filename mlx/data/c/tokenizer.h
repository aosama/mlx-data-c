#ifndef MLXD_TOKENIZER_H
#define MLXD_TOKENIZER_H

#include <stdbool.h>
#include <stddef.h>

#include "mlx/data/c/graph.h"
#include "mlx/data/c/trie.h"
#include "mlx/data/c/vector_int64.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_tokenizer Tokenizer
 * Greedy and sampled tokenization over a trie.
 */
/**@{*/

/**
 * A tokenizer bound to a trie. Wraps core::Tokenizer by value.
 */
typedef struct mlxd_tokenizer_ {
  void* ctx;
} mlxd_tokenizer;

/**
 * Returns a tokenizer over `trie`. `trie_key_scores` (pointer + count,
 * NULL/0 for none) biases random tokenization towards higher scores.
 */
mlxd_tokenizer mlxd_tokenizer_new(
    mlxd_trie trie,
    bool ignore_unk,
    const double* trie_key_scores,
    size_t trie_key_scores_num);

/**
 * Free the tokenizer. Safe no-op returning 0 on an empty handle; the
 * handle is unusable after this call.
 */
int mlxd_tokenizer_free(mlxd_tokenizer tokenizer);

/**
 * Builds the tokenization lattice of `input` into `out`.
 *
 * @returns 0 on success, 1 on an empty handle or NULL input.
 */
int mlxd_tokenizer_tokenize(
    mlxd_graph* out,
    mlxd_tokenizer tokenizer,
    const char* input);

/**
 * Writes the token ids of the shortest tokenization to `out`. Without
 * ignore_unk (see mlxd_tokenizer_new), inputs that cannot be fully
 * tokenized return status 1.
 *
 * @returns 0 on success, 1 on an empty handle, NULL input, or an
 * untokenizable input.
 */
int mlxd_tokenizer_tokenize_shortest(
    mlxd_vector_int64* out,
    mlxd_tokenizer tokenizer,
    const char* input);

/**
 * Writes the token ids of a randomly sampled tokenization to `out`.
 *
 * @returns 0 on success, 1 on an empty handle or NULL input.
 */
int mlxd_tokenizer_tokenize_rand(
    mlxd_vector_int64* out,
    mlxd_tokenizer tokenizer,
    const char* input);

/**
 * Iterator over every tokenization encoded in a graph. Wraps
 * core::TokenizerIterator by value.
 */
typedef struct mlxd_tokenizer_iterator_ {
  void* ctx;
} mlxd_tokenizer_iterator;

/**
 * Returns an iterator over all tokenizations in `graph`.
 */
mlxd_tokenizer_iterator mlxd_tokenizer_iterator_new(mlxd_graph graph);

/**
 * Free the iterator. Safe no-op returning 0 on an empty handle; the
 * handle is unusable after this call.
 */
int mlxd_tokenizer_iterator_free(mlxd_tokenizer_iterator iterator);

/**
 * Writes the next tokenization's ids to `out`. EXHAUSTION CONTRACT: when
 * no tokenizations remain, returns 0 with an EMPTY vector
 * (mlxd_vector_int64_size == 0) — the stream end-of-data pattern.
 *
 * @returns 0 on success (including exhaustion), 1 on an empty handle.
 */
int mlxd_tokenizer_iterator_next(
    mlxd_vector_int64* out,
    mlxd_tokenizer_iterator iterator);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
