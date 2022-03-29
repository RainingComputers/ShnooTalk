#include "../Utils/GetMapElement.hpp"
#include "../Utils/ItemInList.hpp"

#include "FunctionDescription.hpp"

namespace icode
{
    bool FunctionDescription::symbolExists(const std::string& name)
    {
        return symbols.find(name) != symbols.end();
    }

    bool FunctionDescription::getSymbol(const std::string& name, TypeDescription& returnValue)
    {
        return getMapElement<std::string, TypeDescription>(symbols, name, returnValue);
    }

    size_t FunctionDescription::numParameters() const
    {
        return parameters.size();
    }

    bool FunctionDescription::isParameter(const std::string& name) const
    {
        return itemInList(name, parameters);
    }

    bool FunctionDescription::isVoid() const
    {
        return functionReturnType.dtype == VOID;
    }

}