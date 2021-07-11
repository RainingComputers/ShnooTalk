#ifndef FILESYSTEM_FILESYSTEM
#define FILESYSTEM_FILESYSTEM

#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

namespace fs
{
    bool fileExists(const std::string& name);
    bool directoryExists(const std::string& name);
}

#endif