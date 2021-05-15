#ifndef FUNCTION_DESCRIPTION_HPP
#define FUNCTION_DESCRIPTION_HPP

#include <map>
#include <vector>

#include "Entry.hpp"
#include "VariableDescription.hpp"

namespace icode
{
    struct FunctionDescription
    {
        VariableDescription functionReturnDescription;
        std::vector<std::string> parameters;
        std::map<std::string, VariableDescription> symbols;
        std::vector<icode::Entry> icodeTable;
        std::string moduleName;

        bool symbolExists(const std::string&);
        bool getSymbol(const std::string&, icode::VariableDescription&);
    };
}

#endif
