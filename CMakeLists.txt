cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

set (CMAKE_CXX_STANDARD 20)

project(RAYX)

# enable all warnings
if (MSVC)
	message(STATUS "MSVC")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
	message(STATUS "GCC")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic") # -Wconversion -Wsign-conversion")
endif()

if ("$ENV{CI}" STREQUAL "YES")
	# warnings are treated as errors
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Werror -D CI") # -O3 finds more errors!
	message(STATUS "CI mode")
endif()

add_subdirectory(Intern)
add_subdirectory(Extern)
add_subdirectory(Tests)
