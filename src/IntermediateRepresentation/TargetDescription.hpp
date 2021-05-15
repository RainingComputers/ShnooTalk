#ifndef TARGET_DESCRIPTION_HPP
#define TARGET_DESCRIPTION_HPP

#include <map>

#include "DataType.hpp"
#include "Define.hpp"
#include "VariableDescription.hpp"

namespace icode
{
    struct TargetDescription
    {
        std::map<std::string, DataType> dataTypeNames;
        std::map<std::string, Define> defines;
        DataType characterInt;

        bool get_def(const std::string&, Define&);
    };

    DataType dataTypeFromString(const std::string& dtype_name, TargetDescription& target);
    
    VariableDescription variableDescriptionFromDataType(DataType dtype, TargetDescription& target);
}

#endif
