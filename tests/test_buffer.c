#include <stdio.h>
#include <string.h>

#include "test_util.h"

/* Builds a 4-sample buffer of {"label": i, "text": "abcd"[i]}. */
static mlxd_buffer make_labeled(void) {
  mlxd_sample* samples = malloc(4 * sizeof(mlxd_sample));
  for (int64_t i = 0; i < 4; i++) {
    samples[i] = mlxd_sample_new();
    mlxd_array label = mlxd_array_new_scalar_int64(i);
    char text[2] = {(char)('a' + i), 0};
    mlxd_array t = mlxd_array_new_string(text);
    mlxd_sample_set_key(samples[i], "label", label);
    mlxd_sample_set_key(samples[i], "text", t);
    mlxd_array_free(t);
    mlxd_array_free(label);
  }
  mlxd_buffer buffer = mlxd_buffer_new();
  TEST_CHECK(mlxd_buffer_from_samples(&buffer, samples, 4) == 0);
  for (int64_t i = 0; i < 4; i++) mlxd_sample_free(samples[i]);
  free(samples);
  return buffer;
}

int main(void) {
  mlxd_set_error_handler(mlxd_test_quiet_handler, NULL, NULL);

  mlxd_buffer source = make_labeled();

  /* --- indexing and bounds --- */
  int64_t size = 0;
  mlxd_buffer_size(&size, source);
  TEST_CHECK(size == 4);
  mlxd_sample row = mlxd_sample_new();
  mlxd_buffer_get(&row, source, 2);
  mlxd_array label = mlxd_array_new();
  mlxd_sample_get(&label, row, "label");
  int64_t v = 0;
  mlxd_array_item_int64(&v, label);
  TEST_CHECK(v == 2);
  TEST_CHECK(mlxd_buffer_get(&row, source, 9) == 2); /* out of bounds -> 2 */

  /* --- batch with a padded scalar key --- */
  const char* pad_keys[1] = {"label"};
  double pad_values[1] = {0.0};
  mlxd_buffer batched = mlxd_buffer_new();
  TEST_CHECK(mlxd_buffer_batch(
                 &batched, source, 2, pad_keys, pad_values, 1, NULL, NULL, 0) ==
             0);
  int64_t bsize = 0;
  mlxd_buffer_size(&bsize, batched);
  TEST_CHECK(bsize == 2);
  mlxd_buffer_get(&row, batched, 1);
  mlxd_array blabel = mlxd_array_new();
  mlxd_sample_get(&blabel, row, "label");
  const int64_t* bshape = NULL;
  size_t bndim = 0;
  mlxd_array_shape(&bshape, &bndim, blabel);
  TEST_CHECK(bndim == 1 && bshape[0] == 2); /* {2,3} stacked -> shape {2} */

  /* --- to_stream / to_buffer round trip preserves the samples --- */
  mlxd_stream stream = mlxd_stream_new();
  TEST_CHECK(mlxd_buffer_to_stream(&stream, source) == 0);
  mlxd_buffer roundtrip = mlxd_buffer_new();
  TEST_CHECK(mlxd_stream_to_buffer(&roundtrip, stream) == 0);
  int64_t rsize = 0;
  mlxd_buffer_size(&rsize, roundtrip);
  TEST_CHECK(rsize == 4);

  /* --- perm reorders without dropping --- */
  const int64_t perm[4] = {3, 1, 0, 2};
  mlxd_buffer permuted = mlxd_buffer_new();
  TEST_CHECK(mlxd_buffer_perm(&permuted, source, perm, 4) == 0);
  mlxd_buffer_get(&row, permuted, 0);
  mlxd_array plabel = mlxd_array_new();
  mlxd_sample_get(&plabel, row, "label");
  mlxd_array_item_int64(&v, plabel);
  TEST_CHECK(v == 3);

  /* --- tar loading with the checked-in fixture --- */
  char tarpath[512];
  snprintf(tarpath, sizeof(tarpath), "%s/tiny.tar", MLXD_FIXTURES_DIR);
  mlxd_buffer tar_buffer = mlxd_buffer_new();
  TEST_CHECK(mlxd_buffer_files_from_tar(&tar_buffer, tarpath, false, 1) == 0);
  int64_t tsize = 0;
  mlxd_buffer_size(&tsize, tar_buffer);
  TEST_CHECK(tsize == 2);
  /* "file" carries the file NAME; contents load via read_from_tar */
  mlxd_buffer_get(&row, tar_buffer, 0);
  mlxd_array fname = mlxd_array_new();
  TEST_CHECK(mlxd_sample_get(&fname, row, "file") == 0);
  int64_t fsize = 0;
  mlxd_array_size(&fsize, fname);
  const void* fdata = NULL;
  mlxd_array_data(&fdata, fname);
  /* tar order: two.txt comes first here; contents load via read_from_tar */
  TEST_CHECK(fsize == 7);
  TEST_CHECK(memcmp(fdata, "one.txt", 7) == 0 ||
             memcmp(fdata, "two.txt", 7) == 0);

  /* --- read_from_tar as an op: from_key=true reads the path from the
   * sample's tarkey ("tar"); ikey names the file inside the archive --- */
  mlxd_sample tar_row = mlxd_sample_new();
  mlxd_array tar_name = mlxd_array_new_string(tarpath);
  mlxd_array inner = mlxd_array_new_string("one.txt");
  mlxd_sample_set_key(tar_row, "tar", tar_name);
  mlxd_sample_set_key(tar_row, "name", inner);
  mlxd_sample* tar_samples = malloc(sizeof(mlxd_sample));
  tar_samples[0] = tar_row;
  mlxd_buffer tar_dataset = mlxd_buffer_new();
  TEST_CHECK(mlxd_buffer_from_samples(&tar_dataset, tar_samples, 1) == 0);
  mlxd_sample_free(tar_row);
  free(tar_samples);
  mlxd_buffer tar_read = mlxd_buffer_new();
  TEST_CHECK(mlxd_buffer_read_from_tar(
                 &tar_read, tar_dataset, "tar", "name", "bytes", "", "",
                 true, false, 1) == 0);
  mlxd_buffer_get(&row, tar_read, 0);
  mlxd_array bytes = mlxd_array_new();
  TEST_CHECK(mlxd_sample_get(&bytes, row, "bytes") == 0);
  int64_t bsz = 0;
  mlxd_array_size(&bsz, bytes);
  const void* bdata = NULL;
  mlxd_array_data(&bdata, bytes);
  TEST_CHECK(bsz == 10 && memcmp(bdata, "hello tar\n", 10) == 0);

  mlxd_array_free(bytes);
  mlxd_buffer_free(tar_read);
  mlxd_buffer_free(tar_dataset);
  mlxd_array_free(inner);
  mlxd_array_free(tar_name);

  mlxd_array_free(fname);
  mlxd_array_free(plabel);
  mlxd_array_free(blabel);
  mlxd_array_free(label);
  mlxd_sample_free(row);
  mlxd_buffer_free(tar_buffer);
  mlxd_buffer_free(permuted);
  mlxd_buffer_free(roundtrip);
  mlxd_stream_free(stream);
  mlxd_buffer_free(batched);
  mlxd_buffer_free(source);

  printf("test_buffer passed\n");
  return 0;
}
