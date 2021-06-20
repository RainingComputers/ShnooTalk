#ifndef PATHCHK
#define PATHCHK

#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

namespace pathchk
{
    bool file_exists(const std::string& name);
    bool dir_exists(const std::string& name);
}

#endif