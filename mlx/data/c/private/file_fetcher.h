#ifndef MLXD_FILE_FETCHER_PRIVATE_H
#define MLXD_FILE_FETCHER_PRIVATE_H

#include <memory>

#include "mlx/data/core/FileFetcher.h"
#include "mlx/data/c/file_fetcher.h"

inline mlxd_file_fetcher mlxd_file_fetcher_new_() {
  return mlxd_file_fetcher({nullptr});
}

inline mlxd_file_fetcher mlxd_file_fetcher_new_(
    const std::shared_ptr<mlx::data::core::FileFetcher>& s) {
  return mlxd_file_fetcher(
      {new std::shared_ptr<mlx::data::core::FileFetcher>(s)});
}

inline mlxd_file_fetcher& mlxd_file_fetcher_set_(
    mlxd_file_fetcher& d,
    const std::shared_ptr<mlx::data::core::FileFetcher>& s) {
  if (d.ctx) {
    *static_cast<std::shared_ptr<mlx::data::core::FileFetcher>*>(d.ctx) = s;
  } else {
    d.ctx = new std::shared_ptr<mlx::data::core::FileFetcher>(s);
  }
  return d;
}

inline std::shared_ptr<mlx::data::core::FileFetcher>& mlxd_file_fetcher_get_(
    mlxd_file_fetcher d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_file_fetcher");
  }
  return *static_cast<std::shared_ptr<mlx::data::core::FileFetcher>*>(d.ctx);
}

inline void mlxd_file_fetcher_free_(mlxd_file_fetcher d) {
  if (d.ctx) {
    delete static_cast<std::shared_ptr<mlx::data::core::FileFetcher>*>(d.ctx);
  }
}

#endif
