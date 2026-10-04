#include <stdlib.h>
#include <string.h>

#include "test_util.h"

/* C closure: doubles the input scalar, counting invocations in ctx. */
static mlxd_array double_it(mlxd_array input, void* ctx) {
  int64_t* calls = (int64_t*)ctx;
  (*calls)++;
  int64_t v = 0;
  mlxd_array_item_int64(&v, input);
  return mlxd_array_new_scalar_int64(v * 2);
}

/* 3 samples, each with:
 *   "s"    scalar int64 (i+1)          — for the closure transform
 *   "x"    int64[2] {i+1, i+2}         — for pad / slice
 *   "t"    string                      — for remove_value
 *   "tlen" int64[1] strlen("t")        — its size key
 */
static mlxd_buffer make_fixture(void) {
  const char* texts[3] = {"abxx", "kkzz", "hhqq"};
  mlxd_sample* samples = malloc(3 * sizeof(mlxd_sample));
  for (int64_t i = 0; i < 3; i++) {
    samples[i] = mlxd_sample_new();
    mlxd_array s = mlxd_array_new_scalar_int64(i + 1);
    int64_t xs[2] = {i + 1, i + 2};
    const int64_t xshape[1] = {2};
    mlxd_array x = mlxd_array_new_data(MLXD_INT64, xshape, 1, xs);
    mlxd_array t = mlxd_array_new_string(texts[i]);
    int64_t tl = (int64_t)strlen(texts[i]);
    const int64_t lshape[1] = {1};
    mlxd_array tlen = mlxd_array_new_data(MLXD_INT64, lshape, 1, &tl);
    mlxd_sample_set_key(samples[i], "s", s);
    mlxd_sample_set_key(samples[i], "x", x);
    mlxd_sample_set_key(samples[i], "t", t);
    mlxd_sample_set_key(samples[i], "tlen", tlen);
    mlxd_array_free(tlen);
    mlxd_array_free(t);
    mlxd_array_free(x);
    mlxd_array_free(s);
  }
  mlxd_buffer buffer = mlxd_buffer_new();
  TEST_CHECK(mlxd_buffer_from_samples(&buffer, samples, 3) == 0);
  for (int64_t i = 0; i < 3; i++) mlxd_sample_free(samples[i]);
  free(samples);
  return buffer;
}

int main(void) {
  mlxd_set_error_handler(mlxd_test_quiet_handler, NULL, NULL);

  mlxd_buffer buffer = make_fixture();
  mlxd_sample row = mlxd_sample_new();
  mlxd_array a = mlxd_array_new();
  int64_t v = 0;

  /* --- key_transform via a C closure (applied lazily per get) --- */
  int64_t calls = 0;
  mlxd_closure_array closure;
  TEST_CHECK(mlxd_closure_array_new(&closure, double_it, &calls, NULL) == 0);
  mlxd_buffer transformed = mlxd_buffer_new();
  TEST_CHECK(
      mlxd_buffer_key_transform(&transformed, buffer, "s", closure, "s2") ==
      0);
  TEST_CHECK(calls == 0); /* construction has no side effects */
  for (int64_t i = 0; i < 3; i++) {
    mlxd_buffer_get(&row, transformed, i);
    mlxd_sample_get(&a, row, "s2");
    mlxd_array_item_int64(&v, a);
    TEST_CHECK(v == 2 * (i + 1));
  }
  TEST_CHECK(calls == 3); /* exactly one transform per fetched sample */

  /* pipelines survive the closure handle being freed (shared state) */
  TEST_CHECK(mlxd_closure_array_free(closure) == 0);
  mlxd_buffer_get(&row, transformed, 0);
  mlxd_sample_get(&a, row, "s2");
  mlxd_array_item_int64(&v, a);
  TEST_CHECK(v == 2);

  /* --- pad: left-pad "x" by one 0 --- */
  mlxd_buffer padded = mlxd_buffer_new();
  TEST_CHECK(mlxd_buffer_pad(&padded, buffer, "x", 0, 1, 0, 0.0, "xp") == 0);
  mlxd_buffer_get(&row, padded, 1);
  mlxd_sample_get(&a, row, "xp");
  int64_t psize = 0;
  mlxd_array_size(&psize, a);
  const void* pdata = NULL;
  mlxd_array_data(&pdata, a);
  TEST_CHECK(psize == 3);
  TEST_CHECK(((const int64_t*)pdata)[0] == 0 &&
             ((const int64_t*)pdata)[1] == 2);

  /* --- slice: keep the first element --- */
  mlxd_buffer sliced = mlxd_buffer_new();
  TEST_CHECK(mlxd_buffer_slice(&sliced, buffer, "x", 0, 0, 1, "xs") == 0);
  mlxd_buffer_get(&row, sliced, 1);
  mlxd_sample_get(&a, row, "xs");
  mlxd_array_item_int64(&v, a);
  TEST_CHECK(v == 2);

  /* --- remove_value: strip trailing 'x' from "abxx", refill '.' --- */
  mlxd_buffer stripped = mlxd_buffer_new();
  TEST_CHECK(mlxd_buffer_remove_value(
                 &stripped, buffer, "t", "tlen", 0, (double)'x', (double)'.') ==
             0);
  mlxd_buffer_get(&row, stripped, 0);
  mlxd_sample_get(&a, row, "t");
  int64_t tsize = 0;
  mlxd_array_size(&tsize, a);
  const void* tdata = NULL;
  mlxd_array_data(&tdata, a);
  TEST_CHECK(tsize == 4 && memcmp(tdata, "ab..", 4) == 0);
  mlxd_sample_get(&a, row, "tlen");
  mlxd_array_item_int64(&v, a);
  TEST_CHECK(v == 2); /* new valid length */

  /* --- _if with cond=false leaves the output key absent --- */
  mlxd_buffer untouched = mlxd_buffer_new();
  TEST_CHECK(
      mlxd_buffer_pad_if(&untouched, buffer, false, "x", 0, 1, 0, 0.0, "xp") ==
      0);
  mlxd_buffer_get(&row, untouched, 0);
  TEST_CHECK(mlxd_sample_get(&a, row, "xp") == 2); /* never created */

  mlxd_array_free(a);
  mlxd_sample_free(row);
  mlxd_buffer_free(untouched);
  mlxd_buffer_free(stripped);
  mlxd_buffer_free(sliced);
  mlxd_buffer_free(padded);
  mlxd_buffer_free(transformed);
  mlxd_buffer_free(buffer);

  printf("test_ops passed\n");
  return 0;
}
