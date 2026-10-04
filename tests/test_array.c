#include <string.h>

#include "test_util.h"

/* Managed-view dtor for buffers that outlive the handle: a no-op. */
static void no_dtor(void* ctx) {
  (void)ctx;
}

int main(void) {
  mlxd_set_error_handler(mlxd_test_quiet_handler, NULL, NULL);

  /* --- scalar creation and typed reads --- */
  mlxd_array i = mlxd_array_new_scalar_int64(-7);
  mlxd_array f = mlxd_array_new_scalar_float(2.5f);
  mlxd_array u = mlxd_array_new_scalar_uint8(200);
  int64_t iv = 0;
  float fv = 0.0f;
  uint8_t uv = 0;
  TEST_CHECK(mlxd_array_item_int64(&iv, i) == 0 && iv == -7);
  TEST_CHECK(mlxd_array_item_float(&fv, f) == 0 && fv == 2.5f);
  TEST_CHECK(mlxd_array_item_uint8(&uv, u) == 0 && uv == 200);
  TEST_CHECK(mlxd_array_item_float(&fv, i) == 1); /* dtype mismatch */

  /* --- inspection --- */
  int64_t size = 0, itemsize = 0;
  int ndim = 0;
  mlxd_array_type type = MLXD_ANY;
  mlxd_array_size(&size, i);
  mlxd_array_ndim(&ndim, i);
  mlxd_array_itemsize(&itemsize, i);
  mlxd_array_get_type(&type, i);
  TEST_CHECK(size == 1 && ndim == 0 && itemsize == 8);
  TEST_CHECK(type == MLXD_INT64);

  /* --- data-backed arrays: shape and raw data --- */
  const int64_t shape[2] = {2, 3};
  int64_t vals[6] = {10, 11, 12, 13, 14, 15};
  mlxd_array d = mlxd_array_new_data(MLXD_INT64, shape, 2, vals);
  const int64_t* dshape = NULL;
  size_t dndim = 0;
  int64_t dsize = 0;
  mlxd_array_size(&dsize, d);
  mlxd_array_shape(&dshape, &dndim, d);
  TEST_CHECK(dsize == 6 && dndim == 2);
  TEST_CHECK(dshape[0] == 2 && dshape[1] == 3);
  const void* raw = NULL;
  mlxd_array_data(&raw, d);
  TEST_CHECK(((const int64_t*)raw)[5] == 15);

  /* --- set shares ownership (shared_ptr semantics) --- */
  mlxd_array c = mlxd_array_new();
  TEST_CHECK(mlxd_array_set(&c, d) == 0);
  int64_t csize = 0;
  mlxd_array_size(&csize, c);
  const void* craw = NULL;
  mlxd_array_data(&craw, c);
  TEST_CHECK(csize == 6 && craw == raw); /* same underlying buffer */
  int64_t mvals[2] = {42, 43};
  const int64_t mshape[1] = {2};
  /* stack-backed view: no-op dtor, the buffer outlives the handle */
  mlxd_array m = mlxd_array_new_data_managed(
      MLXD_INT64, mshape, 1, mvals, no_dtor, mvals);
  const void* mraw = NULL;
  mlxd_array_data(&mraw, m);
  TEST_CHECK(mraw == (const void*)mvals); /* zero-copy view */
  mvals[0] = 99;
  TEST_CHECK(((const int64_t*)mraw)[0] == 99);

  /* --- strings carry their bytes without a NUL --- */
  mlxd_array s = mlxd_array_new_string("abc");
  int64_t ssize = 0;
  mlxd_array_size(&ssize, s);
  const void* sraw = NULL;
  mlxd_array_data(&sraw, s);
  TEST_CHECK(ssize == 3 && memcmp(sraw, "abc", 3) == 0);

  /* --- lifecycle: errors on empty, free is a no-op there --- */
  mlxd_array empty = mlxd_array_new();
  TEST_CHECK(mlxd_array_size(&size, empty) == 1);
  TEST_CHECK(mlxd_array_free(empty) == 0);
  mlxd_array null_handle = {NULL};
  TEST_CHECK(mlxd_array_free(null_handle) == 0);

  mlxd_array_free(s);
  mlxd_array_free(m);
  mlxd_array_free(c);
  mlxd_array_free(d);
  mlxd_array_free(u);
  mlxd_array_free(f);
  mlxd_array_free(i);

  printf("test_array passed\n");
  return 0;
}
