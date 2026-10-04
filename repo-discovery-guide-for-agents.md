# mlx-data-c Repo Discovery Guide

Pure C11 bindings over Apple's mlx-data C++ core (pinned v0.2.0), laid
out to be consumed from C, Swift, Rust, or any FFI the same way mlx-c
binds MLX.

## Maintenance mandate

Load the repo-discovery-guide skill immediately without delay and
follow its instructions to the letter.

- Update this guide in the same change whenever work touches anything it
  documents; keep it updated before committing or pushing.
- `Last verified` older than 3 days makes every fact here suspect —
  re-verify before relying on it.
- Adding, removing, renaming, or discovering an expensive gotcha gets
  recorded here in the same change.

Last verified: 2026-10-04

## Project overview

`mlx-data-c` wraps every mlx-data C++ capability in a flat `mlxd_*` C
API: the most important architectural fact is that the C layer is a thin
bridge — public headers are pure C, and all C++ lives in a never-installed
`private/` inline bridge plus the `.cpp` files, with the C++ core itself
built from source (FetchContent) into `libmlxdata.a` alongside
`libmlxdc.a`. Upstream ml-explore/mlx-data has been dormant since
September 2025; v0.2.0 is both the pin and the final release, so the API
surface is effectively frozen.

## Known gotchas

- Status-code contract: `0` success, `1` error, `2` documented lookup
  miss (absent key, out-of-bounds buffer index, trie miss). `2` never
  calls the error handler; `1` always does (default handler exits —
  tests install a quiet handler first).
- Handles are by-value structs wrapping `void* ctx`. A freed handle
  cannot invalidate its own copy: reusing a stale handle segfaults. The
  recurring test-authoring trap — always re-init with `mlxd_x_new()`
  before reuse. `free` on an empty (NULL ctx) handle is a safe no-op.
- `mlxd_array_item_*` works on scalars only (size 1).
- `mlxd_array_new_data_managed` REQUIRES a non-NULL `dtor`; pass a
  no-op for stack-backed views. `mlxd_array_set` SHARES the buffer
  (shared_ptr semantics), it does not copy.
- Strings are Int8 arrays without a NUL terminator; CSV cells stay
  Int8 bytes.
- Ops are lazy on BOTH containers: buffer ops fire at
  `mlxd_buffer_get`, stream ops at `mlxd_stream_next` (a closure-based
  `key_transform` runs zero times until the first get).
- `mlxd_stream_repeat(n)` = original + n additional passes, and the new
  stream inherits the source's consumed position — reset the source
  first for deterministic tests.
- `mlxd_buffer_files_from_tar` puts the file NAME in `"file"`;
  contents load via the `read_from_tar` op (from_key=true: tarkey names
  a sample key holding the path; from_key=false: the tarkey parameter
  IS the path).
- `mlxd_uniq` dedups CONSECUTIVE elements; `mlxd_remove` strips
  trailing values and refills with pad (core::remove semantics).
- The AWS fetcher constructor is compile-time guarded (`MLX_HAS_AWS` in
  the linked mlx-data): without AWSSDK it returns status 1 with a clear
  message. This machine's build has no AWS.
- macOS tar needs `COPYFILE_DISABLE=1` or fixtures get `._*` entries;
  `tests/fixtures/tiny.tar` is checked in to avoid this entirely.
- ASan runs against the uninstrumented `libmlxdata.a` need
  `ASAN_OPTIONS=detect_container_overflow=0` (mixed-instrumentation
  libc++ false positive).
- Upstream truths baked into the API (do not "fix" them):
  `repeat(n)` counts the original pass; buffer-level filtering cannot
  drop samples; shard reshapes with n_shards as the leading dim;
  `replace_bytes` maps byte → table entry; `Array(std::string)` has no
  NUL; `mlxd_array_get_type` exists because typedef/function names
  collide in C.
- Exported codecs and the C++ runtime are resolved at CONSUME time by
  `MLXDCConfig.cmake` (find_dependency + find_library loop) and by
  `Libs.private` in the pkg-config file; the exported target carries no
  machine-specific paths. A pure-C consumer needs an explicit `-lc++`
  (the install interface carries it; IMPORTED_LINK_INTERFACE_LANGUAGES
  does not propagate it).

## Conventions

- Opaque handles: `typedef struct mlxd_x_ { void* ctx; } mlxd_x;`;
  results through the first pointer param; lists pointer+count; maps
  parallel arrays.
- Shared_ptr-wrapped types (Array, Trie, Graph, BPEMerges, FileFetcher)
  keep `shared_ptr<T>*` in ctx; value types (string, vectors, sample,
  tokenizer, iterator, bpe_tokenizer, buffer, stream) keep `T*`.
- `mlx/data/c/private/` holds inline bridge helpers (`_new_/_set_/
  _get_/_free_`); never installed, never exposed.
- Headers include their own deps; every module compiles standalone
  under `clang -std=c11 -Wall -Wextra -Werror`.
- Upstream mlx-data source is the ONLY truth for semantics; GitHub
  issues are guides. The reference checkout lives outside the repo.
- Tests assert (exit non-zero on mismatch), install the quiet error
  handler, and are registered via `add_test` in `tests/CMakeLists.txt`
  (one executable per module family).
- AGENTS.md requires a self-code-review before every commit/push and
  keeping README.md current.

## Structure map

- `CMakeLists.txt` — consumption modes (FetchContent pin v0.2.0 /
  `MLXD_C_USE_SYSTEM_MLX_DATA` / `MLXD_C_MLX_DATA_SOURCE_DIR`), mlxdc
  target with codec INTERFACE resolution, install/export, pkg-config
- `mlx/data/c/*.h|cpp` — 18 public modules (array, bpe, buffer,
  closure, core, error, file_fetcher, graph, ops, sample, stream,
  string, tokenizer, trie, vector_int64, vector_string, version) plus
  `mlx-data.h` umbrella
- `mlx/data/c/private/` — bridge headers (never installed)
- `tests/` — ctest executables + `fixtures/tiny.tar` (checked in)
- `examples/` — self-verifying pure-C programs, `-Werror` enforced
- `scripts/consumer_smoke_test.sh` — scratch-prefix install +
  find_package(MLXDC)-only consumer build/run
- `build/` — default dev build dir (mlxdata from source lands in
  `build/mlxdata-from-source/`); `build-fetch/` — clean-slate
  GitHub-FetchContent verification dir

## Entry points

```bash
cmake -S . -B build && cmake --build build -j   # build
ctest --test-dir build --output-on-failure      # test suite
cmake --install build --prefix /somewhere       # install
scripts/consumer_smoke_test.sh                  # end-to-end packaging check
```

Manual compile of a scratch C file against the build tree (the
codec/-L flags matter):

```bash
clang -std=c11 -Wall -Wextra -Werror -I$PWD test.c \
  build/libmlxdc.a build/mlxdata-from-source/libmlxdata.a \
  -L/opt/homebrew/lib -lstdc++ -lturbojpeg -lbz2 -lzstd -llzma -lz \
  -lsndfile -lFLAC -lopus -lvorbis -lvorbisenc -logg
```

## What to verify

- Build matrix: default FetchContent, `MLXD_C_USE_SYSTEM_MLX_DATA=ON`,
  and `MLXD_C_MLX_DATA_SOURCE_DIR` all configure; clean-slate fetch
  works (see `build-fetch/`)
- Zero warnings across C and C++ units; ctest 7/7 green
- Install layout: `include/mlx/data/c/*.h` (no `private/`), both
  archives, `share/cmake/MLXDC/*`, `lib/pkgconfig/mlx-data-c.pc`
- Consumer smoke test passes from a scratch project with only
  `find_package(MLXDC)` + `target_link_libraries(... mlxdc)`
- ASan on new code paths with
  `ASAN_OPTIONS=detect_container_overflow=0`
- README instructions still work from a clean clone
