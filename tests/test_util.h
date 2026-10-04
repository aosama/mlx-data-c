#ifndef MLXD_TEST_UTIL_H
#define MLXD_TEST_UTIL_H

#include <stdio.h>
#include <stdlib.h>

#include "mlx/data/c/mlx-data.h"

/* Tests install this to silence the default exit-on-error handler; the
 * status codes themselves are what get asserted. */
static void mlxd_test_quiet_handler(const char* msg, void* data) {
  (void)msg;
  (void)data;
}

#define TEST_CHECK(cond)                                              \
  do {                                                                \
    if (!(cond)) {                                                    \
      fprintf(stderr, "FAILED: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
      exit(1);                                                        \
    }                                                                 \
  } while (0)

#endif
