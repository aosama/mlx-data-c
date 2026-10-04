#!/bin/sh
# Consumer smoke test for mlx-data-c.
#
# Installs the library into a scratch prefix (outside the build tree),
# then builds and runs a pure-C consumer project that uses ONLY
# find_package(MLXDC) + target_link_libraries(... mlxdc), calling one
# function from every module family.
#
# Usage: scripts/consumer_smoke_test.sh [build-dir]
#   build-dir: an existing configured build directory (default: build)

set -e

SOURCE_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR=${1:-"$SOURCE_DIR/build"}
WORK=$(mktemp -d /tmp/mlxdc_consumer.XXXXXX)
trap 'rm -rf "$WORK"' EXIT

PREFIX="$WORK/prefix"
cmake --install "$BUILD_DIR" --prefix "$PREFIX" > /dev/null

mkdir -p "$WORK/consumer"
cat > "$WORK/consumer/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.16)
project(consumer LANGUAGES C)
find_package(MLXDC REQUIRED)
add_executable(consumer main.c)
target_link_libraries(consumer PRIVATE mlxdc)
EOF

cat > "$WORK/consumer/main.c" <<'EOF'
#include <stdio.h>
#include <string.h>

#include "mlx/data/c/mlx-data.h"

static mlxd_array double_it(mlxd_array input, void* ctx) {
  int64_t* calls = (int64_t*)ctx;
  (*calls)++;
  int64_t v = 0;
  if (mlxd_array_item_int64(&v, input) != 0) return mlxd_array_new();
  return mlxd_array_new_scalar_int64(v * 2);
}

/* One call per module family, through find_package(MLXDC) only. */
int main(void) {
  mlxd_set_error_handler(NULL, NULL, NULL); /* error */

  mlxd_string version = mlxd_string_new(""); /* string */
  if (mlxd_version(&version) != 0) return 1; /* version */

  const int64_t shape[1] = {1};
  int64_t twenty_one = 21;
  mlxd_array a = mlxd_array_new_data(MLXD_INT64, shape, 1, /* array */
                                     &twenty_one);
  int64_t v = 0;
  if (mlxd_array_item_int64(&v, a) != 0 || v != 21) return 2;

  mlxd_sample s = mlxd_sample_new(); /* sample */
  if (mlxd_sample_set_key(s, "x", a) != 0) return 3;

  mlxd_buffer buffer = mlxd_buffer_new(); /* buffer */
  mlxd_sample one[1] = {s};
  if (mlxd_buffer_from_samples(&buffer, one, 1) != 0) return 4;
  mlxd_buffer squeezed = mlxd_buffer_new(); /* ops (buffer side) */
  if (mlxd_buffer_squeeze(&squeezed, buffer, "x", "xs") != 0) return 5;

  mlxd_stream stream = mlxd_stream_new(); /* stream */
  if (mlxd_buffer_to_stream(&stream, buffer) != 0) return 6;
  mlxd_stream squeezed_stream = mlxd_stream_new(); /* ops (stream side) */
  if (mlxd_stream_squeeze(&squeezed_stream, stream, "x", "xs") != 0) {
    return 7;
  }

  mlxd_trie trie = mlxd_trie_new(); /* tokenizer machinery */
  if (mlxd_trie_insert(trie, "hi", 0) != 0) return 8;
  mlxd_tokenizer tokenizer = mlxd_tokenizer_new(trie, true, NULL, 0);
  mlxd_graph graph = mlxd_graph_new();
  if (mlxd_tokenizer_tokenize(&graph, tokenizer, "hi") != 0) return 9;
  mlxd_vector_int64 ids = mlxd_vector_int64_new();
  if (mlxd_tokenizer_tokenize_shortest(&ids, tokenizer, "hi") != 0) return 10;

  /* closure + misc core + file fetcher */
  int64_t calls = 0;
  mlxd_closure_array closure;
  if (mlxd_closure_array_new(&closure, double_it, &calls, NULL) != 0) {
    return 11;
  }
  mlxd_buffer doubled = mlxd_buffer_new();
  if (mlxd_buffer_key_transform(&doubled, buffer, "x", closure, "x2") != 0) {
    return 12;
  }
  mlxd_sample row = mlxd_sample_new();
  mlxd_array x2 = mlxd_array_new();
  if (mlxd_buffer_get(&row, doubled, 0) != 0) return 13;
  if (mlxd_sample_get(&x2, row, "x2") != 0) return 14;
  if (mlxd_array_item_int64(&v, x2) != 0 || v != 42) return 15;
  if (mlxd_set_state(1234) != 0) return 16; /* misc core */
  mlxd_aws_file_fetcher_options opts = /* file fetcher */
      mlxd_aws_file_fetcher_options_default();
  if (opts.num_threads != 4) return 17;
  if (calls == 0) return 18;

  const char* version_cstr = NULL;
  if (mlxd_string_c_str(&version_cstr, version) != 0) return 19;
  printf("consumer smoke test passed (mlx-data %s)\n", version_cstr);

  mlxd_array_free(x2);
  mlxd_sample_free(row);
  mlxd_vector_int64_free(ids);
  mlxd_graph_free(graph);
  mlxd_tokenizer_free(tokenizer);
  mlxd_trie_free(trie);
  mlxd_stream_free(squeezed_stream);
  mlxd_stream_free(stream);
  mlxd_buffer_free(squeezed);
  mlxd_buffer_free(doubled);
  mlxd_buffer_free(buffer);
  mlxd_sample_free(s);
  mlxd_array_free(a);
  mlxd_string_free(version);
  return 0;
}
EOF

echo "Configuring consumer project against installed MLXDC..."
cmake -S "$WORK/consumer" -B "$WORK/consumer/build" \
  -DCMAKE_PREFIX_PATH="$PREFIX" > /dev/null
cmake --build "$WORK/consumer/build" > /dev/null
"$WORK/consumer/build/consumer"
