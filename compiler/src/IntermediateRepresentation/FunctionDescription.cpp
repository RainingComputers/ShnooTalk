#include "../Utils/GetMapElement.hpp"
#include "../Utils/ItemInList.hpp"
#include "../Utils/KeyExistsInMap.hpp"

#include "FunctionDescription.hpp"

namespace icode
{
    bool FunctionDescription::symbolExists(const std::string& name)
    {
        return keyExistsInMap(symbols, name);
    }

    bool FunctionDescription::getSymbol(const std::string& name, TypeDescription& returnValue)
    {
        return getMapElement<std::string, TypeDescription>(symbols, name, returnValue);
    }

    size_t FunctionDescription::numParameters() const
    {
        return parameters.size();
    }

    bool FunctionDescription::isVoid() const
    {
        return functionReturnType.dtype == VOID;
    }

    TypeDescription FunctionDescription::getParamType(const std::string& paramName) const
    {
        return symbols.at(paramName);
    }

    TypeDescription FunctionDescription::getParamTypePos(size_t paramPos) const
    {
        return symbols.at(parameters.at(paramPos));
    }

}