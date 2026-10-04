#ifndef MLXD_STRING_PRIVATE_H
#define MLXD_STRING_PRIVATE_H

#include <string>

#include "mlx/data/c/string.h"

inline mlxd_string mlxd_string_new_() {
  return mlxd_string({nullptr});
}

inline mlxd_string mlxd_string_new_(const std::string& s) {
  return mlxd_string({new std::string(s)});
}

inline mlxd_string mlxd_string_new_(std::string&& s) {
  return mlxd_string({new std::string(std::move(s))});
}

inline mlxd_string& mlxd_string_set_(mlxd_string& d, const std::string& s) {
  if (d.ctx) {
    *static_cast<std::string*>(d.ctx) = s;
  } else {
    d.ctx = new std::string(s);
  }
  return d;
}

inline mlxd_string& mlxd_string_set_(mlxd_string& d, std::string&& s) {
  if (d.ctx) {
    *static_cast<std::string*>(d.ctx) = std::move(s);
  } else {
    d.ctx = new std::string(std::move(s));
  }
  return d;
}

inline std::string& mlxd_string_get_(mlxd_string d) {
  if (!d.ctx) {
    throw std::runtime_error("expected a non-empty mlxd_string");
  }
  return *static_cast<std::string*>(d.ctx);
}

inline void mlxd_string_free_(mlxd_string d) {
  if (d.ctx) {
    delete static_cast<std::string*>(d.ctx);
  }
}

#endif
