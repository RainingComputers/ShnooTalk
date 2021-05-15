#include "GetMapElement.hpp"

#include "TargetDescription.hpp"

namespace icode
{
    VariableDescription variableDescriptionFromDataType(DataType dtype, TargetDescription& target)
    {
        VariableDescription var;

        var.dtype = dtype;

        if (dtype == INT)
            var.dtypeName = "int";
        else if (dtype == FLOAT)
            var.dtypeName = "float";
        else if (dtype == VOID)
            var.dtypeName = "void";
        else
        {
            for (auto pair : target.dataTypeNames)
                if (pair.second == dtype)
                    var.dtypeName = pair.first;
        }

        var.dtypeSize = getDataTypeSize(dtype);
        var.size = var.dtypeSize;
        var.offset = 0;
        var.scopeId = 0;

        return var;
    }

    DataType dataTypeFromString(const std::string& dtype_name, TargetDescription& target)
    {
        if (target.dataTypeNames.find(dtype_name) != target.dataTypeNames.end())
            return target.dataTypeNames[dtype_name];

        return STRUCT;
    }

    bool TargetDescription::get_def(const std::string& name, Define& val)
    {
        return GetMapElement<std::string, Define>(defines, name, val);
    }
}