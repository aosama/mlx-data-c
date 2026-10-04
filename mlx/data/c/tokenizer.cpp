#include "mlx/data/c/tokenizer.h"

#include <vector>

#include "mlx/data/core/Tokenizer.h"
#include "mlx/data/c/error.h"
#include "mlx/data/c/private/graph.h"
#include "mlx/data/c/private/tokenizer.h"
#include "mlx/data/c/private/trie.h"
#include "mlx/data/c/private/vector_int64.h"

extern "C" mlxd_tokenizer mlxd_tokenizer_new(
    mlxd_trie trie,
    bool ignore_unk,
    const double* trie_key_scores,
    size_t trie_key_scores_num) {
  try {
    std::vector<double> scores;
    if (trie_key_scores != nullptr) {
      scores.assign(trie_key_scores, trie_key_scores + trie_key_scores_num);
    }
    return mlxd_tokenizer_new_(mlx::data::core::Tokenizer(
        mlxd_trie_get_(trie), ignore_unk, scores));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_tokenizer_new_();
  }
}

extern "C" int mlxd_tokenizer_free(mlxd_tokenizer tokenizer) {
  try {
    mlxd_tokenizer_free_(tokenizer);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_tokenizer_tokenize(
    mlxd_graph* out,
    mlxd_tokenizer tokenizer,
    const char* input) {
  try {
    if (input == nullptr) {
      throw std::invalid_argument("mlxd_tokenizer_tokenize: input is NULL");
    }
    mlxd_graph_set_(*out, mlxd_tokenizer_get_(tokenizer).tokenize(input));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_tokenizer_tokenize_shortest(
    mlxd_vector_int64* out,
    mlxd_tokenizer tokenizer,
    const char* input) {
  try {
    if (input == nullptr) {
      throw std::invalid_argument(
          "mlxd_tokenizer_tokenize_shortest: input is NULL");
    }
    mlxd_vector_int64_set_(
        *out, mlxd_tokenizer_get_(tokenizer).tokenize_shortest(input));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_tokenizer_tokenize_rand(
    mlxd_vector_int64* out,
    mlxd_tokenizer tokenizer,
    const char* input) {
  try {
    if (input == nullptr) {
      throw std::invalid_argument(
          "mlxd_tokenizer_tokenize_rand: input is NULL");
    }
    mlxd_vector_int64_set_(
        *out, mlxd_tokenizer_get_(tokenizer).tokenize_rand(input));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" mlxd_tokenizer_iterator
mlxd_tokenizer_iterator_new(mlxd_graph graph) {
  try {
    return mlxd_tokenizer_iterator_new_(
        mlx::data::core::TokenizerIterator(mlxd_graph_get_(graph)));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_tokenizer_iterator_new_();
  }
}

extern "C" int mlxd_tokenizer_iterator_free(mlxd_tokenizer_iterator iterator) {
  try {
    mlxd_tokenizer_iterator_free_(iterator);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_tokenizer_iterator_next(
    mlxd_vector_int64* out,
    mlxd_tokenizer_iterator iterator) {
  try {
    mlxd_vector_int64_set_(*out, mlxd_tokenizer_iterator_get_(iterator).next());
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}
