#include <algorithm>

#include "GetMapElement.hpp"

#include "ModuleDescription.hpp"

namespace icode
{
    void ModuleDescription::initializeTarget(const TargetEnums& target)
    {
        defines = target.defines;
    }

    bool ModuleDescription::useExists(const std::string& name)
    {
        return std::find(uses.begin(), uses.end(), name) != uses.end();
    }

    bool ModuleDescription::getStruct(const std::string& name, StructDescription& returnValue)
    {
        return getMapElement<std::string, StructDescription>(structures, name, returnValue);
    }

    bool ModuleDescription::getFunction(const std::string& name, FunctionDescription& returnValue)
    {
        return getMapElement<std::string, FunctionDescription>(functions, name, returnValue);
    }

    bool ModuleDescription::getEnum(const std::string& name, int& returnValue)
    {
        return getMapElement<std::string, int>(enumerations, name, returnValue);
    }

    bool ModuleDescription::getDefineDescription(const std::string& name, DefineDescription& returnValue)
    {
        return getMapElement<std::string, DefineDescription>(defines, name, returnValue);
    }

    bool ModuleDescription::getGlobal(const std::string& name, TypeDescription& returnValue)
    {
        return getMapElement<std::string, TypeDescription>(globals, name, returnValue);
    }

    bool ModuleDescription::symbolExists(const std::string& name)
    {
        return structures.find(name) != structures.end() || functions.find(name) != functions.end() ||
               useExists(name) || stringToDataType(name) != STRUCT ||
               enumerations.find(name) != enumerations.end() || globals.find(name) != globals.end() ||
               defines.find(name) != defines.end();
    }
}