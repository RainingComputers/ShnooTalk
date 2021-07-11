#include "FileSystem.hpp"

namespace fs
{
    bool fileExists(const std::string& name)
    {
        struct stat info;

        if (stat(name.c_str(), &info) != 0)
            return false;
        else if (info.st_mode & S_IFREG)
            return true;

        return false;
    }

    bool directoryExists(const std::string& name)
    {
        struct stat info;

        if (stat(name.c_str(), &info) != 0)
            return false;
        else if (info.st_mode & S_IFDIR)
            return true;

        return false;
    }
} // namespace pathchk
