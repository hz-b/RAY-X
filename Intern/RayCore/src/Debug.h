#pragma once

//

/**
 *  Include this if you want macros for debugging.
 *
 *  Include stdlib before this header if you need it
 *
 */

#include <array>
#include <glm.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <Tracer/Ray.h>

#include "Core.h"

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

namespace RAYX {

///////////////////////////////////////////////
// LOGGING SYSTEM
///////////////////////////////////////////////

/**
 *
 * In the following we define
 * RAYX_LOG: prints to std::cout
 * RAYX_ERR: prints to std::cerr
 * ... and their debug-only variants RAYX_D_LOG, RAYX_D_ERR.
 *
 * example usage:
 * RAYX_LOG << "I am " << age << " years old";
 * */

struct RAYX_API Log {
    Log(std::string filename, int line);
    ~Log();

    template <typename T>
    Log& operator<<(T t) {
        std::cout << t;
        return *this;
    }
};

struct RAYX_API Warn {
    Warn(std::string filename, int line);

    ~Warn();

    template <typename T>
    Warn& operator<<(T t) {
        std::cerr << t;
        return *this;
    }
};

struct RAYX_API Err {
    std::string filename;
    int line;

    Err(std::string filename, int line);

    ~Err();

    template <typename T>
    Err& operator<<(T t) {
        std::cerr << t;
        return *this;
    }
};

struct RAYX_API IgnoreLog {
    template <typename T>
    IgnoreLog& operator<<(T) {
        return *this;
    }
};

#define RAYX_LOG RAYX::Log(__FILE__, __LINE__)
#define RAYX_WARN RAYX::Warn(__FILE__, __LINE__)
#define RAYX_ERR RAYX::Err(__FILE__, __LINE__)

#ifdef RAY_DEBUG_MODE
#define RAYX_D_LOG RAYX_LOG
#define RAYX_D_WARN RAYX_WARN
#define RAYX_D_ERR RAYX_ERR

#else
#define RAYX_D_LOG RAYX::IgnoreLog()
#define RAYX_D_WARN RAYX::IgnoreLog()
#define RAYX_D_ERR RAYX::IgnoreLog()
#endif

/////////////////////////////////////////////////////////////////////////////
// COLLECTION DEBUGGING SYSTEM
/////////////////////////////////////////////////////////////////////////////

/**
 *
 * In the following we define
 * RAYX_DBG: prints collection to RAYX_LOG for debugging
 *
 * example usage:
 * RAYX_DBG(orientation);
 * RAYX_DBG(position);
 * */

template <int N, int M>
inline std::vector<double> formatAsVec(glm::mat<N, M, double> arg) {
    std::vector<double> out(N * M);
    for (size_t i = 0; i < N * M; i++) {
        out[i] = arg[i / N][i % N];
    }
    return out;
}

template <int N>
inline std::vector<double> formatAsVec(glm::vec<N, double> arg) {
    std::vector<double> out(N);
    for (size_t i = 0; i < N; i++) {
        out[i] = arg[i];
    }
    return out;
}

template <size_t N>
inline std::vector<double> formatAsVec(std::array<double, N> arg) {
    std::vector<double> out(N);
    for (size_t i = 0; i < N; i++) {
        out[i] = arg[i];
    }
    return out;
}

inline std::vector<double> formatAsVec(double arg) { return {arg}; }

void dbg(std::string filename, int line, std::string name,
         std::vector<double> v);

#define RAYX_DBG(C) RAYX::dbg(__FILE__, __LINE__, #C, RAYX::formatAsVec(C))

}  // namespace RAYX
