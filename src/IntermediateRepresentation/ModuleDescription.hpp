#ifndef MODULE_DESCRIPTION_HPP
#define MODULE_DESCRIPTION_HPP

#include <map>
#include <string>
#include <vector>

#include "Define.hpp"
#include "FunctionDescription.hpp"
#include "StructDescription.hpp"
#include "TargetDescription.hpp"

namespace icode
{
    struct ModuleDescription
    {
        std::string name;
        std::vector<std::string> uses;
        std::map<std::string, StructDescription> structures;
        std::map<std::string, FunctionDescription> functions;
        std::map<std::string, int> enumerations;
        std::map<std::string, Define> defines;
        std::map<std::string, VariableDescription> globals;
        std::map<std::string, std::string> str_data;

        bool use_exists(const std::string&);
        bool get_struct(const std::string&, StructDescription&);
        bool get_func(const std::string&, FunctionDescription&);
        bool get_enum(const std::string&, int&);
        bool get_def(const std::string&, Define&);
        bool get_global(const std::string&, VariableDescription&);
        bool symbol_exists(const std::string&, TargetDescription& target);
    };

    typedef std::map<std::string, ModuleDescription> StringModulesMap;

}

#endif