#include "GetMapElement.hpp"

#include "FunctionDescription.hpp"

namespace icode
{
    bool FunctionDescription::symbolExists(const std::string& name)
    {
        return symbols.find(name) != symbols.end();
    }

    bool FunctionDescription::getSymbol(const std::string& name, TypeDescription& typeDescription)
    {
        return GetMapElement<std::string, TypeDescription>(symbols, name, typeDescription);
    }

}