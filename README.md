# mlx-data-c

A pure C API for [mlx-data](https://github.com/ml-explore/mlx-data), Apple's data loading library for MLX — giving C programs (and Swift, Rust, or any FFI) the same access to `Buffer`, `Stream`, and the full dataset-operation toolbox that [mlx-c](https://github.com/ml-explore/mlx-c) gives to the MLX array library.

**Status: complete against mlx-data v0.2.0.** Every capability of the C++ core is reachable from C: samples, arrays, buffers, streams, all dataset operations (including conditional `_if` forms), batching, CSV/line/tar readers, trie/BPE tokenization, user callbacks, file fetchers, and the core array utilities.

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

Usage looks like this:

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

Runnable programs live in [`examples/`](examples/) — a buffer pipeline, tokenization, and stream readers, each self-verifying.

## Building

Requires CMake ≥ 3.16, a C11/C++17 toolchain, and network access on the first configure to fetch mlx-data and its dependencies (stb, bxzstr).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure   # 7/7 green
```

### Build matrix

| Mode | Flag | What happens |
|---|---|---|
| Default | — | mlx-data v0.2.0 fetched via FetchContent and built from source |
| System mlx-data | `-DMLXD_C_USE_SYSTEM_MLX_DATA=ON` | `find_package(MLXData)` — an installed mlx-data is consumed and stays external to the package config |
| Local checkout | `-DMLXD_C_MLX_DATA_SOURCE_DIR=/path/to/mlx-data` | Builds against your checkout (development) |

The `tests/` suite is on by default (`MLXD_C_BUILD_TESTS=OFF` to skip); the examples likewise (`MLXD_C_BUILD_EXAMPLES`).

Notes:

- On CMake ≥ 4.0, if a fetched dependency declares an outdated minimum version, retry the configure with `-DCMAKE_POLICY_VERSION_MINIMUM=3.5`.
- mlx-data's optional backends (libsndfile, FFmpeg, libjpeg-turbo, AWS SDK) remain optional here: their absence never breaks the build; the features that need them fail at runtime with the underlying error message. When libsndfile/SampleRate were found at build time, the generated package config re-finds them for consumers.

## Installing and consuming

```bash
cmake --install build --prefix /somewhere
```

This lays out:

- `include/mlx/data/c/*.h` — the public C headers (the C++ bridge under `private/` is never installed; mlx-data's own C++ headers ship alongside for static linking)
- `lib/libmlxdc.a` and `lib/libmlxdata.a` — the C API and the C++ core it wraps (static archives need both)
- `share/cmake/MLXDC/` — `MLXDCConfig.cmake`, `MLXDCConfigVersion.cmake`, `MLXDCTargets.cmake`
- `lib/pkgconfig/mlx-data-c.pc`

Consume from CMake:

```cmake
find_package(MLXDC REQUIRED)
target_link_libraries(myprog PRIVATE mlxdc)
```

The package config resolves mlx-data's codec dependencies (threads, libsndfile, SampleRate, then turbojpeg/bz2/zstd/lzma/z/FLAC/opus/vorbis) and the C++ runtime at consume time — a pure-C target links with nothing else. `scripts/consumer_smoke_test.sh` verifies this end to end from a scratch project.

Consume from pkg-config:

```bash
cc myprog.c $(pkg-config --cflags --libs mlx-data-c)
```

## Project layout

- `mlx/data/c/` — public C headers (one doxygen group per header) and their implementations; installed
- `mlx/data/c/private/` — inline C++ bridge helpers; never installed
- `examples/` — pure-C examples; the compile-time proof that the public headers stay C
- `tests/` — ctest suite (lifecycle, sample dict, batching, stream contract, ops + closures, tokenizer, misc core)
- `scripts/consumer_smoke_test.sh` — installs into a scratch prefix and builds a `find_package(MLXDC)`-only consumer
- `repo-discovery-guide-for-agents.md` — navigation map for coding agents

## License

[MIT](LICENSE)
