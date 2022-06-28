cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

# Define output directories for project
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

cmake_policy(SET CMP0054 NEW)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DRAY_DEBUG_MODE")
# TODO(Jannis): add -fPIC for linux

# if (NOT "$ENV{CI}" STREQUAL "YES") # lhdf5 doesn't exist in CI
# 	# for ubuntu users:
# 	include_directories("/usr/include/hdf5/serial")
# 	link_directories("/usr/lib/x86_64-linux-gnu/hdf5/serial")
# 	link_libraries("hdf5")
# endif()


project(Intern)

add_subdirectory(RayCore)
add_subdirectory(TerminalApp)
