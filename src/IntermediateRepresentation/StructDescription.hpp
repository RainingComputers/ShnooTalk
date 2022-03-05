#ifndef IR_STRUCT_DESCRIPTION
#define IR_STRUCT_DESCRIPTION

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
        std::vector<std::string> fieldNames;

        bool fieldExists(const std::string&) const;
        bool getField(const std::string&, TypeDescription& returnValue);
        DataType getFirstFieldDataType() const;
    };
}

#endif