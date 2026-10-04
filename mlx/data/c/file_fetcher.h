#ifndef MLXD_FILE_FETCHER_H
#define MLXD_FILE_FETCHER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_file_fetcher File fetchers
 * Fetch-on-demand file backends for the readers and read_from_tar.
 *
 * FileFetcher is a virtual base for extension; subclassing from C is
 * out of scope. C consumers use the built-in AWSFileFetcher (only when
 * the mlx-data build had the AWS SDK).
 */
/**@{*/

/**
 * A file fetcher handle. Wraps a std::shared_ptr<core::FileFetcher>.
 * mlxd_file_fetcher_new returns an EMPTY handle; the only way to build
 * a live fetcher is mlxd_aws_file_fetcher_new (AWS builds only).
 */
typedef struct mlxd_file_fetcher_ {
  void* ctx;
} mlxd_file_fetcher;

/**
 * Returns a new empty handle.
 */
mlxd_file_fetcher mlxd_file_fetcher_new(void);

/**
 * Free the fetcher handle. Readers and tar reads keep their shared
 * references alive. Safe no-op returning 0 on an empty handle.
 */
int mlxd_file_fetcher_free(mlxd_file_fetcher fetcher);

/**
 * Copies `src`'s fetcher reference into `dst`.
 *
 * @returns 0 on success, 1 if `src` is an empty handle.
 */
int mlxd_file_fetcher_set(mlxd_file_fetcher* dst, mlxd_file_fetcher src);

/**
 * Connection and retry settings for mlxd_aws_file_fetcher_new, mirroring
 * AWSFileFetcherOptions field-for-field. Use
 * mlxd_aws_file_fetcher_options_default() and override as needed; every
 * string may be NULL, meaning the documented default.
 */
typedef struct {
  const char* region;            /* default "us-east-1" */
  const char* prefix;            /* default "" */
  const char* local_prefix;      /* default "" */
  const char* ca_bundle;         /* default "" */
  int virtual_host;              /* default 0 (false) */
  int verify_ssl;                /* default 1 (true) */
  int64_t connect_timeout_ms;    /* default 1000 */
  int64_t num_retry_max;         /* default 10 */
  int num_connection_max;        /* default 25 */
  int64_t buffer_size;           /* default 104857600 (100MB) */
  int num_threads;               /* default 4 */
  int num_prefetch_max;          /* default 1 */
  int num_prefetch_threads;      /* default 1 */
  int64_t num_kept_files;        /* default 0 */
  const char* access_key_id;     /* default "" */
  const char* secret_access_key; /* default "" */
  const char* session_token;     /* default "" */
  const char* expiration;        /* default "" */
  int verbose;                   /* default 0 (false) */
} mlxd_aws_file_fetcher_options;

/**
 * Returns the option defaults documented above.
 */
mlxd_aws_file_fetcher_options mlxd_aws_file_fetcher_options_default(void);

/**
 * Builds an S3-backed fetcher. ONLY AVAILABLE when the linked mlx-data
 * was built with the AWS SDK; in a non-AWS build this returns status 1
 * with a "built without AWS support" message (compile-time guard).
 *
 * @returns 0 on success, 1 on failure or in a non-AWS build.
 */
int mlxd_aws_file_fetcher_new(
    mlxd_file_fetcher* out,
    const char* bucket,
    const mlxd_aws_file_fetcher_options* options);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
