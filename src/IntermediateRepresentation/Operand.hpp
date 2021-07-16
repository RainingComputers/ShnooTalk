#ifndef IR_OPERAND
#define IR_OPERAND

#include "DataType.hpp"
#include "TypeDescription.hpp"

namespace icode
{

    enum OperandType
    {
        TEMP,
        TEMP_PTR,
        ADDR,
        STR_DATA,
        VAR,
        GBL_VAR,
        PTR,
        RET_VALUE,
        CALLEE_RET_VAL,
        LITERAL,
        LABEL,
        MODULE,
        NONE,
    };

    struct Operand
    {
        unsigned int operandId;

        union
        {
            int integer;
            float floating;
            unsigned int address;
            unsigned int size;
        } val;

        std::string name;
        DataType dtype;
        OperandType operandType;

        bool operator<(const Operand& other) const;
        bool operator==(const Operand& other) const;
        bool isPointer() const;
        bool canPassAsMutable() const;
        bool isInvalidForInput() const;
    };
}

#endif