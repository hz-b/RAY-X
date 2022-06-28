#pragma once

// Precompiled headers:

#include <array>
#include <glm.hpp>
#include <iostream>
#include <vector>

#ifdef _MSC_VER 
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

#include "Core.h"
#include "Data/xml.h"