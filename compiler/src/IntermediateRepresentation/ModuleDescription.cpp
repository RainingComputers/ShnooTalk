#include <algorithm>
#include <cmath>

#include "../Utils/GetMapElement.hpp"
#include "../Utils/ItemInList.hpp"
#include "../Utils/KeyExistsInMap.hpp"

#include "ModuleDescription.hpp"

namespace icode
{
    ModuleDescription::ModuleDescription()
    {
        intDefines = { { "false", 0 }, { "true", 1 }, { "NULL", 0 } };

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

    bool ModuleDescription::getEnum(const std::string& name, EnumDescription& returnValue)
    {
        return getMapElement<std::string, EnumDescription>(enums, name, returnValue);
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

    bool ModuleDescription::getIncompleteTypeModule(const std::string& typeName, std::string& returnValue)
    {
        return getMapElement<std::string, std::string>(incompleteTypes, typeName, returnValue);
    }

    bool ModuleDescription::getIncompleteFunctionModule(const std::string& typeName, std::string& returnValue)
    {
        return getMapElement<std::string, std::string>(incompleteFunctions, typeName, returnValue);
    }

    bool ModuleDescription::enumTypeExists(const std::string& name) const
    {
        return keyExistsInMap(definedEnumsTypes, name);
    }

    bool ModuleDescription::useExists(const std::string& name) const
    {
        return itemInList(name, uses);
    }

    bool ModuleDescription::aliasExists(const std::string& name) const
    {
        return keyExistsInMap(aliases, name);
    }

    bool ModuleDescription::functionExists(const std::string& name) const
    {
        return keyExistsInMap(functions, name);
    }

    bool ModuleDescription::symbolExists(const std::string& name) const
    {
        return keyExistsInMap(structures, name) || keyExistsInMap(functions, name) || enumTypeExists(name) ||
               stringToDataType(name) != STRUCT || keyExistsInMap(enums, name) || keyExistsInMap(globals, name) ||
               keyExistsInMap(intDefines, name) || keyExistsInMap(floatDefines, name) ||
               keyExistsInMap(stringDefines, name) || keyExistsInMap(externFunctions, name);
    }
}