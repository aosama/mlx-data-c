#ifndef MLXD_GRAPH_H
#define MLXD_GRAPH_H

#include <stdbool.h>

#include "mlx/data/c/trie.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_graph Tokenization graph
 * The lattice of possible tokenizations produced from a trie.
 */
/**@{*/

/**
 * A tokenization graph. Wraps a std::shared_ptr<core::Graph<int64_t>>.
 * Graphs are consumed by mlxd_tokenizer_tokenize and
 * mlxd_tokenizer_iterator_new; they are opaque otherwise.
 */
typedef struct mlxd_graph_ {
  void* ctx;
} mlxd_graph;

/**
 * Returns a new empty handle; graphs come from mlxd_tokenize_graph or
 * mlxd_tokenizer_tokenize.
 */
mlxd_graph mlxd_graph_new(void);

/**
 * Free the graph handle. Safe no-op returning 0 on an empty handle; the
 * handle is unusable after this call.
 */
int mlxd_graph_free(mlxd_graph graph);

/**
 * Builds the tokenization lattice of `input` under `trie` (wraps
 * core::tokenize). With `ignore_unk`, unknown characters are skipped
 * instead of rejecting the input.
 */
int mlxd_tokenize_graph(
    mlxd_graph* out,
    mlxd_trie trie,
    const char* input,
    bool ignore_unk);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
