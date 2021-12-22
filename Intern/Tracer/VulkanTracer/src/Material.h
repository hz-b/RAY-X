#pragma once

#include <vector>

/**
 * This enum class is the c++ perspective on the Materials stored in
 * "materials.xmacro".
 * Whenever you want to add a material, just add a line in materials.xmacro:
 * everything else is done automagically. This makes heavy use of X Macros, see
 * https://en.wikipedia.org/wiki/X_Macro
 **/
enum class Material {
#define X(e) e,
#include "materials.xmacro"
#undef X
};

// the following functions load the tables, and grant you a pointer to them.
// the tables will be written to the nkp and nkpIdx buffers of shader.comp
const std::vector<double>* getMaterialTable();
const std::vector<int>* getMaterialIndexTable();