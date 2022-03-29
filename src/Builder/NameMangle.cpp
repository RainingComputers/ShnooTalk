#include <algorithm>

#include "NameMangle.hpp"

std::string mangleModuleName(const std::string& moduleName)
{
    std::string mangledModuleName = moduleName;
    std::replace(mangledModuleName.begin(), mangledModuleName.end(), '/', '_');
    std::replace(mangledModuleName.begin(), mangledModuleName.end(), '.', '_');

    return mangledModuleName;
}

std::string lineColNameMangle(const Token& token, const std::string& moduleName)
{
    return "_str_" + mangleModuleName(moduleName) + '_' + token.getLineColString();
}

std::string nameMangleString(const std::string& name, const std::string& moduleName)
{
    return "_fn_" + mangleModuleName(moduleName) + '_' + name;
}

std::string nameMangle(const Token& token, const std::string& moduleName)
{
    if (token.toString() == "main")
        return "main";

    return "_fn_" + mangleModuleName(moduleName) + '_' + token.toFunctionNameString();
}
