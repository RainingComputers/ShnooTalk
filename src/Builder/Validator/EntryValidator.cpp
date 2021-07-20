#include "EntryUtil.hpp"
#include "OperandUtil.hpp"
#include "OperandValidator.hpp"

#include "EntryValidator.hpp"

void equal(const icode::Entry& e, Console& con)
{
    con.check(twoOperand(e));
    con.check(notPointer(e.op1) || retValue(e.op1));
    con.check(notPointer(e.op2));
    con.check(allOperandNonVoidBaseDataType(e));
}

void binaryOperater(const icode::Entry& e, Console& con)
{
    con.check(threeOperand(e));
    con.check(temp(e.op1));
    con.check(allOperandNonPointer(e));
    con.check(allOperandEqualDataType(e));
    con.check(allOperandNonVoidBaseDataType(e));
}

void compareOperater(const icode::Entry& e, Console& con)
{
    con.check(twoOperand(e));
    con.check(allOperandNonPointer(e));
    con.check(allOperandEqualDataType(e));
    con.check(allOperandNonVoidBaseDataType(e));
}

void unaryOrCastOperator(const icode::Entry& e, Console& con)
{
    con.check(twoOperand(e));
    con.check(allOperandNonPointer(e));
    con.check(allOperandNonVoidBaseDataType(e));
}

void branch(const icode::Entry& e, Console& con)
{
    con.check(oneOperand(e));
    con.check(label(e.op1));
}

void pass(const icode::Entry& e, Console& con)
{
    con.check(threeOperand(e));
    con.check(notPointer(e.op1));
    con.check(var(e.op2));
    con.check(mod(e.op3));
}

void validateEntry(const icode::Entry& entry, Console& con)
{
    validateOperand(entry.op1, con);
    validateOperand(entry.op2, con);
    validateOperand(entry.op3, con);

    switch (entry.opcode)
    {
        case icode::EQUAL:
            equal(entry, con);
            break;
        case icode::ADD:
        case icode::SUB:
        case icode::MUL:
        case icode::DIV:
        case icode::MOD:
        case icode::LSH:
        case icode::RSH:
        case icode::BWA:
        case icode::BWO:
        case icode::BWX:
            binaryOperater(entry, con);
            break;
        case icode::NOT:
        case icode::UNARY_MINUS:
        case icode::CAST:
            unaryOrCastOperator(entry, con);
            break;
        case icode::EQ:
        case icode::NEQ:
        case icode::LT:
        case icode::LTE:
        case icode::GT:
        case icode::GTE:
            compareOperater(entry, con);
            break;
        case icode::CREATE_LABEL:
        case icode::IF_TRUE_GOTO:
        case icode::IF_FALSE_GOTO:
        case icode::GOTO:
            branch(entry, con);
            break;
        case icode::CREATE_PTR:
            break;
        case icode::ADDR_ADD:
        case icode::ADDR_MUL:
            break;
        case icode::READ:
            break;
        case icode::WRITE:
            break;
        case icode::PRINT:
            break;
        case icode::PRINT_STR:
            break;
        case icode::PASS:
            pass(entry, con);
            break;
        case icode::PASS_ADDR:
            break;
        case icode::CALL:
            break;
        case icode::INPUT:
            break;
        case icode::INPUT_STR:
            break;
        case icode::MEMCPY:
            break;
        case icode::NEWLN:
        case icode::SPACE:
        case icode::RET:
        case icode::EXIT:
            con.check(noOperand(entry));
            break;
        default:
            con.internalBugError();
    }
}