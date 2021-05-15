#ifndef STRUCT_DESCRIPTION_HPP
#define STRUCT_DESCRIPTION_HPP

#include <map>
#include <string>

#include "VariableDescription.hpp"

namespace icode
{
    struct StructDescription
    {
        std::map<std::string, VariableDescription> structFields;
        unsigned int size;
        std::string moduleName;

        bool fieldExists(const std::string&);
    };
}

#endif