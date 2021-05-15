#include <algorithm>

#include "GetMapElement.hpp"

#include "ModuleDescription.hpp"

namespace icode
{
    bool ModuleDescription::use_exists(const std::string& name)
    {
        return std::find(uses.begin(), uses.end(), name) != uses.end();
    }

    bool ModuleDescription::get_struct(const std::string& name, StructDescription& val)
    {
        return GetMapElement<std::string, StructDescription>(structures, name, val);
    }

    bool ModuleDescription::get_func(const std::string& name, FunctionDescription& val)
    {
        return GetMapElement<std::string, FunctionDescription>(functions, name, val);
    }

    bool ModuleDescription::get_enum(const std::string& name, int& val)
    {
        return GetMapElement<std::string, int>(enumerations, name, val);
    }

    bool ModuleDescription::get_def(const std::string& name, Define& val)
    {
        return GetMapElement<std::string, Define>(defines, name, val);
    }

    bool ModuleDescription::get_global(const std::string& name, VariableDescription& val)
    {
        return GetMapElement<std::string, VariableDescription>(globals, name, val);
    }

    bool ModuleDescription::symbol_exists(const std::string& name, TargetDescription& target)
    {
        return structures.find(name) != structures.end() || functions.find(name) != functions.end() ||
               use_exists(name) || dataTypeFromString(name, target) != STRUCT ||
               enumerations.find(name) != enumerations.end() || globals.find(name) != globals.end() ||
               defines.find(name) != defines.end() || target.defines.find(name) != target.defines.end();
    }
}