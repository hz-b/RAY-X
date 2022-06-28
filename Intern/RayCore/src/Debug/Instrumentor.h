//
// Basic instrumentation profiler by Cherno

// Usage: include this header file somewhere in your code (eg. precompiled
// header), and then use like:
//
// Instrumentor::Get().BeginSession("Session Name");        // Begin session
// {
//     InstrumentationTimer timer("Profiled Scope Name");   // Place code like
//     this in scopes you'd like to include in profiling
//     // Code
// }
// Instrumentor::Get().EndSession();                        // End Session
//
// You will probably want to macro-fy this, to switch on/off easily and use
// things like __FUNCSIG__ for the profile name.
//
#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <string>
#include <thread>

#include "Core.h"

namespace RAYX {

struct RAYX_API ProfileResult {
    std::string Name;
    long long Start, End;
    uint32_t ThreadID;
};

struct RAYX_API InstrumentationSession {
    std::string Name;
};

class RAYX_API Instrumentor {
  private:
    InstrumentationSession* m_CurrentSession;
    std::ofstream m_OutputStream;
    int m_ProfileCount;

  public:
    Instrumentor() : m_CurrentSession(nullptr), m_ProfileCount(0) {}

    void BeginSession(const std::string& name,
                      const std::string& filepath = "results.json") {
        m_OutputStream.open(filepath);
        WriteHeader();
        m_CurrentSession = new InstrumentationSession{name};
    }

    void EndSession() {
        WriteFooter();
        m_OutputStream.close();
        delete m_CurrentSession;
        m_CurrentSession = nullptr;
        m_ProfileCount = 0;
    }

    void WriteProfile(const ProfileResult& result) {
        if (m_ProfileCount++ > 0) m_OutputStream << ",";

        std::string name = result.Name;
        std::replace(name.begin(), name.end(), '"', '\'');

        m_OutputStream << "{";
        m_OutputStream << "\"cat\":\"function\",";
        m_OutputStream << "\"dur\":" << (result.End - result.Start) << ',';
        m_OutputStream << "\"name\":\"" << name << "\",";
        m_OutputStream << "\"ph\":\"X\",";
        m_OutputStream << "\"pid\":0,";
        m_OutputStream << "\"tid\":" << result.ThreadID << ",";
        m_OutputStream << "\"ts\":" << result.Start;
        m_OutputStream << "}";

        m_OutputStream.flush();
    }

    void WriteHeader() {
        m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
        m_OutputStream.flush();
    }

    void WriteFooter() {
        m_OutputStream << "]}";
        m_OutputStream.flush();
    }

    static Instrumentor& Get() {
        static Instrumentor instance;
        return instance;
    }
};

class RAYX_API InstrumentationTimer {
  public:
    InstrumentationTimer(const char* name) : m_Name(name), m_Stopped(false) {
        m_StartTimepoint = std::chrono::high_resolution_clock::now();
    }

    ~InstrumentationTimer() {
        if (!m_Stopped) Stop();
    }

    void Stop() {
        auto endTimepoint = std::chrono::high_resolution_clock::now();

        long long start =
            std::chrono::time_point_cast<std::chrono::microseconds>(
                m_StartTimepoint)
                .time_since_epoch()
                .count();
        long long end = std::chrono::time_point_cast<std::chrono::microseconds>(
                            endTimepoint)
                            .time_since_epoch()
                            .count();

        uint32_t threadID =
            std::hash<std::thread::id>{}(std::this_thread::get_id());
        Instrumentor::Get().WriteProfile({m_Name, start, end, threadID});

        m_Stopped = true;
    }

  private:
    const char* m_Name;
    std::chrono::time_point<std::chrono::high_resolution_clock>
        m_StartTimepoint;
    bool m_Stopped;
};

}  // namespace RAYX

// Define profiling macros
#ifdef RAYX_PROFILE
#define RAYX_PROFILE_BEGIN_SESSION(name, filepath) \
    ::RAYX::Instrumentor::Get().BeginSession(name, filepath)
#define RAYX_PROFILE_END_SESSION() ::RAYX::Instrumentor::Get().EndSession()
#define RAYX_PROFILE_SCOPE(name) \
    ::RAYX::InstrumentationTimer timer##__LINE__(name)
#if !defined(__PRETTY_FUNCTION__) && !defined(__GNUC__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif
#define RAYX_PROFILE_FUNCTION() RAYX_PROFILE_SCOPE(__PRETTY_FUNCTION__)
#else
#define RAYX_PROFILE_BEGIN_SESSION(name, filepath)
#define RAYX_PROFILE_END_SESSION()
#define RAYX_PROFILE_SCOPE(name)
#define RAYX_PROFILE_FUNCTION()
#endif