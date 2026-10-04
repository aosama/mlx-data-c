#include <stdlib.h>
#include <string.h>

#include "test_util.h"

/* Collects token ids until the iterator is exhausted (empty vector). */
static int64_t collect_ids(int64_t* ids, int64_t max, mlxd_graph graph) {
  mlxd_tokenizer_iterator iterator = mlxd_tokenizer_iterator_new(graph);
  int64_t n = 0;
  for (;;) {
    mlxd_vector_int64 batch = mlxd_vector_int64_new();
    TEST_CHECK(mlxd_tokenizer_iterator_next(&batch, iterator) == 0);
    size_t size = 0;
    mlxd_vector_int64_size(&size, batch);
    if (size == 0) {
      mlxd_vector_int64_free(batch);
      break;
    }
    TEST_CHECK(n + (int64_t)size <= max);
    const int64_t* data = NULL;
    size_t ndata = 0;
    mlxd_vector_int64_data(&data, &ndata, batch);
    for (size_t i = 0; i < size; i++) {
      ids[n++] = data[i];
    }
    mlxd_vector_int64_free(batch);
  }
  TEST_CHECK(mlxd_tokenizer_iterator_free(iterator) == 0);
  return n;
}

/* Joins the trie keys for `ids` into `out`. */
static void decode(
    char* out,
    size_t out_size,
    mlxd_trie trie,
    const int64_t* ids,
    int64_t n) {
  out[0] = 0;
  for (int64_t i = 0; i < n; i++) {
    mlxd_string key = mlxd_string_new("");
    TEST_CHECK(mlxd_trie_key_string(&key, trie, ids[i]) == 0);
    const char* ckey = NULL;
    TEST_CHECK(mlxd_string_c_str(&ckey, key) == 0);
    strncat(out, ckey, out_size - strlen(out) - 1);
    TEST_CHECK(mlxd_string_free(key) == 0);
  }
}

int main(void) {
  mlxd_set_error_handler(mlxd_test_quiet_handler, NULL, NULL);

  /* --- trie: insert / search / num_keys / key round trip --- */
  mlxd_trie trie = mlxd_trie_new();
  TEST_CHECK(mlxd_trie_insert(trie, "he", 0) == 0);
  TEST_CHECK(mlxd_trie_insert(trie, "hell", 1) == 0);
  TEST_CHECK(mlxd_trie_insert(trie, "hello", 2) == 0);
  int64_t nkeys = 0;
  TEST_CHECK(mlxd_trie_num_keys(&nkeys, trie) == 0 && nkeys == 3);

  /* search resolves to the id stored for the exact key; misses -> 2 */
  int64_t id = -1;
  TEST_CHECK(mlxd_trie_search(&id, trie, "hello") == 0 && id == 2);
  TEST_CHECK(mlxd_trie_search(&id, trie, "hell") == 0 && id == 1);
  TEST_CHECK(mlxd_trie_search(&id, trie, "he") == 0 && id == 0);
  TEST_CHECK(mlxd_trie_search(&id, trie, "zzz") == 2);

  mlxd_string key_back = mlxd_string_new("");
  TEST_CHECK(mlxd_trie_key_string(&key_back, trie, 2) == 0);
  const char* key_cstr = NULL;
  TEST_CHECK(mlxd_string_c_str(&key_cstr, key_back) == 0);
  TEST_CHECK(strcmp(key_cstr, "hello") == 0);

  /* --- tokenize + iterate: ids must decode back to the input --- */
  mlxd_tokenizer tokenizer = mlxd_tokenizer_new(trie, false, NULL, 0);
  mlxd_graph graph = mlxd_graph_new();
  TEST_CHECK(mlxd_tokenizer_tokenize(&graph, tokenizer, "hello") == 0);
  int64_t ids[64] = {0};
  int64_t n = collect_ids(ids, 64, graph);
  TEST_CHECK(n > 0);
  char decoded[64];
  decode(decoded, sizeof(decoded), trie, ids, n);
  TEST_CHECK(strcmp(decoded, "hello") == 0);

  /* --- shortest tokenization is deterministic --- */
  mlxd_vector_int64 shortest = mlxd_vector_int64_new();
  TEST_CHECK(mlxd_tokenizer_tokenize_shortest(&shortest, tokenizer, "hello") ==
             0);
  size_t ssize = 0;
  mlxd_vector_int64_size(&ssize, shortest);
  TEST_CHECK(ssize > 0);
  const int64_t* sdata = NULL;
  size_t sndata = 0;
  mlxd_vector_int64_data(&sdata, &sndata, shortest);
  char sdecoded[64];
  decode(sdecoded, sizeof(sdecoded), trie, sdata, (int64_t)ssize);
  TEST_CHECK(strcmp(sdecoded, "hello") == 0);

  /* --- untokenizable input rejected without ignore_unk --- */
  mlxd_graph bad = mlxd_graph_new();
  TEST_CHECK(mlxd_tokenizer_tokenize(&bad, tokenizer, "zzz") == 1);

  /* --- BPE: symbols + merges round trip --- */
  mlxd_trie symbols = mlxd_trie_new();
  TEST_CHECK(mlxd_trie_insert(symbols, "a", 0) == 0);
  TEST_CHECK(mlxd_trie_insert(symbols, "b", 1) == 0);
  TEST_CHECK(mlxd_trie_insert(symbols, "c", 2) == 0);
  TEST_CHECK(mlxd_trie_insert(symbols, "ab", 3) == 0);
  TEST_CHECK(mlxd_trie_insert(symbols, "abc", 4) == 0);
  mlxd_bpe_merges merges = mlxd_bpe_merges_new();
  TEST_CHECK(mlxd_bpe_merges_add(merges, "a", "b", 3) == 0);
  TEST_CHECK(mlxd_bpe_merges_add(merges, "ab", "c", 4) == 0);
  mlxd_bpe_tokenizer bpe = mlxd_bpe_tokenizer_new(symbols, merges);
  mlxd_vector_int64 bpe_ids = mlxd_vector_int64_new();
  TEST_CHECK(mlxd_bpe_tokenizer_tokenize(&bpe_ids, bpe, "abc") == 0);
  size_t bsize = 0;
  mlxd_vector_int64_size(&bsize, bpe_ids);
  TEST_CHECK(bsize == 1); /* fully merged to the single token "abc" */
  const int64_t* bdata = NULL;
  size_t bndata = 0;
  mlxd_vector_int64_data(&bdata, &bndata, bpe_ids);
  TEST_CHECK(bdata[0] == 4);

  mlxd_vector_int64_free(bpe_ids);
  mlxd_vector_int64_free(shortest);
  TEST_CHECK(mlxd_bpe_tokenizer_free(bpe) == 0);
  TEST_CHECK(mlxd_bpe_merges_free(merges) == 0);
  TEST_CHECK(mlxd_trie_free(symbols) == 0);
  TEST_CHECK(mlxd_graph_free(bad) == 0);
  TEST_CHECK(mlxd_graph_free(graph) == 0);
  TEST_CHECK(mlxd_tokenizer_free(tokenizer) == 0);
  TEST_CHECK(mlxd_string_free(key_back) == 0);
  TEST_CHECK(mlxd_trie_free(trie) == 0);

  printf("test_tokenizer passed\n");
  return 0;
}
