#include "mlx/data/c/bpe.h"

#include "mlx/data/core/BPETokenizer.h"
#include "mlx/data/c/error.h"
#include "mlx/data/c/private/bpe.h"
#include "mlx/data/c/private/trie.h"
#include "mlx/data/c/private/vector_int64.h"

extern "C" mlxd_bpe_merges mlxd_bpe_merges_new(void) {
  try {
    return mlxd_bpe_merges_new_(
        std::make_shared<mlx::data::core::BPEMerges>());
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_bpe_merges_new_();
  }
}

extern "C" int mlxd_bpe_merges_free(mlxd_bpe_merges merges) {
  try {
    mlxd_bpe_merges_free_(merges);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_bpe_merges_add(
    mlxd_bpe_merges merges,
    const char* left,
    const char* right,
    int64_t token) {
  try {
    if (left == nullptr || right == nullptr) {
      throw std::invalid_argument("mlxd_bpe_merges_add: NULL symbol");
    }
    mlxd_bpe_merges_get_(merges)->add(left, right, token);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" mlxd_bpe_tokenizer mlxd_bpe_tokenizer_new(
    mlxd_trie symbols,
    mlxd_bpe_merges merges) {
  try {
    return mlxd_bpe_tokenizer_new_(mlx::data::core::BPETokenizer(
        mlxd_trie_get_(symbols), mlxd_bpe_merges_get_(merges)));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_bpe_tokenizer_new_();
  }
}

extern "C" int mlxd_bpe_tokenizer_free(mlxd_bpe_tokenizer tokenizer) {
  try {
    mlxd_bpe_tokenizer_free_(tokenizer);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_bpe_tokenizer_tokenize(
    mlxd_vector_int64* out,
    mlxd_bpe_tokenizer tokenizer,
    const char* input) {
  try {
    if (input == nullptr) {
      throw std::invalid_argument("mlxd_bpe_tokenizer_tokenize: input is NULL");
    }
    mlxd_vector_int64_set_(
        *out, mlxd_bpe_tokenizer_get_(tokenizer).tokenize(input));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}
