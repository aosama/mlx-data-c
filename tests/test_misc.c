#include <string.h>

#include "test_util.h"

static mlxd_buffer make_labeled(void) {
  mlxd_sample* samples = malloc(8 * sizeof(mlxd_sample));
  for (int64_t i = 0; i < 8; i++) {
    samples[i] = mlxd_sample_new();
    mlxd_array label = mlxd_array_new_scalar_int64(i);
    mlxd_sample_set_key(samples[i], "label", label);
    mlxd_array_free(label);
  }
  mlxd_buffer buffer = mlxd_buffer_new();
  TEST_CHECK(mlxd_buffer_from_samples(&buffer, samples, 8) == 0);
  for (int64_t i = 0; i < 8; i++) mlxd_sample_free(samples[i]);
  free(samples);
  return buffer;
}

/* Collects the label sequence of a shuffled buffer. */
static void labels(int64_t* out, int64_t n, mlxd_buffer buffer) {
  mlxd_sample row = mlxd_sample_new();
  mlxd_array label = mlxd_array_new();
  for (int64_t i = 0; i < n; i++) {
    TEST_CHECK(mlxd_buffer_get(&row, buffer, i) == 0);
    TEST_CHECK(mlxd_sample_get(&label, row, "label") == 0);
    TEST_CHECK(mlxd_array_item_int64(&out[i], label) == 0);
  }
  mlxd_array_free(label);
  mlxd_sample_free(row);
}

int main(void) {
  mlxd_set_error_handler(mlxd_test_quiet_handler, NULL, NULL);

  /* --- set_state makes shuffle reproducible --- */
  mlxd_buffer source = make_labeled();
  int64_t run1[8], run2[8], run3[8];
  TEST_CHECK(mlxd_set_state(1234) == 0);
  mlxd_buffer s1 = mlxd_buffer_new();
  TEST_CHECK(mlxd_buffer_shuffle(&s1, source) == 0);
  labels(run1, 8, s1);
  TEST_CHECK(mlxd_set_state(1234) == 0);
  mlxd_buffer s2 = mlxd_buffer_new();
  TEST_CHECK(mlxd_buffer_shuffle(&s2, source) == 0);
  labels(run2, 8, s2);
  TEST_CHECK(memcmp(run1, run2, sizeof(run1)) == 0); /* same seed: equal */

  TEST_CHECK(mlxd_set_state(42) == 0);
  mlxd_buffer s3 = mlxd_buffer_new();
  TEST_CHECK(mlxd_buffer_shuffle(&s3, source) == 0);
  labels(run3, 8, s3);
  mlxd_buffer_free(s3);
  mlxd_buffer_free(s2);
  mlxd_buffer_free(s1);
  mlxd_buffer_free(source);

  /* --- levenshtein: kitten/sitting -> counts summing to 3 --- */
  const int64_t shape1[1] = {1};
  mlxd_array kitten = mlxd_array_new_string("kitten");
  mlxd_array sitting = mlxd_array_new_string("sitting");
  int64_t klen_val = 6, slen_val = 7;
  mlxd_array klen = mlxd_array_new_data(MLXD_INT64, shape1, 1, &klen_val);
  mlxd_array slen = mlxd_array_new_data(MLXD_INT64, shape1, 1, &slen_val);
  mlxd_array counts = mlxd_array_new();
  TEST_CHECK(
      mlxd_levenshtein(&counts, kitten, klen, sitting, slen) == 0);
  const void* cdata = NULL;
  mlxd_array_data(&cdata, counts);
  const int64_t* c = (const int64_t*)cdata;
  TEST_CHECK(c[0] + c[1] + c[2] == 3);

  int64_t same_val = 6;
  mlxd_array same_len = mlxd_array_new_data(MLXD_INT64, shape1, 1, &same_val);
  TEST_CHECK(mlxd_levenshtein(&counts, kitten, klen, kitten, same_len) == 0);
  mlxd_array_data(&cdata, counts);
  c = (const int64_t*)cdata;
  TEST_CHECK(c[0] + c[1] + c[2] == 0);

  float flen_val = 6.0f;
  mlxd_array flen = mlxd_array_new_data(MLXD_FLOAT, shape1, 1, &flen_val);
  TEST_CHECK(mlxd_levenshtein(&counts, kitten, flen, sitting, slen) == 1);

  /* --- uniq: consecutive-dedup "aabbca" -> "abca" --- */
  int64_t src_shape[1] = {6};
  mlxd_array src = mlxd_array_new_data(MLXD_INT8, src_shape, 1, "aabbca");
  int64_t src_len_val = 6;
  mlxd_array src_len = mlxd_array_new_data(MLXD_INT64, shape1, 1, &src_len_val);
  mlxd_array dst = mlxd_array_new();
  mlxd_array dst_len = mlxd_array_new();
  TEST_CHECK(mlxd_uniq(&dst, &dst_len, src, src_len, 0, 0.0) == 0);
  const void* ddata = NULL;
  mlxd_array_data(&ddata, dst);
  int64_t new_len = 0;
  TEST_CHECK(mlxd_array_item_int64(&new_len, dst_len) == 0);
  TEST_CHECK(new_len == 4 && memcmp(ddata, "abca", 4) == 0);

  /* --- remove: strip trailing 'x' from "abxx", refill '.' --- */
  int64_t x_shape[1] = {4};
  mlxd_array xsrc = mlxd_array_new_data(MLXD_INT8, x_shape, 1, "abxx");
  int64_t x_len_val = 4;
  mlxd_array xlen = mlxd_array_new_data(MLXD_INT64, shape1, 1, &x_len_val);
  mlxd_array rdst = mlxd_array_new();
  mlxd_array rdst_len = mlxd_array_new();
  TEST_CHECK(
      mlxd_remove(&rdst, &rdst_len, xsrc, xlen, 0, (double)'x', (double)'.') ==
      0);
  const void* rdata = NULL;
  mlxd_array_data(&rdata, rdst);
  TEST_CHECK(mlxd_array_item_int64(&new_len, rdst_len) == 0);
  TEST_CHECK(new_len == 2 && memcmp(rdata, "ab..", 4) == 0);

  /* --- version strings are non-empty --- */
  mlxd_string version = mlxd_string_new("");
  mlxd_vector_string lib_names = mlxd_vector_string_new();
  mlxd_vector_string lib_versions = mlxd_vector_string_new();
  TEST_CHECK(mlxd_version(&version) == 0);
  const char* version_cstr = NULL;
  TEST_CHECK(mlxd_string_c_str(&version_cstr, version) == 0);
  TEST_CHECK(mlxd_libs_version(&lib_names, &lib_versions) == 0);
  TEST_CHECK(strlen(version_cstr) > 0);

  /* --- AWS fetcher in a non-AWS build: guarded status 1 --- */
  mlxd_file_fetcher aws = mlxd_file_fetcher_new();
  mlxd_aws_file_fetcher_options options = mlxd_aws_file_fetcher_options_default();
  if (mlxd_aws_file_fetcher_new(&aws, "bucket", &options) == 0) {
    /* AWS build: freeing the live handle also works */
    TEST_CHECK(mlxd_file_fetcher_free(aws) == 0);
  } else {
    TEST_CHECK(aws.ctx == NULL);
  }
  mlxd_file_fetcher empty = mlxd_file_fetcher_new();
  mlxd_stream stream = mlxd_stream_new();
  TEST_CHECK(
      mlxd_stream_line_reader_with_fetcher(&stream, "f.txt", "line", false, "",
                                          empty) == 1);

  mlxd_stream_free(stream);
  mlxd_file_fetcher_free(empty);
  mlxd_vector_string_free(lib_versions);
  mlxd_vector_string_free(lib_names);
  mlxd_string_free(version);
  mlxd_array_free(rdst_len);
  mlxd_array_free(rdst);
  mlxd_array_free(xlen);
  mlxd_array_free(xsrc);
  mlxd_array_free(dst_len);
  mlxd_array_free(dst);
  mlxd_array_free(src_len);
  mlxd_array_free(src);
  mlxd_array_free(flen);
  mlxd_array_free(same_len);
  mlxd_array_free(counts);
  mlxd_array_free(slen);
  mlxd_array_free(klen);
  mlxd_array_free(sitting);
  mlxd_array_free(kitten);

  printf("test_misc passed\n");
  return 0;
}
