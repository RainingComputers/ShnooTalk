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
        std::map<std::string, DefineDescription> defines;
    };
}

#endif
