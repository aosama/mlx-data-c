/* Build three samples of unequal length, batch them with padding, and read
 * the batch back through the C API. Exits non-zero on any mismatch. */

#include <stdio.h>
#include <string.h>

#include "mlx/data/c/mlx-data.h"

int main(void) {
  /* Three samples: a bytes key and an int64 label each. Lengths 4, 2, 3
   * so batch padding is observable. */
  const char* words[3] = {"bear", "ox", "cat"};
  mlxd_sample samples[3];
  for (int i = 0; i < 3; i++) {
    samples[i] = mlxd_sample_new();
    mlxd_array word = mlxd_array_new_string(words[i]);
    mlxd_array label = mlxd_array_new_scalar_int64(i);
    mlxd_sample_set_key(samples[i], "word", word);
    mlxd_sample_set_key(samples[i], "label", label);
    mlxd_array_free(label);
    mlxd_array_free(word);
  }

  mlxd_buffer buffer = mlxd_buffer_new();
  if (mlxd_buffer_from_samples(&buffer, samples, 3) != 0) {
    return 1;
  }
  for (int i = 0; i < 3; i++) {
    mlxd_sample_free(samples[i]);
  }

  int64_t size = 0;
  if (mlxd_buffer_size(&size, buffer) != 0 || size != 3) {
    return 2;
  }

  /* One batch of all three samples; "word" padded with 0. */
  mlxd_buffer batched = mlxd_buffer_new();
  const char* pad_keys[] = {"word"};
  const double pad_values[] = {0.0};
  if (mlxd_buffer_batch(&batched, buffer, 3, pad_keys, pad_values, 1, NULL,
                        NULL, 0) != 0) {
    return 3;
  }

  mlxd_sample row = mlxd_sample_new();
  if (mlxd_buffer_get(&row, batched, 0) != 0) {
    return 4;
  }

  mlxd_array words_arr = mlxd_array_new();
  if (mlxd_sample_get(&words_arr, row, "word") != 0) {
    return 5;
  }
  const int64_t* shape = NULL;
  size_t ndim = 0;
  int64_t itemsize = 0;
  mlxd_array_type type = MLXD_ANY;
  if (mlxd_array_shape(&shape, &ndim, words_arr) != 0 || ndim != 2 ||
      shape[0] != 3 || shape[1] != 4) {
    return 6;
  }
  if (mlxd_array_get_type(&type, words_arr) != 0 || type != MLXD_INT8) {
    return 7;
  }
  if (mlxd_array_itemsize(&itemsize, words_arr) != 0 || itemsize != 1) {
    return 8;
  }

  const void* data = NULL;
  if (mlxd_array_data(&data, words_arr) != 0) {
    return 9;
  }
  const char* expected = "bearox\0\0cat\0"; /* rows: bear | ox\0\0 | cat\0 */
  if (memcmp(data, expected, 12) != 0) {
    return 10;
  }
  printf("batched words: %.*s | %.*s | %.*s\n", 4, (const char*)data + 0, 4,
         (const char*)data + 4, 4, (const char*)data + 8);

  mlxd_array labels_arr = mlxd_array_new();
  if (mlxd_sample_get(&labels_arr, row, "label") != 0) {
    return 11;
  }
  const int64_t* labels = NULL;
  int64_t n_labels = 0;
  if (mlxd_array_data(&data, labels_arr) != 0) {
    return 12;
  }
  labels = (const int64_t*)data;
  if (mlxd_array_size(&n_labels, labels_arr) != 0) {
    return 12;
  }
  if (n_labels != 3 || labels[0] != 0 || labels[1] != 1 || labels[2] != 2) {
    return 13;
  }
  printf("labels: %lld %lld %lld\n", (long long)labels[0],
         (long long)labels[1], (long long)labels[2]);

  mlxd_array_free(labels_arr);
  mlxd_array_free(words_arr);
  mlxd_sample_free(row);
  mlxd_buffer_free(batched);
  mlxd_buffer_free(buffer);

  printf("example_buffer: OK\n");
  return 0;
}
