#include "mlx/data/c/file_fetcher.h"

#include <stdexcept>
#include <string>

#include "mlx/data/core/FileFetcher.h"
#include "mlx/data/c/error.h"
#include "mlx/data/c/private/file_fetcher.h"

#if defined(MLX_HAS_AWS)
#include "mlx/data/core/AWSFileFetcher.h"
#endif

extern "C" mlxd_file_fetcher mlxd_file_fetcher_new(void) {
  return mlxd_file_fetcher_new_();
}

extern "C" int mlxd_file_fetcher_free(mlxd_file_fetcher fetcher) {
  try {
    mlxd_file_fetcher_free_(fetcher);
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" int
mlxd_file_fetcher_set(mlxd_file_fetcher* dst, mlxd_file_fetcher src) {
  try {
    mlxd_file_fetcher_set_(*dst, mlxd_file_fetcher_get_(src));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
}

extern "C" mlxd_aws_file_fetcher_options
mlxd_aws_file_fetcher_options_default(void) {
  mlxd_aws_file_fetcher_options options;
  options.region = "us-east-1";
  options.prefix = "";
  options.local_prefix = "";
  options.ca_bundle = "";
  options.virtual_host = 0;
  options.verify_ssl = 1;
  options.connect_timeout_ms = 1000;
  options.num_retry_max = 10;
  options.num_connection_max = 25;
  options.buffer_size = 100 * 1024 * 1024;
  options.num_threads = 4;
  options.num_prefetch_max = 1;
  options.num_prefetch_threads = 1;
  options.num_kept_files = 0;
  options.access_key_id = "";
  options.secret_access_key = "";
  options.session_token = "";
  options.expiration = "";
  options.verbose = 0;
  return options;
}

extern "C" int mlxd_aws_file_fetcher_new(
    mlxd_file_fetcher* out,
    const char* bucket,
    const mlxd_aws_file_fetcher_options* options) {
#if defined(MLX_HAS_AWS)
  try {
    if (bucket == nullptr) {
      throw std::invalid_argument("mlxd_aws_file_fetcher_new: bucket is NULL");
    }
    if (options == nullptr) {
      throw std::invalid_argument(
          "mlxd_aws_file_fetcher_new: options is NULL");
    }
    mlx::data::core::AWSFileFetcherOptions cpp_options;
    cpp_options.region = options->region ? options->region : "us-east-1";
    cpp_options.prefix = options->prefix ? options->prefix : "";
    cpp_options.local_prefix =
        options->local_prefix ? options->local_prefix : "";
    cpp_options.ca_bundle = options->ca_bundle ? options->ca_bundle : "";
    cpp_options.virtual_host = options->virtual_host != 0;
    cpp_options.verify_ssl = options->verify_ssl != 0;
    cpp_options.connect_timeout_ms = options->connect_timeout_ms;
    cpp_options.num_retry_max = options->num_retry_max;
    cpp_options.num_connection_max = options->num_connection_max;
    cpp_options.buffer_size = options->buffer_size;
    cpp_options.num_threads = options->num_threads;
    cpp_options.num_prefetch_max = options->num_prefetch_max;
    cpp_options.num_prefetch_threads = options->num_prefetch_threads;
    cpp_options.num_kept_files = options->num_kept_files;
    cpp_options.access_key_id =
        options->access_key_id ? options->access_key_id : "";
    cpp_options.secret_access_key =
        options->secret_access_key ? options->secret_access_key : "";
    cpp_options.session_token =
        options->session_token ? options->session_token : "";
    cpp_options.expiration = options->expiration ? options->expiration : "";
    cpp_options.verbose = options->verbose != 0;
    mlxd_file_fetcher_set_(
        *out,
        std::make_shared<mlx::data::core::AWSFileFetcher>(bucket, cpp_options));
  } catch (std::exception& e) {
    mlxd_error(e.what());
    return 1;
  }
  return 0;
#else
  (void)bucket;
  (void)options;
  (void)out;
  mlxd_error("mlxd_aws_file_fetcher_new: mlx-data was built without AWS "
             "support (AWSSDK not found at its configure time)");
  return 1;
#endif
}
