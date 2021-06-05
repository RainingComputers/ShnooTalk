#ifndef STRUCT_DESCRIPTION_HPP
#define STRUCT_DESCRIPTION_HPP

#include <map>
#include <string>

#include "TypeDescription.hpp"

namespace icode
{
    struct StructDescription
    {
        std::map<std::string, TypeDescription> structFields;
        unsigned int size;
        std::string moduleName;

        bool fieldExists(const std::string&);
        bool getField(const std::string&, TypeDescription& returnValue);
    };
}

#endif