#include "DataType.hpp"

namespace icode
{
    static std::string dataTypeStringsArray[] = { "I8",       "UI8",  "I16",   "UI16",   "I32",    "UI32",
                                                  "I64",      "UI64", "F32",   "F64",    "VM_INT", "VM_UINT",
                                                  "VM_FLOAT", "INT",  "FLOAT", "STRUCT", "VOID" };

    bool isSignedInteger(DataType dtype)
    {
        return (dtype == I8 || dtype == I16 || dtype == I32 || dtype == I64 || dtype == VM_INT || dtype == INT);
    }

    bool isUnsignedInteger(DataType dtype)
    {
        return (dtype == UI8 || dtype == UI16 || dtype == UI32 || dtype == UI64 || dtype == VM_UINT);
    }

    bool isInteger(DataType dtype)
    {
        return (isSignedInteger(dtype) || isUnsignedInteger(dtype));
    }

    bool isFloat(DataType dtype)
    {
        return (dtype == F32 || dtype == F64 || dtype == FLOAT || dtype == VM_FLOAT);
    }

    bool dataTypeIsEqual(DataType dtype1, DataType dtype2)
    {
        return dtype1 == dtype2 || (dtype1 == INT && isInteger(dtype2)) || (isInteger(dtype1) && dtype2 == INT) ||
               (dtype1 == FLOAT && isFloat(dtype2)) || (isFloat(dtype1) && dtype2 == FLOAT);
    }

    std::string dataTypeToString(const DataType dtype)
    {
        return dataTypeStringsArray[dtype];
    }

    int getDataTypeSize(const DataType dtype)
    {
        const int dataTypeSizesArray[] = { 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 1, 1, 1, 0, 0, 0, 0 };
        return dataTypeSizesArray[dtype];
    }
}