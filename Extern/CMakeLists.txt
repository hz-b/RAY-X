cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

set( gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll" FORCE)

add_subdirectory(googletest)

# CLI 
# cli11
set(CLI11_SANITIZERS OFF)
set(CLI11_BUILD_DOCS OFF)
set(CLI11_BUILD_TESTS OFF)
set(CLI11_BUILD_EXAMPLES OFF)
set(CLI11_BUILD_EXAMPLES_JSON OFF)
set(CLI11_SINGLE_FILE_TESTS OFF)
set(CLI11_INSTALL OFF)
set(CLI11_FORCE_LIBCXX OFF)
set(CLI11_CUDA_TESTS OFF)
set(CLI11_CLANG_TIDY OFF)
add_subdirectory(CLI11)

