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

    void TypeDescription::becomeMutable()
    {
        setProperty(IS_MUT);
    }

    void TypeDescription::becomePointer()
    {
        setProperty(IS_PTR);
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

    // TODO move this
    TypeDescription typeDescriptionFromDataType(DataType dtype)
    {
        TypeDescription var;
        var.dtype = dtype;
        var.dtypeName = dataTypeToString(dtype);
        var.dtypeSize = getDataTypeSize(dtype);
        var.size = var.dtypeSize;
        var.offset = 0;
        return var;
    }
}