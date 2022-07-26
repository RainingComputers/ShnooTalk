#ifndef IR_ENTRY
#define IR_ENTRY

#include "Operand.hpp"

namespace icode
{
    enum Instruction
    {
        PASS,
        PASS_PTR,
        PASS_PTR_PTR,
        CALL,
        RET,
        ADDR_ADD,
        ADDR_MUL,
        EQUAL,
        PTR_ASSIGN,
        READ,
        WRITE,
        CREATE_PTR,
        ALLOC_PTR,
        ALLOC_ARRAY_PTR,
        CAST,
        ADDR,
        PTR_CAST,
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
        MEMCPY
    };

    struct Entry
    {
        Instruction opcode;
        Operand op1;
        Operand op2;
        Operand op3;
    };
}

#endif