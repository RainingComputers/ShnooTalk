#include <cstdlib>
#include <filesystem>

#include "../config.hpp"

#include "ModuleSearch.hpp"

namespace fs = std::filesystem;

bool pathExists(const fs::path& path)
{
    std::error_code ec{};
    return std::filesystem::exists(path, ec);
}

std::string getAbsoluteModulePath(const std::string& relativePath)
{
    if (pathExists(fs::path(relativePath)))
        return relativePath;

    const char* shnootalkPath = getenv(SHNOOTALK_PATH_ENV_NAME);
    if (shnootalkPath != nullptr)
    {
        fs::path absoluteShnootalkPath = fs::path(shnootalkPath) / relativePath;

        if (pathExists(absoluteShnootalkPath))
            return absoluteShnootalkPath.string();
    }

    fs::path defaultLibPath = fs::path(DEFAULT_LIB_PATH) / (std::string("shnootalk-") + VERSION);
    fs::path absoluteDefaultPath = defaultLibPath / relativePath;

    if (pathExists(absoluteDefaultPath))
        return absoluteDefaultPath.string();

    return "";
}
