# Changelog

All notable changes to mlx-data-c are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/); versioning is
SemVer.

## [0.1.0] - 2026-10-04

First complete release: the full mlx-data v0.2.0 C++ core is reachable
from pure C.

### Added

- Handle types: `mlxd_string`, `mlxd_vector_string`, `mlxd_vector_int64`,
  `mlxd_array` (copy + managed zero-copy), `mlxd_sample`, `mlxd_buffer`,
  `mlxd_stream`, `mlxd_trie`, `mlxd_graph`, `mlxd_tokenizer`,
  `mlxd_tokenizer_iterator`, `mlxd_bpe_merges`, `mlxd_bpe_tokenizer`,
  `mlxd_closure_array` / `_sample` / `_buffer`, `mlxd_file_fetcher`
- Error module: exit-on-error default, installable handler
- Dataset operations on both buffers and streams — every shared op,
  each with a conditional `_if` variant; C overloads for
  shape/dims/slice/pad argument lists
- Constructors: `buffer_from_vector`, `files_from_tar`,
  `stream_csv_reader` (file and string), `stream_line_reader`,
  per-key `_from_key` reader variants, and `_with_fetcher` variants of
  the readers and tar reading
- Closures: C callbacks for `key_transform`, `sample_transform`,
  `buffered`; pipelines share closure state and may fire after the
  closure handle is freed
- File fetchers: `FileFetcher` (consume-only handle) and
  `AWSFileFetcher` with the full options struct; AWS is a compile-time
  guard returning status 1 with a clear message when the linked
  mlx-data lacks AWS support
- Misc core: `set_state`, `levenshtein`, `uniq`, `remove`
- Tokenizer machinery: trie insert/search, tokenization lattices,
  iterator protocol, shortest/random tokenization, BPE merges +
  tokenizer, version info
- Packaging: CMake install/export (`find_package(MLXDC)`),
  pkg-config file; consumer smoke test script
- ctest suite: seven asserting executables (array, sample, buffer,
  stream, ops+closures, tokenizer, misc) with a checked-in tar fixture
- Three self-verifying pure-C examples (buffer pipeline, tokenization,
  stream readers), CMake-enforced `-Werror`, ASan-clean

### Fixed

- `mlxd_sample_get` / `mlxd_sample_erase` crashed on a NULL key
  instead of returning status 1 (found by the new test suite)
