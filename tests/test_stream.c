#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "test_util.h"

static char* make_temp_dir(void) {
  char tmpl[] = "/tmp/mlxdc_test_stream_XXXXXX";
  char* dir = mkdtemp(tmpl);
  TEST_CHECK(dir != NULL);
  return strdup(dir);
}

/* Drains a stream, recording how many samples it yielded. */
static int64_t drain(mlxd_stream stream) {
  mlxd_sample row = mlxd_sample_new();
  mlxd_vector_string keys = mlxd_vector_string_new();
  int64_t count = 0;
  for (;;) {
    size_t nkeys = 0;
    TEST_CHECK(mlxd_stream_next(&row, stream) == 0);
    TEST_CHECK(mlxd_sample_keys(&keys, row) == 0);
    mlxd_vector_string_size(&nkeys, keys);
    if (nkeys == 0) break; /* end-of-stream contract: empty sample */
    count++;
  }
  mlxd_vector_string_free(keys);
  mlxd_sample_free(row);
  return count;
}

int main(void) {
  mlxd_set_error_handler(mlxd_test_quiet_handler, NULL, NULL);

  char* dir = make_temp_dir();
  char csv_path[512], lines_path[512];
  snprintf(csv_path, sizeof(csv_path), "%s/data.csv", dir);
  snprintf(lines_path, sizeof(lines_path), "%s/lines.txt", dir);

  FILE* csv_file = fopen(csv_path, "w");
  TEST_CHECK(csv_file != NULL);
  fputs("a,b\n1,2\n3,4\n", csv_file);
  fclose(csv_file);
  FILE* lines_file = fopen(lines_path, "w");
  TEST_CHECK(lines_file != NULL);
  fputs("one\ntwo\nthree\n", lines_file);
  fclose(lines_file);

  /* --- csv reader: one sample per record, bytes for cells --- */
  mlxd_stream csv = mlxd_stream_new();
  TEST_CHECK(mlxd_stream_csv_reader(&csv, csv_path, ',', '"', "") == 0);
  mlxd_sample row = mlxd_sample_new();
  TEST_CHECK(mlxd_stream_next(&row, csv) == 0);
  mlxd_array cell = mlxd_array_new();
  TEST_CHECK(mlxd_sample_get(&cell, row, "a") == 0);
  int64_t csize = 0;
  mlxd_array_size(&csize, cell);
  const void* cdata = NULL;
  mlxd_array_data(&cdata, cell);
  TEST_CHECK(csize == 1 && ((const char*)cdata)[0] == '1');

  /* --- reset replays the stream (one record was consumed above) --- */
  TEST_CHECK(mlxd_stream_reset(csv) == 0);
  TEST_CHECK(drain(csv) == 2);
  TEST_CHECK(mlxd_stream_reset(csv) == 0);
  TEST_CHECK(drain(csv) == 2);

  /* --- line reader --- */
  mlxd_stream lines = mlxd_stream_new();
  TEST_CHECK(mlxd_stream_line_reader(&lines, lines_path, "line", false, "") ==
             0);
  TEST_CHECK(drain(lines) == 3);

  /* --- repeat: original pass + num_time additional passes --- */
  TEST_CHECK(mlxd_stream_reset(lines) == 0); /* repeat inherits position */
  mlxd_stream repeated = mlxd_stream_new();
  TEST_CHECK(mlxd_stream_repeat(&repeated, lines, 2) == 0);
  TEST_CHECK(drain(repeated) == 9); /* 3 x (1 + 2) */
  TEST_CHECK(mlxd_stream_reset(repeated) == 0);
  TEST_CHECK(drain(repeated) == 9);

  /* --- to_buffer materializes exactly the drained samples --- */
  TEST_CHECK(mlxd_stream_reset(repeated) == 0);
  mlxd_buffer materialized = mlxd_buffer_new();
  TEST_CHECK(mlxd_stream_to_buffer(&materialized, repeated) == 0);
  int64_t msize = 0;
  mlxd_buffer_size(&msize, materialized);
  TEST_CHECK(msize == 9);

  mlxd_array_free(cell);
  mlxd_sample_free(row);
  mlxd_buffer_free(materialized);
  mlxd_stream_free(repeated);
  mlxd_stream_free(lines);
  mlxd_stream_free(csv);
  unlink(csv_path);
  unlink(lines_path);
  rmdir(dir);

  printf("test_stream passed\n");
  return 0;
}
