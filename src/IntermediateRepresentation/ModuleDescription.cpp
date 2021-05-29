#include <algorithm>

#include "GetMapElement.hpp"

#include "ModuleDescription.hpp"

namespace icode
{
    void ModuleDescription::initializeTargetInfo(const TargetDescription& target)
    {
        defines = target.defines;
        dataTypeNames = target.dataTypeNames;
    }

    DataType ModuleDescription::dataTypeFromString(const std::string& dataTypeString)
    {
        if (dataTypeNames.find(dataTypeString) != dataTypeNames.end())
            return dataTypeNames[dataTypeString];

        return STRUCT;
    }

    bool ModuleDescription::useExists(const std::string& name)
    {
        return std::find(uses.begin(), uses.end(), name) != uses.end();
    }

    bool ModuleDescription::getStruct(const std::string& name, StructDescription& val)
    {
        return GetMapElement<std::string, StructDescription>(structures, name, val);
    }

    bool ModuleDescription::getFunction(const std::string& name, FunctionDescription& val)
    {
        return GetMapElement<std::string, FunctionDescription>(functions, name, val);
    }

    bool ModuleDescription::getEnum(const std::string& name, int& val)
    {
        return GetMapElement<std::string, int>(enumerations, name, val);
    }

    bool ModuleDescription::getDefineDescription(const std::string& name, DefineDescription& val)
    {
        return GetMapElement<std::string, DefineDescription>(defines, name, val);
    }

    bool ModuleDescription::getGlobal(const std::string& name, TypeDescription& val)
    {
        return GetMapElement<std::string, TypeDescription>(globals, name, val);
    }

    bool ModuleDescription::symbolExists(const std::string& name)
    {
        return structures.find(name) != structures.end() || functions.find(name) != functions.end() ||
               useExists(name) || dataTypeFromString(name) != STRUCT ||
               enumerations.find(name) != enumerations.end() || globals.find(name) != globals.end() ||
               defines.find(name) != defines.end();
    }
}