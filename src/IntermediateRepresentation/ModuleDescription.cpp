#include <algorithm>

#include "GetMapElement.hpp"

#include "ModuleDescription.hpp"

namespace icode
{
    void ModuleDescription::initializeTarget(const TargetEnums& target)
    {
        intDefines = target.intDefines;
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

    bool ModuleDescription::getIntDefine(const std::string& name, int& returnValue)
    {
        return getMapElement<std::string, int>(intDefines, name, returnValue);
    }

    bool ModuleDescription::getFloatDefine(const std::string& name, float& returnValue)
    {
        return getMapElement<std::string, float>(floatDefines, name, returnValue);
    }

    bool ModuleDescription::getStringDefine(const std::string& name, std::string& returnValue)
    {
        return getMapElement<std::string, std::string>(stringDefines, name, returnValue);
    }

    bool ModuleDescription::getGlobal(const std::string& name, TypeDescription& returnValue)
    {
        return getMapElement<std::string, TypeDescription>(globals, name, returnValue);
    }

    bool ModuleDescription::symbolExists(const std::string& name)
    {
        return structures.find(name) != structures.end() || functions.find(name) != functions.end() ||
               useExists(name) || stringToDataType(name) != STRUCT || enumerations.find(name) != enumerations.end() ||
               globals.find(name) != globals.end() || intDefines.find(name) != intDefines.end() ||
               floatDefines.find(name) != floatDefines.end() || stringDefines.find(name) != stringDefines.end();
    }
}