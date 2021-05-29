#include "TypeDescription.hpp"

namespace icode
{
    TypeDescription::TypeDescription()
    {
        properties = 0;
    }

    void TypeDescription::setProperty(TypeProperties property)
    {
        properties |= (1 << property);
    }

    void TypeDescription::clearProperty(TypeProperties property)
    {
        properties &= ~(1 << property);
    }

    bool TypeDescription::checkProperty(TypeProperties property) const
    {
        return properties & (1 << property);
    }

    bool TypeDescription::isStruct() const
    {
        return dtype == STRUCT;
    }

    bool TypeDescription::isArray() const
    {
        return dimensions.size() > 0;
    }

    bool isSameType(TypeDescription var1, TypeDescription var2)
    {
        if (var1.dtype == STRUCT || var2.dtype == STRUCT)
            return (var1.dtypeName == var2.dtypeName && var1.dimensions == var2.dimensions &&
                    var1.moduleName == var2.moduleName);

        return (dataTypeIsEqual(var1.dtype, var2.dtype) && var1.dimensions == var2.dimensions);
    }

    // TODO remove this
    TypeDescription typeDescriptionFromDataType(DataType dtype, TargetDescription& target)
    {
        TypeDescription var;

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