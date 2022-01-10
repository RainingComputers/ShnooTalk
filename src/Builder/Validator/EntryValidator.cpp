#include "EntryUtil.hpp"
#include "OperandUtil.hpp"
#include "OperandValidator.hpp"

#include "EntryValidator.hpp"

void equal(const icode::Entry& e, Console& con)
{
    con.check(twoOperand(e));
    con.check(e.op1.isNotPointer());
    con.check(e.op2.isNotPointer());
    con.check(allOperandNonVoidBaseDataType(e));
}

void pointerAssign(const icode::Entry& e, Console& con)
{
    con.check(twoOperand(e));
    con.check(e.op1.isUserPointer() || e.op1.operandType == icode::RET_PTR);
    con.check(allOperandNonVoidDataType(e));
}

void binaryOperater(const icode::Entry& e, Console& con)
{
    con.check(threeOperand(e));
    con.check(e.op1.operandType == icode::TEMP);
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

void pointerCastOperator(const icode::Entry& e, Console& con)
{
    con.check(twoOperand(e));
    con.check(e.op1.operandType == icode::TEMP_PTR);
    con.check(e.op2.isValidForPointerAssignment() || (e.op2.isNotPointer() && icode::isInteger(e.op2.dtype)));
    con.check(allOperandNonVoidDataType(e));
}

void branch(const icode::Entry& e, Console& con)
{
    con.check(oneOperand(e));
    con.check(e.op1.operandType == icode::LABEL);
}

void print(const icode::Entry& e, Console& con)
{
    con.check(oneOperand(e));
    con.check(e.op1.isNotPointer());
    con.check(nonVoidBaseDataType(e.op1));
}

void printString(const icode::Entry& e, Console& con)
{
    con.check(oneOperand(e));
    con.check(e.op2.operandType != icode::MODULE && e.op2.operandType != icode::LABEL);
    con.check(e.op1.dtype == icode::UI8);
}

void createPointer(const icode::Entry& e, Console& con)
{
    con.check(twoOperand(e));
    con.check(e.op1.isPointer());
    con.check(e.op2.operandType != icode::MODULE && e.op2.operandType != icode::LABEL);
    con.check(nonVoidDataType(e.op1));
    con.check(nonVoidDataType(e.op2));
}

void allocPointer(const icode::Entry& e, Console& con)
{
    con.check(oneOperand(e));
    con.check(e.op1.isPointer());
    con.check(allOperandNonVoidBaseDataType(e));
}

void allocArrayPointer(const icode::Entry& e, Console& con)
{
    con.check(twoOperand(e));
    con.check(e.op1.isPointer());
    con.check(e.op2.operandType == icode::BYTES);
    con.check(allOperandNonVoidBaseDataType(e));
}

void addrAdd(const icode::Entry& e, Console& con)
{
    con.check(threeOperand(e));
    con.check(e.op1.operandType == icode::TEMP_PTR || e.op1.operandType == icode::TEMP_PTR_PTR);
    con.check(e.op2.isPointer());
    con.check(e.op3.operandType == icode::TEMP_PTR || e.op3.operandType == icode::BYTES);
    con.check(nonVoidDataType(e.op1));
    con.check(nonVoidDataType(e.op2));
    con.check(e.op3.dtype == icode::VOID || e.op3.dtype == icode::AUTO_INT);
}

void addrMul(const icode::Entry& e, Console& con)
{
    con.check(threeOperand(e));
    con.check(e.op1.operandType == icode::TEMP_PTR);
    con.check(e.op2.isNotPointer());
    con.check(e.op3.operandType == icode::BYTES);
    con.check(e.op1.dtype == icode::VOID);
    con.check(icode::isInteger(e.op2.dtype));
    con.check(e.op3.dtype == icode::AUTO_INT);
}

void read(const icode::Entry& e, Console& con)
{
    con.check(twoOperand(e));
    con.check(e.op1.isNotPointer());
    con.check(e.op2.isPointer());
    con.check(allOperandNonVoidBaseDataType(e));
}

void write(const icode::Entry& e, Console& con)
{
    con.check(twoOperand(e));
    con.check(e.op1.isPointer());
    con.check(e.op2.isNotPointer());
    con.check(allOperandNonVoidBaseDataType(e));
}

void pass(const icode::Entry& e, Console& con)
{
    con.check(threeOperand(e));
    con.check(e.op1.isNotPointer());
    con.check(e.op2.operandType == icode::VAR);
    con.check(e.op3.operandType == icode::MODULE);
    con.check(nonVoidBaseDataType(e.op1));
}

void passAddress(const icode::Entry& e, Console& con)
{
    con.check(threeOperand(e));
    con.check(e.op1.isPointer() || e.op1.operandType == icode::STR_DATA);
    con.check(e.op2.operandType == icode::VAR);
    con.check(e.op3.operandType == icode::MODULE);
    con.check(nonVoidDataType(e.op1));
}

void passPointer(const icode::Entry& e, Console& con)
{
    con.check(threeOperand(e));
    con.check(e.op1.isUserPointer());
    con.check(e.op2.operandType == icode::VAR);
    con.check(e.op3.operandType == icode::MODULE);
    con.check(nonVoidDataType(e.op1));
}

void call(const icode::Entry& e, Console& con)
{
    con.check(threeOperand(e));
    con.check(e.op1.operandType == icode::CALLEE_RET_VAL || e.op1.operandType == icode::CALLEE_RET_PTR);
    con.check(e.op2.operandType == icode::VAR);
    con.check(e.op3.operandType == icode::MODULE);
}

void input(const icode::Entry& e, Console& con)
{
    con.check(twoOperand(e));
    con.check(e.op1.isValidForPointerAssignment());
    con.check(e.op2.operandType == icode::LITERAL);
    con.check(nonVoidBaseDataType(e.op1));
    con.check(e.op2.dtype == icode::AUTO_INT);
}

void inputString(const icode::Entry& e, Console& con)
{
    con.check(twoOperand(e));
    con.check(e.op1.isValidForPointerAssignment());
    con.check(e.op2.operandType == icode::LITERAL);
    con.check(e.op1.dtype == icode::UI8);
    con.check(e.op2.dtype == icode::AUTO_INT);
}

void memoryCopy(const icode::Entry& e, Console& con)
{
    con.check(threeOperand(e));
    con.check(e.op1.isPointer());
    con.check(e.op2.isPointer());
    con.check(e.op3.operandType == icode::LITERAL);
    con.check(nonVoidDataType(e.op1));
    con.check(nonVoidDataType(e.op2));
    con.check(e.op3.dtype == icode::AUTO_INT);
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
        case icode::PTR_ASSIGN:
            pointerAssign(entry, con);
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
        case icode::PTR_CAST:
            pointerCastOperator(entry, con);
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
            createPointer(entry, con);
            break;
        case icode::ALLOC_PTR:
            allocPointer(entry, con);
            break;
        case icode::ALLOC_ARRAY_PTR:
            allocArrayPointer(entry, con);
            break;
        case icode::ADDR_ADD:
            addrAdd(entry, con);
            break;
        case icode::ADDR_MUL:
            addrMul(entry, con);
            break;
        case icode::READ:
            read(entry, con);
            break;
        case icode::WRITE:
            write(entry, con);
            break;
        case icode::PRINT:
            print(entry, con);
            break;
        case icode::PRINT_STR:
            printString(entry, con);
            break;
        case icode::PASS:
            pass(entry, con);
            break;
        case icode::PASS_PTR:
            passAddress(entry, con);
            break;
        case icode::PASS_PTR_PTR:
            passPointer(entry, con);
            break;
        case icode::CALL:
            call(entry, con);
            break;
        case icode::INPUT:
            input(entry, con);
            break;
        case icode::INPUT_STR:
            inputString(entry, con);
            break;
        case icode::MEMCPY:
            memoryCopy(entry, con);
            break;
        case icode::NEWLN:
        case icode::SPACE:
        case icode::RET:
            con.check(noOperand(entry));
            break;
        default:
            con.internalBugError();
    }
}
