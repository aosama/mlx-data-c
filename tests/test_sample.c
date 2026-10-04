#include <string.h>

#include "test_util.h"

/* Managed-view dtor for a buffer that outlives the handle: a no-op. */
static void no_dtor(void* ctx) {
  (void)ctx;
}

int main(void) {
  mlxd_set_error_handler(mlxd_test_quiet_handler, NULL, NULL);

  mlxd_sample sample = mlxd_sample_new();

  /* --- set / get / share semantics --- */
  mlxd_array v = mlxd_array_new_scalar_int64(5);
  TEST_CHECK(mlxd_sample_set_key(sample, "x", v) == 0);
  mlxd_array r = mlxd_array_new();
  TEST_CHECK(mlxd_sample_get(&r, sample, "x") == 0);
  int64_t x = 0;
  TEST_CHECK(mlxd_array_item_int64(&x, r) == 0 && x == 5);

  /* shared buffer: writes through one view are visible in the other */
  int64_t vals[1] = {1};
  const int64_t shape[1] = {1};
  mlxd_array w = mlxd_array_new_data_managed(
      MLXD_INT64, shape, 1, vals, no_dtor, vals);
  TEST_CHECK(mlxd_sample_set_key(sample, "y", w) == 0);
  vals[0] = 77;
  mlxd_array y = mlxd_array_new();
  TEST_CHECK(mlxd_sample_get(&y, sample, "y") == 0);
  TEST_CHECK(mlxd_array_item_int64(&x, y) == 0 && x == 77);

  /* --- size, keys, erase --- */
  int64_t nsize = 0;
  TEST_CHECK(mlxd_sample_size(&nsize, sample) == 0 && nsize == 2);
  mlxd_vector_string keys = mlxd_vector_string_new();
  TEST_CHECK(mlxd_sample_keys(&keys, sample) == 0);
  size_t nkeys = 0;
  mlxd_vector_string_size(&nkeys, keys);
  TEST_CHECK(nkeys == 2);
  const char* first = NULL;
  TEST_CHECK(mlxd_vector_string_get(&first, keys, 0) == 0);
  TEST_CHECK(first != NULL &&
             (strcmp(first, "x") == 0 || strcmp(first, "y") == 0));

  /* rename = set under the new key, then erase the old one */
  TEST_CHECK(mlxd_sample_set_key(sample, "z", w) == 0);
  TEST_CHECK(mlxd_sample_erase(sample, "y") == 0);
  TEST_CHECK(mlxd_sample_get(&y, sample, "y") == 2); /* absent -> 2 */
  TEST_CHECK(mlxd_sample_get(&y, sample, "z") == 0);
  TEST_CHECK(mlxd_sample_erase(sample, "z") == 0);
  TEST_CHECK(mlxd_sample_get(&y, sample, "z") == 2);
  TEST_CHECK(mlxd_sample_size(&nsize, sample) == 0 && nsize == 1);

  /* --- end-of-stream contract: an empty sample carries no keys --- */
  mlxd_sample fresh = mlxd_sample_new();
  mlxd_vector_string fkeys = mlxd_vector_string_new();
  TEST_CHECK(mlxd_sample_keys(&fkeys, fresh) == 0);
  nkeys = 99;
  mlxd_vector_string_size(&nkeys, fkeys);
  TEST_CHECK(nkeys == 0);

  /* NULL / empty-handle guards */
  TEST_CHECK(mlxd_sample_set_key(sample, NULL, v) == 1);
  TEST_CHECK(mlxd_sample_get(&y, sample, NULL) == 1);
  mlxd_sample null_sample = {NULL};
  TEST_CHECK(mlxd_sample_keys(&fkeys, null_sample) == 1);
  TEST_CHECK(mlxd_sample_free(null_sample) == 0);

  mlxd_vector_string_free(fkeys);
  mlxd_vector_string_free(keys);
  mlxd_sample_free(fresh);
  mlxd_array_free(y);
  mlxd_array_free(w);
  mlxd_array_free(r);
  mlxd_array_free(v);
  mlxd_sample_free(sample);

  printf("test_sample passed\n");
  return 0;
}
