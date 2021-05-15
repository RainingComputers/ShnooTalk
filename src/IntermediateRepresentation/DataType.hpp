#ifndef DATA_TYPE_HPP
#define DATA_TYPE_HPP

#include <string>

namespace icode
{
    enum DataType
    {
        I8,
        UI8,
        I16,
        UI16,
        I32,
        UI32,
        I64,
        UI64,
        F32,
        F64,
        VM_INT,
        VM_UINT,
        VM_FLOAT,
        INT,
        FLOAT,
        STRUCT,
        VOID
    };

    bool isSignedInteger(DataType);
    bool isUnsignedInteger(DataType);
    bool isInteger(DataType);
    bool isFloat(DataType);
    bool dataTypeIsEqual(DataType dtype1, DataType dtype2);

    std::string dataTypeToString(const DataType dtype);
    int getDataTypeSize(const DataType dtype);

}

#endif