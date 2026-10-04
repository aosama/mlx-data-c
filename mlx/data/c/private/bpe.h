#ifndef MLXD_BPE_PRIVATE_H
#define MLXD_BPE_PRIVATE_H

#include <memory>

#include "mlx/data/core/BPETokenizer.h"
#include "mlx/data/c/bpe.h"

// The merge table is shared: BPETokenizer keeps a shared_ptr to it.
using mlxd_bpe_merges_cpp = mlx::data::core::BPEMerges;

inline mlxd_bpe_merges mlxd_bpe_merges_new_() {
  return mlxd_bpe_merges({nullptr});
}

inline mlxd_bpe_merges mlxd_bpe_merges_new_(
    const std::shared_ptr<mlxd_bpe_merges_cpp>& s) {
  return mlxd_bpe_merges({new std::shared_ptr<mlxd_bpe_merges_cpp>(s)});
}

inline mlxd_bpe_merges mlxd_bpe_merges_new_(
    std::shared_ptr<mlxd_bpe_merges_cpp>&& s) {
  return mlxd_bpe_merges(
      {new std::shared_ptr<mlxd_bpe_merges_cpp>(std::move(s))});
}

inline mlxd_bpe_merges& mlxd_bpe_merges_set_(
    mlxd_bpe_merges& d,
    const std::shared_ptr<mlxd_bpe_merges_cpp>& s) {
  if (d.ctx) {
    *static_cast<std::shared_ptr<mlxd_bpe_merges_cpp>*>(d.ctx) = s;
  } else {
    d.ctx = new std::shared_ptr<mlxd_bpe_merges_cpp>(s);
  }
  return d;
}

inline std::shared_ptr<mlxd_bpe_merges_cpp>& mlxd_bpe_merges_get_(
    mlxd_bpe_merges d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_bpe_merges");
  }
  return *static_cast<std::shared_ptr<mlxd_bpe_merges_cpp>*>(d.ctx);
}

inline void mlxd_bpe_merges_free_(mlxd_bpe_merges d) {
  if (d.ctx) {
    delete static_cast<std::shared_ptr<mlxd_bpe_merges_cpp>*>(d.ctx);
  }
}

inline mlxd_bpe_tokenizer mlxd_bpe_tokenizer_new_() {
  return mlxd_bpe_tokenizer({nullptr});
}

inline mlxd_bpe_tokenizer mlxd_bpe_tokenizer_new_(
    const mlx::data::core::BPETokenizer& s) {
  return mlxd_bpe_tokenizer({new mlx::data::core::BPETokenizer(s)});
}

inline mlxd_bpe_tokenizer mlxd_bpe_tokenizer_new_(
    mlx::data::core::BPETokenizer&& s) {
  return mlxd_bpe_tokenizer({new mlx::data::core::BPETokenizer(std::move(s))});
}

inline mlx::data::core::BPETokenizer& mlxd_bpe_tokenizer_get_(
    mlxd_bpe_tokenizer d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_bpe_tokenizer");
  }
  return *static_cast<mlx::data::core::BPETokenizer*>(d.ctx);
}

inline void mlxd_bpe_tokenizer_free_(mlxd_bpe_tokenizer d) {
  if (d.ctx) {
    delete static_cast<mlx::data::core::BPETokenizer*>(d.ctx);
  }
}

#endif
