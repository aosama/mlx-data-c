#ifndef MLXD_TOKENIZER_PRIVATE_H
#define MLXD_TOKENIZER_PRIVATE_H

#include <vector>

#include "mlx/data/core/Tokenizer.h"
#include "mlx/data/c/tokenizer.h"

inline mlxd_tokenizer mlxd_tokenizer_new_() {
  return mlxd_tokenizer({nullptr});
}

inline mlxd_tokenizer mlxd_tokenizer_new_(const mlx::data::core::Tokenizer& s) {
  return mlxd_tokenizer({new mlx::data::core::Tokenizer(s)});
}

inline mlxd_tokenizer mlxd_tokenizer_new_(mlx::data::core::Tokenizer&& s) {
  return mlxd_tokenizer({new mlx::data::core::Tokenizer(std::move(s))});
}

inline mlx::data::core::Tokenizer& mlxd_tokenizer_get_(mlxd_tokenizer d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_tokenizer");
  }
  return *static_cast<mlx::data::core::Tokenizer*>(d.ctx);
}

inline void mlxd_tokenizer_free_(mlxd_tokenizer d) {
  if (d.ctx) {
    delete static_cast<mlx::data::core::Tokenizer*>(d.ctx);
  }
}

inline mlxd_tokenizer_iterator mlxd_tokenizer_iterator_new_() {
  return mlxd_tokenizer_iterator({nullptr});
}

inline mlxd_tokenizer_iterator mlxd_tokenizer_iterator_new_(
    const mlx::data::core::TokenizerIterator& s) {
  return mlxd_tokenizer_iterator(
      {new mlx::data::core::TokenizerIterator(s)});
}

inline mlxd_tokenizer_iterator mlxd_tokenizer_iterator_new_(
    mlx::data::core::TokenizerIterator&& s) {
  return mlxd_tokenizer_iterator(
      {new mlx::data::core::TokenizerIterator(std::move(s))});
}

inline mlx::data::core::TokenizerIterator& mlxd_tokenizer_iterator_get_(
    mlxd_tokenizer_iterator d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_tokenizer_iterator");
  }
  return *static_cast<mlx::data::core::TokenizerIterator*>(d.ctx);
}

inline void mlxd_tokenizer_iterator_free_(mlxd_tokenizer_iterator d) {
  if (d.ctx) {
    delete static_cast<mlx::data::core::TokenizerIterator*>(d.ctx);
  }
}

#endif
