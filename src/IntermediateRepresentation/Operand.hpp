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
        BYTES,
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
            long integer;
            double floating;
            unsigned long bytes;
            unsigned long size;
        } val;

        std::string name;
        DataType dtype;
        OperandType operandType;

        Operand();
        bool isPointer() const;
        bool isNotPointer() const;
        bool isUserPointer() const;
        bool isValidForPointerAssignment() const;
    };
}

#endif