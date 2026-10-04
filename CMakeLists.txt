cmake_minimum_required(VERSION 3.16)

project(mlx.data.c LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

include(GNUInstallDirs)

if(NOT MLXD_C_VERSION)
  set(MLXD_C_VERSION 0.1.0)
endif()

option(MLXD_C_BUILD_EXAMPLES "Build the pure-C examples" ON)
option(MLXD_C_USE_SYSTEM_MLX_DATA
       "Consume an installed mlx-data via find_package(MLXData)" OFF)
set(MLXD_C_MLX_DATA_SOURCE_DIR
    ""
    CACHE PATH "Local mlx-data checkout to build against (development override)")

# ----------------------------- mlx-data -----------------------------

if(MLXD_C_USE_SYSTEM_MLX_DATA)
  find_package(MLXData REQUIRED)
elseif(MLXD_C_MLX_DATA_SOURCE_DIR)
  add_subdirectory(${MLXD_C_MLX_DATA_SOURCE_DIR}
                   ${CMAKE_BINARY_DIR}/mlxdata-from-source)
else()
  include(FetchContent)
  if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
  endif()
  FetchContent_Declare(
    mlxdata_ext
    GIT_REPOSITORY "https://github.com/ml-explore/mlx-data.git"
    GIT_TAG v0.2.0)
  FetchContent_MakeAvailable(mlxdata_ext)
endif()

# Whether mlx-data linked the optional-codec IMPORTED targets into its
# interface decides which find_dependency calls the generated package
# config must make. mlx-data runs these finds inside its own scope
# (FetchContent add_subdirectory), so repeat them here — the results
# are cached, making this effectively free.
find_package(SndFile QUIET)
find_package(SampleRate QUIET)
set(MLXD_C_HAD_SNDFILE ${SndFile_FOUND})
set(MLXD_C_HAD_SAMPLERATE ${SampleRate_FOUND})

# ----------------------------- lib -----------------------------

set(mlxdc-src
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/array.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/bpe.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/buffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/closure.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/core.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/error.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/file_fetcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/graph.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/ops.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/sample.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/stream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/tokenizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/trie.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/version.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/string.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/vector_int64.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/vector_string.cpp)

add_library(mlxdc STATIC ${mlxdc-src})

target_link_libraries(mlxdc PRIVATE mlxdata)

# The C++ runtime must reach consumers of the static archives: an
# exported pure-C target's link does not pick it up automatically
# (IMPORTED_LINK_INTERFACE_LANGUAGES proved not to), so the install
# interface carries an explicit -l flag. In-tree, CMake already selects
# the C++ linker from the target's own language, so the build interface
# stays empty to avoid duplicate-library warnings.
if(APPLE)
  target_link_libraries(mlxdc PUBLIC $<INSTALL_INTERFACE:-lc++>)
else()
  target_link_libraries(mlxdc PUBLIC $<INSTALL_INTERFACE:-lstdc++>)
endif()

# mlx-data's codec dependencies must reach every consumer of the static
# archive: full paths in this build tree; exported consumers resolve
# them themselves inside MLXDCConfig.cmake, keeping machine-specific
# paths out of the export.
set(MLXD_C_CODEC_LIBS turbojpeg bz2 zstd lzma z sndfile FLAC opus vorbis
                      vorbisenc ogg)
foreach(mlxd_codec IN LISTS MLXD_C_CODEC_LIBS)
  find_library(MLXD_C_CODEC_${mlxd_codec}_PATH ${mlxd_codec})
  if(MLXD_C_CODEC_${mlxd_codec}_PATH)
    target_link_libraries(mlxdc PUBLIC
      $<BUILD_INTERFACE:${MLXD_C_CODEC_${mlxd_codec}_PATH}>)
  endif()
endforeach()
target_include_directories(
  mlxdc PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_options(mlxdc PRIVATE -Wall -Wextra)
set_property(TARGET mlxdc PROPERTY POSITION_INDEPENDENT_CODE ON)

# Windows DLL symbol exports
if(WIN32)
  set_target_properties(mlxdc PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

# ----------------------------- ctest suite -----------------------------

option(MLXD_C_BUILD_TESTS "Build the ctest suite" ON)
if(MLXD_C_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# ----------------------------- install / package -----------------------------

include(CMakePackageConfigHelpers)

set(MLXDC_CMAKE_INSTALL_DIR share/cmake/MLXDC)

install(
  TARGETS mlxdc
  EXPORT MLXDCTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# A from-source mlxdata (FetchContent or local checkout) ships with the
# package: libmlxdc.a references its symbols, so the consumer needs the
# archive to link. A system mlxdata stays external and the generated
# config finds it via find_dependency(MLXData).
if(NOT MLXD_C_USE_SYSTEM_MLX_DATA)
  install(
    TARGETS mlxdata
    EXPORT MLXDCTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

install(
  DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/mlx
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT headers
  FILES_MATCHING
  PATTERN "*.h"
  PATTERN "private" EXCLUDE)

install(
  EXPORT MLXDCTargets
  FILE MLXDCTargets.cmake
  DESTINATION ${MLXDC_CMAKE_INSTALL_DIR})

set(MLXDC_BUILD_CONFIG ${CMAKE_BINARY_DIR}/MLXDCConfig.cmake)
set(MLXDC_BUILD_VERSION_CONFIG ${CMAKE_BINARY_DIR}/MLXDCConfigVersion.cmake)

write_basic_package_version_file(
  ${MLXDC_BUILD_VERSION_CONFIG}
  COMPATIBILITY SameMajorVersion
  VERSION ${MLXD_C_VERSION})

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/MLXDCConfig.cmake.in ${MLXDC_BUILD_CONFIG}
  INSTALL_DESTINATION ${MLXDC_CMAKE_INSTALL_DIR}
  PATH_VARS CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_INCLUDEDIR
            MLXDC_CMAKE_INSTALL_DIR)

install(FILES ${MLXDC_BUILD_CONFIG} ${MLXDC_BUILD_VERSION_CONFIG}
        DESTINATION ${MLXDC_CMAKE_INSTALL_DIR})

set(MLXDC_BUILD_PC ${CMAKE_BINARY_DIR}/mlx-data-c.pc)
if(APPLE)
  set(MLXD_C_PC_RUNTIME_LIB c++)
else()
  set(MLXD_C_PC_RUNTIME_LIB stdc++)
endif()
configure_file(${CMAKE_CURRENT_LIST_DIR}/mlx-data-c.pc.in ${MLXDC_BUILD_PC}
               @ONLY)
install(FILES ${MLXDC_BUILD_PC}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if(MLXD_C_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
