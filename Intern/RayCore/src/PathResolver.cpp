#include "PathResolver.h"

#include <Debug.h>

#include <filesystem>
#include <optional>

static std::optional<std::filesystem::path> ROOT;

void initPathResolver(char* executablePath) {
    std::filesystem::path p = std::filesystem::canonical(
        executablePath);  // ray-x/build/bin/TerminalApp
    p = p.parent_path();  // ray-x/build/bin/
    p = p.parent_path();  // ray-x/build
    p = p.parent_path();  // ray-x
    ROOT = p;
}

std::string resolvePath(std::string path) {
    if (!ROOT) {
        RAYX_ERR
            << "can not resolve path without prior call to initPathResolver";
    }

    std::filesystem::path p = *ROOT;
    p.append(path);
    return p.string();
}
