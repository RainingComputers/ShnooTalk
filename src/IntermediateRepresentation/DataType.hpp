#ifndef IR_DATA_TYPE
#define IR_DATA_TYPE

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
        AUTO_INT,
        AUTO_FLOAT,
        STRUCT,
        VOID
    };

    bool isSignedInteger(DataType);
    bool isUnsignedInteger(DataType);
    bool isInteger(DataType);
    bool isFloat(DataType);

    std::string dataTypeToString(const DataType dtype);
    DataType stringToDataType(const std::string& dtypeName);
    int getDataTypeSize(const DataType dtype);
    bool dataTypeIsEqual(DataType dtype1, DataType dtype2);

}

#endif