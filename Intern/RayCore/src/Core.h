#pragma once

#if 1
#define RAYX_PROFILE
#endif

// Memory leak detection (RAYX_NEW instead of new allows leaks to be detected)
#ifdef RAY_DEBUG_MODE
#ifdef RAYX_PLATFORM_MSVC
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#define RAYX_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define RAYX_NEW new
#endif

// Debug only code; use it as: DEBUG(<statement>);
#ifdef RAY_DEBUG_MODE
#define RAYX_DEBUG(x) (x)
#else
#define RAYX_DEBUG(x) \
    do {              \
    } while (0)
#endif

/**
 *  Defining the RAYX_API macro, which helps with
 *  building the library (context based import/export of code).
 */
#if defined(RAYX_PLATFORM_MSVC)  //  Microsoft
#ifdef RAYX_BUILD_DLL
#define RAYX_API __declspec(dllexport)
#else
#define RAYX_API __declspec(dllimport)
#endif
#elif defined(RAYX_PLATFORM_GCC)  //  GCC
#ifdef RAYX_BUILD_DLL
#define RAYX_API __attribute__((visibility("default")))
#else
#define RAYX_API
#endif
#else  //  do nothing and hope for the best?
#ifdef RAYX_BUILD_DLL
#define RAYX_API
#else
#define RAYX_API
#endif
#pragma warning Unknown dynamic link import / export semantics.
#endif