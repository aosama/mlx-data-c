#include "mlx/data/c/graph.h"

#include "mlx/data/core/Tokenizer.h"
#include "mlx/data/c/error.h"
#include "mlx/data/c/private/graph.h"
#include "mlx/data/c/private/trie.h"

extern "C" mlxd_graph mlxd_graph_new(void) {
  return mlxd_graph_new_();
}

extern "C" int mlxd_graph_free(mlxd_graph graph) {
  try {
    mlxd_graph_free_(graph);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int
mlxd_tokenize_graph(mlxd_graph* out, mlxd_trie trie, const char* input, bool ignore_unk) {
  try {
    if (input == nullptr) {
      throw std::invalid_argument("mlxd_tokenize_graph: input is NULL");
    }
    mlxd_graph_set_(
        *out,
        mlx::data::core::tokenize(mlxd_trie_get_(trie), input, ignore_unk));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}
