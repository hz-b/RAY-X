#include "TerminalApp.h"

#include "Debug.h"
#include "PathResolver.h"

// TODO: (potential) Replace Getopt with boost(header-only)
#include <Tracer/CpuTracer.h>
#include <Tracer/VulkanTracer.h>

#include <Writer/Writer.hpp>
#include <memory>
#include <stdexcept>

TerminalApp::TerminalApp() {}

TerminalApp::TerminalApp(int argc, char** argv) : m_argv(argv), m_argc(argc) {
    initPathResolver(argv[0]);
    RAYX_D_LOG << "TerminalApp created!";
}

TerminalApp::~TerminalApp() { RAYX_D_LOG << "TerminalApp deleted!"; }

void TerminalApp::run() {
    RAYX_PROFILE_FUNCTION();

    RAYX_D_LOG << "TerminalApp running...";

    /////////////////// Argument Parser
    m_CommandParser = std::make_unique<CommandParser>(m_argc, m_argv);
    // Check correct use (This will exit if error)
    m_CommandParser->analyzeCommands();

    auto start_time = std::chrono::steady_clock::now();
    /////////////////// Argument treatement
    if (m_CommandParser->m_args.m_version) {
        m_CommandParser->getVersion();
        exit(1);
    }
    // Load RML files
    if (m_CommandParser->m_args.m_providedFile != "") {
        // load rml file
        m_Beamline = std::make_unique<RAYX::Beamline>(
            RAYX::importBeamline(m_CommandParser->m_args.m_providedFile));
    } else {
        RAYX_LOG << "No Pipeline/Beamline provided, exiting..";
        exit(1);
    }

    // Chose Hardware
    if (m_CommandParser->m_args.m_cpuFlag) {
        m_Tracer = std::make_unique<RAYX::CpuTracer>();
    } else {
        m_Tracer = std::make_unique<RAYX::VulkanTracer>();
    }

    if (m_CommandParser->m_args.m_benchmark) {
        RAYX_D_LOG << "Starting in Benchmark Mode.\n";
    }
    // Run RAY-X Core
    m_Tracer->trace(*m_Beamline);

    // Export Rays to external data.
    if (!exportRays()) {
        RAYX_ERR << "Error in exporting";
    }

    if (m_CommandParser->m_args.m_benchmark) {
        std::chrono::steady_clock::time_point end =
            std::chrono::steady_clock::now();
        RAYX_LOG << "Benchmark: Done in "
                 << std::chrono::duration_cast<std::chrono::milliseconds>(
                        end - start_time)
                        .count()
                 << " ms";
    }

    //  Plot in Python
    if (m_CommandParser->m_args.m_plotFlag) {
        // Setup to create venv if needed
        try {
            std::shared_ptr<PythonInterp> pySetup =
                std::make_shared<PythonInterp>("py_setup", "setup",
                                               (const char*)nullptr);
            pySetup->execute();
        } catch (std::exception& e) {
            RAYX_ERR << e.what();
        }
        RAYX_D_LOG << "Python Setup OK.";

        // Call PythonInterp from rayx venv:
        // *Temporary method (Calls sys python interpreter that calls rayx
        // interpreter) [Python Dynamic linking problem]
        try {
            std::shared_ptr<PythonInterp> pyPlot =
                std::make_shared<PythonInterp>("py_plot_entry", "startPlot",
                                               (const char*)nullptr);
            if (m_CommandParser->m_args.m_providedFile != "") {
                pyPlot->setPlotName(
                    m_CommandParser->m_args.m_providedFile.c_str());
            } else if (m_CommandParser->m_args.m_dummyFlag) {
                std::string _name = "Dummy Beamline";
                pyPlot->setPlotName(_name.c_str());
            }
            if (m_CommandParser->m_args.m_multiplePlots) {
                pyPlot->setPlotType(3);
            }
            pyPlot->execute();
        } catch (std::exception& e) {
            RAYX_ERR << e.what();
        }
    }
}

bool TerminalApp::exportRays() {
    bool retval = false;
    std::unique_ptr<Writer> w;

#ifdef CI
    w = std::make_unique<CSVWriter>();
    RAYX_LOG << "Using CSV Writer because of CI!";
#else
    if (m_CommandParser->m_args.m_csvFlag) {
        w = std::make_unique<CSVWriter>();
    } else {
        w = std::make_unique<H5Writer>();
    }
#endif

    size_t index = 0;
    auto doubleVecSize = RAY_MAX_ELEMENTS_IN_VECTOR * RAY_DOUBLE_COUNT;
    std::vector<double> doubleVec(doubleVecSize);

    // Transform list into double vectors for correct output.
    for (auto outputRayIterator = m_Tracer->m_OutputRays.begin(),
              outputIteratorEnd = m_Tracer->m_OutputRays.end();
         outputRayIterator != outputIteratorEnd; outputRayIterator++) {
        RAYX_D_LOG << "(*outputRayIterator).size(): "
                   << (*outputRayIterator).size();

        memcpy(doubleVec.data(), (*outputRayIterator).data(),
               (*outputRayIterator).size() * VULKANTRACER_RAY_DOUBLE_AMOUNT *
                   sizeof(double));
        doubleVec.resize((*outputRayIterator).size() *
                         VULKANTRACER_RAY_DOUBLE_AMOUNT);

        RAYX_D_LOG << "sample ray: " << doubleVec[0] << ", " << doubleVec[1]
                   << ", " << doubleVec[2] << ", " << doubleVec[3] << ", "
                   << doubleVec[4] << ", " << doubleVec[5] << ", "
                   << doubleVec[6] << ", energy: " << doubleVec[7]
                   << ", stokes 0: " << doubleVec[8];

        w->appendRays(doubleVec, index);
        index = index + (*outputRayIterator).size();
    }

    // TODO(Oussama): Add the debug buffer output too.
    retval = true;

    return retval;
}
