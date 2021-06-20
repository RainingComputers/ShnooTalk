#ifndef IR_ENTRY
#define IR_ENTRY

#include "Operand.hpp"

namespace icode
{
    enum Instruction
    {
        PASS,
        PASS_ADDR,
        CALL,
        RET,
        ADDR_ADD,
        ADDR_MUL,
        EQUAL,
        READ,
        WRITE,
        CREATE_PTR,
        CAST,
        UNARY_MINUS,
        NOT,
        MUL,
        DIV,
        MOD,
        ADD,
        SUB,
        RSH,
        LSH,
        LT,
        LTE,
        GT,
        GTE,
        EQ,
        NEQ,
        BWA,
        BWX,
        BWO,
        GOTO,
        IF_TRUE_GOTO,
        IF_FALSE_GOTO,
        CREATE_LABEL,
        PRINT,
        PRINT_STR,
        SPACE,
        NEWLN,
        INPUT,
        INPUT_STR,
        EXIT
    };

    struct Entry
    {
        Instruction opcode;
        Operand op1;
        Operand op2;
        Operand op3;

        Entry();
    };
}

#endif