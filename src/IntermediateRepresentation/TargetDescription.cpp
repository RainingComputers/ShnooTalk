#include "GetMapElement.hpp"

#include "TargetDescription.hpp"

namespace icode
{
    DataType dataTypeFromString(const std::string& dataTypeString, TargetDescription& target)
    {
        if (target.dataTypeNames.find(dataTypeString) != target.dataTypeNames.end())
            return target.dataTypeNames[dataTypeString];

        return STRUCT;
    }

    bool TargetDescription::getDefine(const std::string& name, Define& val)
    {
        return GetMapElement<std::string, Define>(defines, name, val);
    }
}