#ifndef MLXD_TRIE_PRIVATE_H
#define MLXD_TRIE_PRIVATE_H

#include <memory>

#include "mlx/data/core/Trie.h"
#include "mlx/data/c/trie.h"

using mlxd_trie_cpp = mlx::data::core::Trie<char>;

inline mlxd_trie mlxd_trie_new_() {
  return mlxd_trie({nullptr});
}

inline mlxd_trie mlxd_trie_new_(const std::shared_ptr<mlxd_trie_cpp>& s) {
  return mlxd_trie({new std::shared_ptr<mlxd_trie_cpp>(s)});
}

inline mlxd_trie mlxd_trie_new_(std::shared_ptr<mlxd_trie_cpp>&& s) {
  return mlxd_trie({new std::shared_ptr<mlxd_trie_cpp>(std::move(s))});
}

inline mlxd_trie& mlxd_trie_set_(
    mlxd_trie& d,
    const std::shared_ptr<mlxd_trie_cpp>& s) {
  if (d.ctx) {
    *static_cast<std::shared_ptr<mlxd_trie_cpp>*>(d.ctx) = s;
  } else {
    d.ctx = new std::shared_ptr<mlxd_trie_cpp>(s);
  }
  return d;
}

inline std::shared_ptr<mlxd_trie_cpp>& mlxd_trie_get_(mlxd_trie d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_trie");
  }
  return *static_cast<std::shared_ptr<mlxd_trie_cpp>*>(d.ctx);
}

inline void mlxd_trie_free_(mlxd_trie d) {
  if (d.ctx) {
    delete static_cast<std::shared_ptr<mlxd_trie_cpp>*>(d.ctx);
  }
}

#endif
