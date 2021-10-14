#include <algorithm>
#include <cmath>

#include "GetMapElement.hpp"

#include "ModuleDescription.hpp"

namespace icode
{
    void ModuleDescription::initializeTarget(const TargetEnums& target)
    {
        intDefines = target.intDefines;

        intDefines = { { "false", 0 }, { "true", 1 } };

        floatDefines = { { "nan", NAN }, { "infinity", INFINITY } };
    }

    bool ModuleDescription::getModuleNameFromAlias(const std::string& alias, std::string& returnValue)
    {
        return getMapElement<std::string, std::string>(aliases, alias, returnValue);
    }

    bool ModuleDescription::getStruct(const std::string& name, StructDescription& returnValue)
    {
        return getMapElement<std::string, StructDescription>(structures, name, returnValue);
    }

    bool ModuleDescription::getFunction(const std::string& name, FunctionDescription& returnValue)
    {
        return getMapElement<std::string, FunctionDescription>(functions, name, returnValue);
    }

    bool ModuleDescription::getExternFunction(const std::string& name, FunctionDescription& returnValue)
    {
        return getMapElement<std::string, FunctionDescription>(externFunctions, name, returnValue);
    }

    bool ModuleDescription::getEnum(const std::string& name, int& returnValue)
    {
        return getMapElement<std::string, int>(enumerations, name, returnValue);
    }

    bool ModuleDescription::getIntDefine(const std::string& name, long& returnValue)
    {
        return getMapElement<std::string, long>(intDefines, name, returnValue);
    }

    bool ModuleDescription::getFloatDefine(const std::string& name, double& returnValue)
    {
        return getMapElement<std::string, double>(floatDefines, name, returnValue);
    }

    bool ModuleDescription::getStringDefine(const std::string& name, std::string& returnValue)
    {
        return getMapElement<std::string, std::string>(stringDefines, name, returnValue);
    }

    bool ModuleDescription::getGlobal(const std::string& name, TypeDescription& returnValue)
    {
        return getMapElement<std::string, TypeDescription>(globals, name, returnValue);
    }

    bool ModuleDescription::useExists(const std::string& name)
    {
        return std::find(uses.begin(), uses.end(), name) != uses.end();
    }

    bool ModuleDescription::aliasExists(const std::string& name)
    {
        return aliases.find(name) != aliases.end();
    }

    bool ModuleDescription::symbolExists(const std::string& name)
    {
        return structures.find(name) != structures.end() || functions.find(name) != functions.end() ||
               stringToDataType(name) != STRUCT || enumerations.find(name) != enumerations.end() ||
               globals.find(name) != globals.end() || intDefines.find(name) != intDefines.end() ||
               floatDefines.find(name) != floatDefines.end() || stringDefines.find(name) != stringDefines.end() ||
               externFunctions.find(name) != externFunctions.end();
    }
}