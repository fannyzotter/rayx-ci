#include "TerminalApp.h"

#include <filesystem>
#include <memory>
#include <stdexcept>

#include "Debug.h"
#include "PathResolver.h"
#include "Tracer/CpuTracer.h"
#include "Tracer/VulkanTracer.h"
#include "Writer/Writer.h"

TerminalApp::TerminalApp(int argc, char** argv) : m_argv(argv), m_argc(argc) {
    initPathResolver();
    RAYX_D_LOG << "TerminalApp created!";

    /// warn if the binary is compiled with 32-bit (i.e. sizeof(void*) == 4)
    /// or worse.
    /// 64-bit has sizeof(void*) == 8.
    if (sizeof(void*) <= 4) {
        RAYX_WARN
            << "This application should be compiled as 64-bit! Using 32-bit"
               "will make this program run out of memory for larger ray "
               "numbers!";
    }
}

TerminalApp::~TerminalApp() { RAYX_D_LOG << "TerminalApp deleted!"; }

void TerminalApp::tracePath(std::filesystem::path path) {
    namespace fs = std::filesystem;
    if (!fs::exists(path)) {
        if (path.empty()) {
            RAYX_ERR << "No input file provided!";
        } else {
            RAYX_ERR << "File '" << path << "' not found!";
        }
    }

    if (fs::is_directory(path)) {
        for (auto p : fs::directory_iterator(path)) {
            tracePath(p.path());
        }
    } else if (path.extension() == ".rml") {
        // Load RML file
        m_Beamline =
            std::make_unique<RAYX::Beamline>(RAYX::importBeamline(path));

        // Run RAY-X Core
        auto rays = m_Tracer->trace(*m_Beamline);

        // Export Rays to external data.
        exportRays(rays, path.string());

        // Plot
        if (m_CommandParser->m_args.m_plotFlag) {
            std::shared_ptr<RAYX::Plotter> plotter =
                std::make_shared<RAYX::Plotter>();
            if (m_CommandParser->m_args.m_multiplePlots) {
                plotter->plot(2, path.string(), rays);
            } else
                plotter->plot(0, path.string(), rays);
        }

#if defined(RAYX_DEBUG_MODE) && not defined(CPP)
        // Export Debug Matrics.
        exportDebug();
#endif

    } else {
        RAYX_LOG << "ignoring non-rml file: '" << path << "'";
    }
}

void TerminalApp::run() {
    RAYX_PROFILE_FUNCTION();

    RAYX_D_LOG << "TerminalApp running...";

    /////////////////// Argument Parser
    m_CommandParser = std::make_unique<CommandParser>(m_argc, m_argv);
    // Check correct use (This will exit if error)
    m_CommandParser->analyzeCommands();

    auto start_time = std::chrono::steady_clock::now();
    if (m_CommandParser->m_args.m_benchmark) {
        RAYX_D_LOG << "Starting in Benchmark Mode.\n";
    }
    /////////////////// Argument treatement
    if (m_CommandParser->m_args.m_version) {
        m_CommandParser->getVersion();
        exit(1);
    }

    // Choose Hardware
    if (m_CommandParser->m_args.m_cpuFlag) {
        m_Tracer = std::make_unique<RAYX::CpuTracer>();
    } else {
        m_Tracer = std::make_unique<RAYX::VulkanTracer>();
    }
    // Trace, export and plot
    tracePath(m_CommandParser->m_args.m_providedFile);

    if (m_CommandParser->m_args.m_benchmark) {
        std::chrono::steady_clock::time_point end =
            std::chrono::steady_clock::now();
        RAYX_LOG << "Benchmark: Done in "
                 << std::chrono::duration_cast<std::chrono::milliseconds>(
                        end - start_time)
                        .count()
                 << " ms";
    }
}

void TerminalApp::exportRays(const std::vector<RAYX::Ray>& rays,
                             std::string path) {
#ifdef CI
    bool csv = true;
#else
    bool csv = m_CommandParser->m_args.m_csvFlag;
#endif

    // strip .rml
    if (path.ends_with(".rml")) {
        path = path.substr(0, path.length() - 4);
    }

    if (csv) {
        writeCSV(rays, path + ".csv");
    } else {
#ifndef CI  // writeH5 is not defined in the CI!
        writeH5(rays, path + ".h5");
#endif
    }
}

#if defined(RAYX_DEBUG_MODE) && not defined(CPP)
/**
 * @brief Gets All Debug Buffers and check if they are the identity matrix.
 * This is a default function to show how the implemented Debug Buffer works.
 * You can write your own checking func.
 *
 * Debugging Matrices are only available on Vulkan Tracing In Debug mode
 *
 */
void TerminalApp::exportDebug() {
    if (m_CommandParser->m_args.m_cpuFlag) {
        return;
    }
    auto d = (const RAYX::VulkanTracer*)(m_Tracer.get());
    int index = 0;
    RAYX_D_LOG << "Debug Matrix Check...";
    for (auto m : d->getDebugList()) {
        if (isIdentMatrix(m._dMat)) {
            printDMat4(m._dMat);
            RAYX_D_ERR << "@" << index;
        }
        index += 1;
    }
}
#endif
