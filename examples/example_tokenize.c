/* Tokenize a sentence two ways: greedy shortest path through a trie, and
 * the buffer-level tokenize op. Prints the token id sequences. Exits
 * non-zero on any mismatch with the hand-computed expectations. */

#include <stdio.h>
#include <string.h>

#include "mlx/data/c/mlx-data.h"

/* Trie: "ab"=0, "c"=1, "abc"=2. "abc" therefore tokenizes either as
 * [2] or as [0, 1]; the shortest path is [2]. */

static int check_ids(mlxd_vector_int64 ids, const int64_t* expected,
                     size_t n) {
  size_t size = 0;
  if (mlxd_vector_int64_size(&size, ids) != 0 || size != n) {
    return 0;
  }
  for (size_t i = 0; i < n; i++) {
    int64_t value = -1;
    if (mlxd_vector_int64_get(&value, ids, i) != 0 || value != expected[i]) {
      return 0;
    }
  }
  return 1;
}

int main(void) {
  mlxd_trie trie = mlxd_trie_new();
  mlxd_trie_insert(trie, "ab", 0);
  mlxd_trie_insert(trie, "c", 1);
  mlxd_trie_insert(trie, "abc", 2);

  /* --- greedy shortest tokenization of a sentence --- */
  mlxd_tokenizer tokenizer = mlxd_tokenizer_new(trie, false, NULL, 0);
  mlxd_vector_int64 ids = mlxd_vector_int64_new();
  if (mlxd_tokenizer_tokenize_shortest(&ids, tokenizer, "abcabc") != 0) {
    return 1;
  }
  const int64_t expected[] = {2, 2};
  if (!check_ids(ids, expected, 2)) {
    return 2;
  }
  printf("shortest abcabc: %lld %lld\n", 2LL, 2LL);

  /* --- the buffer-level tokenize op, with ids as Int64 arrays --- */
  const char* texts[] = {"abc", "ababc"};
  mlxd_sample samples[2];
  for (int i = 0; i < 2; i++) {
    samples[i] = mlxd_sample_new();
    mlxd_array text = mlxd_array_new_string(texts[i]);
    mlxd_sample_set_key(samples[i], "text", text);
    mlxd_array_free(text);
  }
  mlxd_buffer buffer = mlxd_buffer_new();
  if (mlxd_buffer_from_samples(&buffer, samples, 2) != 0) {
    return 3;
  }
  for (int i = 0; i < 2; i++) {
    mlxd_sample_free(samples[i]);
  }

  mlxd_buffer tokenized = mlxd_buffer_new();
  if (mlxd_buffer_tokenize(&tokenized, buffer, "text", trie,
                           MLXD_TOKENIZE_SHORTEST, false, NULL, 0, "ids") != 0) {
    return 4;
  }

  const int64_t expected_ababc[] = {0, 2}; /* "ab" + "abc" */
  const int64_t expected_lengths[2] = {1, 2}; /* "abc"=[2], "ababc"=[0,2] */
  for (int i = 0; i < 2; i++) {
    mlxd_sample row = mlxd_sample_new();
    if (mlxd_buffer_get(&row, tokenized, i) != 0) {
      return 5;
    }
    mlxd_array ids_arr = mlxd_array_new();
    if (mlxd_sample_get(&ids_arr, row, "ids") != 0) {
      return 6;
    }
    int64_t n = 0;
    const void* data = NULL;
    if (mlxd_array_size(&n, ids_arr) != 0 || mlxd_array_data(&data, ids_arr) != 0) {
      return 7;
    }
    const int64_t* got = (const int64_t*)data;
    const int64_t* want = (i == 0) ? expected : expected_ababc;
    if (n != expected_lengths[i]) {
      return 8;
    }
    for (int64_t k = 0; k < n; k++) {
      if (got[k] != want[k]) {
        return 8;
      }
    }
    printf("tokenize %s:", texts[i]);
    for (int64_t k = 0; k < n; k++) {
      printf(" %lld", (long long)got[k]);
    }
    printf("\n");
    mlxd_array_free(ids_arr);
    mlxd_sample_free(row);
  }

  /* --- BPE: symbol trie + merge list, "abc" merges to one token --- */
  mlxd_trie symbols = mlxd_trie_new();
  mlxd_trie_insert(symbols, "a", 0);
  mlxd_trie_insert(symbols, "b", 1);
  mlxd_trie_insert(symbols, "ab", 2);
  mlxd_trie_insert(symbols, "c", 3);
  mlxd_trie_insert(symbols, "abc", 4);
  mlxd_bpe_merges merges = mlxd_bpe_merges_new();
  mlxd_bpe_merges_add(merges, "a", "b", 2);
  mlxd_bpe_merges_add(merges, "ab", "c", 4);
  mlxd_bpe_tokenizer bpe = mlxd_bpe_tokenizer_new(symbols, merges);
  mlxd_vector_int64 bpe_ids = mlxd_vector_int64_new();
  if (mlxd_bpe_tokenizer_tokenize(&bpe_ids, bpe, "abc") != 0) {
    return 9;
  }
  const int64_t expected_bpe[] = {4};
  if (!check_ids(bpe_ids, expected_bpe, 1)) {
    return 10;
  }
  printf("bpe abc: 4\n");

  mlxd_vector_int64_free(bpe_ids);
  mlxd_bpe_tokenizer_free(bpe);
  mlxd_bpe_merges_free(merges);
  mlxd_trie_free(symbols);

  mlxd_vector_int64_free(ids);
  mlxd_tokenizer_free(tokenizer);
  mlxd_buffer_free(tokenized);
  mlxd_buffer_free(buffer);
  mlxd_trie_free(trie);

  printf("example_tokenize: OK\n");
  return 0;
}
