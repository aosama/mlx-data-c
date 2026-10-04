#ifndef MLXD_TRIE_H
#define MLXD_TRIE_H

#include <stdint.h>

#include "mlx/data/c/string.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_trie Trie
 * Character trie for tokenization vocabulary.
 */
/**@{*/

/**
 * A trie over characters. Wraps a std::shared_ptr<core::Trie<char>>;
 * tokenizers share the same trie.
 */
typedef struct mlxd_trie_ {
  void* ctx;
} mlxd_trie;

/**
 * Returns a new empty trie.
 */
mlxd_trie mlxd_trie_new(void);

/**
 * Free the trie handle. Tokenizers still holding it stay alive through
 * their own references. Safe no-op returning 0 on an empty handle; the
 * handle is unusable after this call.
 */
int mlxd_trie_free(mlxd_trie trie);

/**
 * Inserts `key` with token id `id`; an `id` < 0 assigns the next automatic
 * id (starting at 0). Inserting an existing key is a no-op.
 *
 * @returns 0 on success, 1 on an empty handle or NULL key.
 */
int mlxd_trie_insert(mlxd_trie trie, const char* key, int64_t id);

/**
 * Writes the token id stored under `key` to `out`.
 *
 * @returns 0 on success, 2 if `key` is not in the trie, 1 on an empty
 * handle or NULL key.
 */
int mlxd_trie_search(int64_t* out, mlxd_trie trie, const char* key);

/**
 * Writes the number of stored keys to `out`.
 *
 * @returns 0 on success, 1 if `trie` is an empty handle.
 */
int mlxd_trie_num_keys(int64_t* out, mlxd_trie trie);

/**
 * Writes the key stored under token id `id` to `out`.
 *
 * @returns 0 on success, 2 if no key has this id, 1 on an empty handle.
 */
int mlxd_trie_key_string(mlxd_string* out, mlxd_trie trie, int64_t id);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
