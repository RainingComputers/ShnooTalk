#include <algorithm>

#include "NameMangle.hpp"

std::string nameMangle(const Token& token, const std::string& moduleName)
{
    if (token.toString() == "main")
        return "main";

    std::string mangledModuleName = moduleName;
    std::replace(mangledModuleName.begin(), mangledModuleName.end(), '/', '_');

    return mangledModuleName + '_' + token.toString();
}