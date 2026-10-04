#ifndef MLXD_BPE_H
#define MLXD_BPE_H

#include <stdint.h>

#include "mlx/data/c/trie.h"
#include "mlx/data/c/vector_int64.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_bpe Byte-pair encoding
 * BPE merge table and tokenizer over a symbol trie.
 */
/**@{*/

/**
 * A BPE merge table. Wraps a std::shared_ptr<core::BPEMerges>; BPE
 * tokenizers share the same table.
 */
typedef struct mlxd_bpe_merges_ {
  void* ctx;
} mlxd_bpe_merges;

/**
 * Returns a new empty merge table.
 */
mlxd_bpe_merges mlxd_bpe_merges_new(void);

/**
 * Free the merge table. Safe no-op returning 0 on an empty handle; the
 * handle is unusable after this call.
 */
int mlxd_bpe_merges_free(mlxd_bpe_merges merges);

/**
 * Registers that merging the symbols `left` + `right` produces `token`.
 *
 * @returns 0 on success, 1 on an empty handle or NULL argument.
 */
int mlxd_bpe_merges_add(
    mlxd_bpe_merges merges,
    const char* left,
    const char* right,
    int64_t token);

/**
 * A BPE tokenizer over a symbol trie plus merge table. Wraps
 * core::BPETokenizer by value.
 */
typedef struct mlxd_bpe_tokenizer_ {
  void* ctx;
} mlxd_bpe_tokenizer;

/**
 * Returns a BPE tokenizer over `symbols` applying `merges`; the tokenizer
 * shares both (they stay alive through their own handles).
 */
mlxd_bpe_tokenizer mlxd_bpe_tokenizer_new(
    mlxd_trie symbols,
    mlxd_bpe_merges merges);

/**
 * Free the tokenizer. Safe no-op returning 0 on an empty handle; the
 * handle is unusable after this call.
 */
int mlxd_bpe_tokenizer_free(mlxd_bpe_tokenizer tokenizer);

/**
 * Writes the token ids of BPE-tokenizing `input` to `out`.
 *
 * @returns 0 on success, 1 on an empty handle or NULL input.
 */
int mlxd_bpe_tokenizer_tokenize(
    mlxd_vector_int64* out,
    mlxd_bpe_tokenizer tokenizer,
    const char* input);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
