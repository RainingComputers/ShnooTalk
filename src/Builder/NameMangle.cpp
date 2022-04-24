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

std::string unMangleString(const std::string& name, const std::string& moduleName)
{
    const size_t prefixLength = std::string("_fn_").length();
    const size_t unMangleLength = moduleName.size() + prefixLength + 1;

    /* This condition is true for names that are not mangled, like the main function for example */
    if (unMangleLength >= name.size())
        return name;

    const std::string unMangled = name.substr(unMangleLength);

    return unMangled;
}
