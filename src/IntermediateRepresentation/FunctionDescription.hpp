#ifndef FUNCTION_DESCRIPTION_HPP
#define FUNCTION_DESCRIPTION_HPP

#include <map>
#include <vector>

#include "Entry.hpp"
#include "TypeDescription.hpp"

namespace icode
{
    struct FunctionDescription
    {
        TypeDescription functionReturnDescription;
        std::vector<std::string> parameters;
        std::map<std::string, TypeDescription> symbols;
        std::vector<icode::Entry> icodeTable;
        std::string moduleName;

        bool symbolExists(const std::string& name);
        bool getSymbol(const std::string& name, icode::TypeDescription& typeDescription);
    };
}

#endif
