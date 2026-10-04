# mlx-data-c

A pure C API for [mlx-data](https://github.com/ml-explore/mlx-data), Apple's data loading library for MLX — giving C programs (and Swift, Rust, or any FFI) the same access to `Buffer`, `Stream`, and the full dataset-operation toolbox that [mlx-c](https://github.com/ml-explore/mlx-c) gives to the MLX array library.

**Status: in active development.** The API surface is being built out issue by issue — see the [tracking issue](https://github.com/aosama/mlx-data-c/issues/14) for the milestone map and the definition of done. Everything below describes the target design.

## API conventions

The C layer follows mlx-c's proven pattern, adapted to mlx-data's shapes:

| Concept | C form |
|---|---|
| Wrapped C++ type | Opaque handle: `typedef struct mlxd_x_ { void* ctx; } mlxd_x;` |
| Status | `int` return: `0` success, `1` error, `2` documented lookup miss |
| Results | Written through the first pointer parameter |
| Lists (shapes, dims, perms) | Pointer + count: `const T*, size_t n` |
| Maps (`pad_values`, `batch_dims`) | Parallel arrays: `const char** keys, const T* values, size_t n` |
| Returned strings | Owned `mlxd_string` handle |
| Errors | `mlxd_error(...)` plus installable handler via `mlxd_set_error_handler` |

Target API shape (design preview — see the [tracking issue](https://github.com/aosama/mlx-data-c/issues/14) for implementation status):

```c
#include "mlx/data/c/mlx-data.h"

mlxd_buffer buffer = mlxd_buffer_new();
mlxd_buffer_from_samples(&buffer, samples, 3);

mlxd_buffer batched = mlxd_buffer_new();
const char* pad_keys[] = {"text"};
const double pad_values[] = {0.0};
mlxd_buffer_batch(&batched, buffer, 3, pad_keys, pad_values, 1, NULL, NULL, 0);

mlxd_sample sample = mlxd_sample_new();
mlxd_buffer_get(&sample, batched, 0);
```

Semantics are inherited from the C++ core, including two contracts worth knowing up front:

- Operations are lazy: they build a pipeline; work happens at `mlxd_buffer_get` / `mlxd_stream_next`.
- `mlxd_stream_next` returns an empty sample at end of stream (same signal the Python binding uses).

## Building

Requires CMake ≥ 3.16, a C11/C++17 toolchain, and network access on the first configure to fetch mlx-data and its dependencies (stb, bxzstr).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Options:

| Option | Default | Meaning |
|---|---|---|
| `MLXD_C_BUILD_EXAMPLES` | `ON` | Build the pure-C examples |
| `MLXD_C_USE_SYSTEM_MLX_DATA` | `OFF` | `find_package(MLXData)` instead of FetchContent |
| `MLXD_C_MLX_DATA_SOURCE_DIR` | empty | Point FetchContent at a local mlx-data checkout (development) |

Notes:

- On CMake ≥ 4.0, if a fetched dependency declares an outdated minimum version, retry the configure with `-DCMAKE_POLICY_VERSION_MINIMUM=3.5`.
- mlx-data's optional backends (libsndfile, FFmpeg, libjpeg-turbo, AWS SDK) remain optional here: their absence never breaks the build; the features that need them fail at runtime with the underlying error message.

## Project layout

- `mlx/data/c/` — public C headers (one doxygen group per header) and their implementations; installed
- `mlx/data/c/private/` — inline C++ bridge helpers; never installed
- `examples/` — pure-C examples; the compile-time proof that the public headers stay C
- `tests/` — ctest suite (arriving with milestone 3)

## Roadmap

- Milestone 1 — core pipeline (array, sample, buffer, stream, all shared ops) and tokenizer machinery
- Milestone 2 — user callbacks (closures), file fetchers, misc core utilities
- Milestone 3 — packaging, install, ctest suite, consumer smoke test

Progress and acceptance criteria live in the [GitHub issues](https://github.com/aosama/mlx-data-c/issues/14).

## License

[MIT](LICENSE)
