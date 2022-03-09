#include "PathResolver.h"

#include <Debug.h>

#include <filesystem>
#include <optional>

static std::optional<std::filesystem::path> ROOT;

void initPathResolver(char* executablePath) {
    RAYX_WARN << "executablePath=" << executablePath;
    std::filesystem::path p = std::filesystem::canonical(
        executablePath);  // rayreworked/build/bin/TerminalApp
    RAYX_WARN << "canonical=" << p;
    p = p.parent_path();  // rayreworked/build/bin/
    p = p.parent_path();  // rayreworked/build
    p = p.parent_path();  // rayreworked
    RAYX_WARN << "root=" << p;
    ROOT = p;
}

std::string resolvePath(std::string path) {
    if (!ROOT) {
        RAYX_ERR
            << "can not resolve path without prior call to initPathResolver";
    }

    std::filesystem::path p = *ROOT;
    p.append(path);
    return p;
}
