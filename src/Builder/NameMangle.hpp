#ifndef BUILDER_NAME_MANGLE
#define BUILDER_NAME_MANGLE

#include "../Token/Token.hpp"

std::string mangleModuleName(const std::string& moduleName);
std::string nameMangle(const Token& token, const std::string& moduleName);
std::string lineColNameMangle(const Token& token, const std::string& moduleName);

#endif