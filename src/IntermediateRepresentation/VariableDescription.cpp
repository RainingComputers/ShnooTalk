#include "VariableDescription.hpp"

namespace icode
{
    VariableDescription::VariableDescription()
    {
        properties = 0;
    }

    void VariableDescription::setProperty(VariableProperty property)
    {
        properties |= (1 << property);
    }

    void VariableDescription::clearProperty(VariableProperty property)
    {
        properties &= ~(1 << property);
    }

    bool VariableDescription::checkProperty(VariableProperty property) const
    {
        return properties & (1 << property);
    }

    bool VariableDescription::isStruct() const
    {
        return dtype == STRUCT;
    }

    bool VariableDescription::isArray() const
    {
        return dimensions.size() > 0;
    }

    bool isSameType(VariableDescription var1, VariableDescription var2)
    {
        if (var1.dtype == STRUCT || var2.dtype == STRUCT)
            return (var1.dtypeName == var2.dtypeName && var1.dimensions == var2.dimensions &&
                    var1.moduleName == var2.moduleName);

        return (dataTypeIsEqual(var1.dtype, var2.dtype) && var1.dimensions == var2.dimensions);
    }

    // TODO remove this
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

        return var;
    }
}