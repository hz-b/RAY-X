cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(RayTests)

# if (NOT "$ENV{CI}" STREQUAL "YES") # lhdf5 doesn't exist in CI
# 	# for ubuntu users:
# 	include_directories("/usr/include/hdf5/serial")
# 	link_directories("/usr/lib/x86_64-linux-gnu/hdf5/serial")

# 	link_libraries("hdf5")
# endif()


include(CTest)
enable_testing()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(BINARY ${CMAKE_PROJECT_NAME}_tst)


file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp)
set(SOURCES ${TEST_SOURCES})
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})

target_precompile_headers(${BINARY} PRIVATE testsPCH.h)
set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)
#if (WIN32)
    #target_compile_definitions(${BINARY} PUBLIC RAYX_PLATFORM_MSVC)
#endif ()
target_link_libraries(${BINARY} PUBLIC RayCore gtest gmock)

