/* Read an embedded CSV as a stream, apply a per-sample op chain, collect
 * the rows into a buffer, and drain the stream twice to demonstrate
 * reset + the end-of-stream empty-sample contract. Exits non-zero on any
 * mismatch. */

#include <stdio.h>
#include <string.h>

#include "mlx/data/c/mlx-data.h"

static const char* kCsv = "word,count\nant,3\nbee,2\ncat,5\n";

int main(void) {
  mlxd_stream csv = mlxd_stream_new();
  if (mlxd_stream_csv_reader_from_string(&csv, kCsv, ',', '"') != 0) {
    return 1;
  }

  /* Per-sample op chain: rename "count" to "n", then record the word
   * length under "len". */
  mlxd_stream renamed = mlxd_stream_new();
  if (mlxd_stream_rename_key(&renamed, csv, "count", "n") != 0) {
    return 2;
  }
  mlxd_stream with_len = mlxd_stream_new();
  if (mlxd_stream_shape_dim(&with_len, renamed, "word", 0, "len") != 0) {
    return 3;
  }

  /* First drain: CSV cells are raw bytes; verify row contents. */
  const char* words[3] = {"ant", "bee", "cat"};
  const int64_t counts[3] = {3, 2, 5};
  const int64_t lens[3] = {3, 3, 3};
  int64_t rows = 0;
  for (;;) {
    mlxd_sample row = mlxd_sample_new();
    if (mlxd_stream_next(&row, with_len) != 0) {
      return 4;
    }
    int64_t n_keys = 0;
    mlxd_sample_size(&n_keys, row);
    if (n_keys == 0) {
      /* END-OF-STREAM CONTRACT: an empty sample means exhausted. */
      mlxd_sample_free(row);
      break;
    }
    if (rows >= 3) {
      return 5;
    }
    mlxd_array cell = mlxd_array_new();

    const void* data = NULL;
    int64_t size = 0;
    if (mlxd_sample_get(&cell, row, "word") != 0) {
      return 6;
    }
    mlxd_array_data(&data, cell);
    mlxd_array_size(&size, cell);
    if (size != (int64_t)strlen(words[rows]) ||
        memcmp(data, words[rows], (size_t)size) != 0) {
      return 7;
    }

    int64_t value = -1;
    if (mlxd_sample_get(&cell, row, "n") != 0) {
      return 8;
    }
    /* CSV cells stay raw bytes in the C++ core; compare as text. */
    {
      const void* n_data = NULL;
      int64_t n_size = 0;
      mlxd_array_data(&n_data, cell);
      mlxd_array_size(&n_size, cell);
      char count_byte = (char)('0' + counts[rows]);
      if (n_size != 1 || *(const char*)n_data != count_byte) {
        return 9;
      }
    }

    if (mlxd_sample_get(&cell, row, "len") != 0) {
      return 10;
    }
    mlxd_array_item_int64(&value, cell);
    if (value != lens[rows]) {
      return 11;
    }

    printf("row %lld: %s n=%lld len=%lld\n", (long long)rows, words[rows],
           (long long)counts[rows], (long long)lens[rows]);
    mlxd_array_free(cell);
    mlxd_sample_free(row);
    rows++;
  }
  if (rows != 3) {
    return 12;
  }

  /* Second drain without reset: still exhausted. */
  mlxd_sample row = mlxd_sample_new();
  if (mlxd_stream_next(&row, with_len) != 0) {
    return 13;
  }
  int64_t n_keys = 0;
  mlxd_sample_size(&n_keys, row);
  mlxd_sample_free(row);
  if (n_keys != 0) {
    return 14;
  }

  /* Collect the same stream family into a buffer via to_buffer after
   * reset, proving streams are resettable. */
  if (mlxd_stream_reset(with_len) != 0) {
    return 15;
  }
  mlxd_buffer collected = mlxd_buffer_new();
  if (mlxd_stream_to_buffer(&collected, with_len) != 0) {
    return 16;
  }
  int64_t collected_size = 0;
  if (mlxd_buffer_size(&collected_size, collected) != 0 ||
      collected_size != 3) {
    return 17;
  }

  mlxd_buffer_free(collected);
  mlxd_stream_free(with_len);
  mlxd_stream_free(renamed);
  mlxd_stream_free(csv);

  printf("example_stream: OK\n");
  return 0;
}
