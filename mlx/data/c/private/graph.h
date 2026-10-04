#ifndef MLXD_GRAPH_PRIVATE_H
#define MLXD_GRAPH_PRIVATE_H

#include <memory>

#include "mlx/data/core/Graph.h"
#include "mlx/data/c/graph.h"

using mlxd_graph_cpp = mlx::data::core::Graph<int64_t>;

inline mlxd_graph mlxd_graph_new_() {
  return mlxd_graph({nullptr});
}

inline mlxd_graph mlxd_graph_new_(const std::shared_ptr<mlxd_graph_cpp>& s) {
  return mlxd_graph({new std::shared_ptr<mlxd_graph_cpp>(s)});
}

inline mlxd_graph mlxd_graph_new_(std::shared_ptr<mlxd_graph_cpp>&& s) {
  return mlxd_graph({new std::shared_ptr<mlxd_graph_cpp>(std::move(s))});
}

inline mlxd_graph& mlxd_graph_set_(
    mlxd_graph& d,
    const std::shared_ptr<mlxd_graph_cpp>& s) {
  if (d.ctx) {
    *static_cast<std::shared_ptr<mlxd_graph_cpp>*>(d.ctx) = s;
  } else {
    d.ctx = new std::shared_ptr<mlxd_graph_cpp>(s);
  }
  return d;
}

inline std::shared_ptr<mlxd_graph_cpp>& mlxd_graph_get_(mlxd_graph d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_graph");
  }
  return *static_cast<std::shared_ptr<mlxd_graph_cpp>*>(d.ctx);
}

inline void mlxd_graph_free_(mlxd_graph d) {
  if (d.ctx) {
    delete static_cast<std::shared_ptr<mlxd_graph_cpp>*>(d.ctx);
  }
}

#endif
