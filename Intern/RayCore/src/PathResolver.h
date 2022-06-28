#pragma once

#include <string>

#include "Core.h"

// the path resolver has to be initialized with the path of the executable: argv[0].
void RAYX_API initPathResolver(char* executablePath);

/** @param path: path relative to the root of the repository.
 * Converts this path to be relative to the current working directory, so that
 * it may be used by eg. open()
 */
std::string RAYX_API resolvePath(std::string path);