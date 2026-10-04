cmake_minimum_required(VERSION 3.16)

project(mlx.data.c LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

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

# ----------------------------- lib -----------------------------

set(mlxdc-src
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/array.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/buffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/error.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/sample.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/stream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/string.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/vector_int64.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/c/vector_string.cpp)

add_library(mlxdc STATIC ${mlxdc-src})

target_link_libraries(mlxdc PRIVATE mlxdata)
target_include_directories(
  mlxdc PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)
target_compile_options(mlxdc PRIVATE -Wall -Wextra)
set_property(TARGET mlxdc PROPERTY POSITION_INDEPENDENT_CODE ON)

# Windows DLL symbol exports
if(WIN32)
  set_target_properties(mlxdc PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

if(MLXD_C_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
