#include "mlx/data/c/trie.h"

#include "mlx/data/core/Trie.h"
#include "mlx/data/c/error.h"
#include "mlx/data/c/private/string.h"
#include "mlx/data/c/private/trie.h"

extern "C" mlxd_trie mlxd_trie_new(void) {
  try {
    return mlxd_trie_new_(std::make_shared<mlxd_trie_cpp>());
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return mlxd_trie_new_();
  }
}

extern "C" int mlxd_trie_free(mlxd_trie trie) {
  try {
    mlxd_trie_free_(trie);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_trie_insert(mlxd_trie trie, const char* key, int64_t id) {
  try {
    if (key == nullptr) {
      throw std::invalid_argument("mlxd_trie_insert: key is NULL");
    }
    mlxd_trie_get_(trie)->insert(std::string(key), id);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_trie_search(int64_t* out, mlxd_trie trie, const char* key) {
  try {
    if (key == nullptr) {
      throw std::invalid_argument("mlxd_trie_search: key is NULL");
    }
    auto node = mlxd_trie_get_(trie)->search(std::string(key));
    if (node == nullptr) {
      return 2;
    }
    *out = node->id;
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int mlxd_trie_num_keys(int64_t* out, mlxd_trie trie) {
  try {
    *out = mlxd_trie_get_(trie)->num_keys();
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int
mlxd_trie_key_string(mlxd_string* out, mlxd_trie trie, int64_t id) {
  try {
    const std::vector<char>& key = mlxd_trie_get_(trie)->key(id);
    mlxd_string_set_(
        *out, std::string(key.begin(), key.end()));
  } catch (std::out_of_range&) {
    return 2;
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}
