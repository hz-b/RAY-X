#pragma once

#include <CLI/CLI.hpp>

#include "Debug.h"
#include "TerminalAppConfig.h"

class CommandParser {
  public:
    // Default constructor
    CommandParser() = default;
    // Custom constructor
    CommandParser(int ___argc, char* const* ___argv);

    ~CommandParser();

    void analyzeCommands();

    std::shared_ptr<CLI::App> m_cli11;
    // CLI Arguments
    // Flags initialize to DISABLED
    // Set options in .cpp file
    struct Args {
        bool m_plotFlag = false;          // -p (Plot)
        bool m_csvFlag = false;           // -c (.csv Output)
        bool m_dummyFlag = false;         // -d (Dummy Beamline)
        bool m_benchmark = false;         // -b (Benchmark)
        bool m_cpuFlag = false;           // -x (CPU Tracer)
        bool m_multiplePlots = false;     // -m (Multiple Plots)
        bool m_version = false;           // -v (Version)
        std::string m_providedFile = "";  // -i (Input)
    } m_args;

    inline void getVersion() const {
        RAYX_LOG << R"(
          
        ╔═══╗╔═══╗╔╗  ╔╗     ╔═╗╔═╗
        ║╔═╗║║╔═╗║║╚╗╔╝║     ╚╗╚╝╔╝
        ║╚═╝║║║ ║║╚╗╚╝╔╝      ╚╗╔╝ 
        ║╔╗╔╝║╚═╝║ ╚╗╔╝ ╔═══╗ ╔╝╚╗ 
        ║║║╚╗║╔═╗║  ║║  ╚═══╝╔╝╔╗╚╗
        ╚╝╚═╝╚╝ ╚╝  ╚╝       ╚═╝╚═╝ HZB 2022.
        )";
        RAYX_LOG << "\t RAY-X Terminal Application "
                 << TERMINALAPP_VERSION_MAJOR << "."
                 << TERMINALAPP_VERSION_MINOR << "."
                 << TERMINALAPP_VERSION_PATCH << "."
                 << TERMINALAPP_VERSION_TWEAK << "\n \t GIT: " << GIT_REVISION
                 << "\n \t BUILD: " << BUILD_TIMESTAMP;
    };  // TODO: CMake config needed
  private:
    int m_cli11_return;
    enum OptionType { BOOL, INT, STRING };
    struct Options {
        // CLI::Option cli11_option;
        const OptionType type;
        const char* full_name;
        const char* description;
        void* option_flag;
    };
    // Map short arg to its parameters
    // this can also be done with app.get_option()
    std::unordered_map<char, Options> m_ParserCommands = {
        {'p',
         {OptionType::BOOL, "plot", "Plot output footprints and histograms.",
          &(m_args.m_plotFlag)}},
        {'c',
         {OptionType::BOOL, "ocsv", "Output stored as .csv file.",
          &(m_args.m_csvFlag)}},
        {'d',
         {OptionType::BOOL, "dummy", "Run an in-house built Beamline.",
          &(m_args.m_dummyFlag)}},
        {'x',
         {OptionType::BOOL, "cpu", "Run CPU tracer instead of GPU tracer.",
          &(m_args.m_cpuFlag)}},
        {'b',
         {OptionType::BOOL, "benchmark",
          "Benchmark application: \t (RML Parse → Trace → Output Storage)",
          &(m_args.m_benchmark)}},
        {'m',
         {OptionType::BOOL, "mult", "Multiple plots extension at output.",
          &(m_args.m_multiplePlots)}},
        {'i',
         {OptionType::STRING, "input", "Input RML File Path.",
          &(m_args.m_providedFile)}},
        {'v', {OptionType::BOOL, "version", "", &(m_args.m_version)}},
    };
};
