#ifndef TARGET_DESCRIPTION_HPP
#define TARGET_DESCRIPTION_HPP

#include <map>

#include "DataType.hpp"
#include "Define.hpp"

namespace icode
{
    struct TargetDescription
    {
        std::map<std::string, DataType> dataTypeNames;
        std::map<std::string, Define> defines;
        DataType characterInt;

        bool getDefine(const std::string&, Define&);
    };

    DataType dataTypeFromString(const std::string& dtype_name, TargetDescription& target);
}

#endif
