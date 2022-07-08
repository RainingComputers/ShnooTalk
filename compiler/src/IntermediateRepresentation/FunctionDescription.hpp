#ifndef IR_FUNCTION_DESCRIPTION
#define IR_FUNCTION_DESCRIPTION

#include <map>
#include <vector>

#include "Entry.hpp"
#include "TypeDescription.hpp"

namespace icode
{
    struct FunctionDescription
    {
        TypeDescription functionReturnType;
        std::vector<std::string> parameters;
        std::map<std::string, TypeDescription> symbols;
        std::vector<icode::Entry> icodeTable;
        std::string moduleName; 
        std::string absoluteName;

        bool symbolExists(const std::string& name);
        bool getSymbol(const std::string& name, icode::TypeDescription& returnValue);
        size_t numParameters() const;
        bool isVoid() const;
        TypeDescription getParamType(const std::string& paramName) const;
        TypeDescription getParamTypePos(size_t paramPos) const;
    };
}

#endif
