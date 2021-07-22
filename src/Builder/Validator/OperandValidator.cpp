#include "OperandUtil.hpp"

#include "OperandValidator.hpp"

void validateNoneOperand(const icode::Operand& op, Console& con)
{
    con.check(op.dtype == icode::VOID);
    con.check(op.name.size() == 0);
}

void validateTempOrLiteralOperand(const icode::Operand& op, Console& con)
{
    con.check(nonVoidBaseDataType(op));
    con.check(op.name.size() == 0);
}

void validateTempPtrOperand(const icode::Operand& op, Console& con)
{
    con.check(validDataType(op));
    con.check(op.name.size() == 0);
}

void validateStrDataOperand(const icode::Operand& op, Console& con)
{
    con.check(op.dtype == icode::UI8);
    con.check(op.name.size() != 0);
}

void validateNameOperand(const icode::Operand& op, Console& con)
{
    con.check(op.dtype == icode::VOID);
    con.check(op.name.size() != 0);
}

void validateVaOrPtrOperand(const icode::Operand& op, Console& con)
{
    con.check(validDataType(op));
    con.check(op.name.size() != 0);
}

void validateRetOperand(const icode::Operand& op, Console& con)
{
    con.check(validDataType(op));
    con.check(op.name.size() == 0);
}

void validateOperand(const icode::Operand& op, Console& con)
{
    switch (op.operandType)
    {
        case icode::TEMP:
        case icode::ADDR:
        case icode::LITERAL:
            validateTempOrLiteralOperand(op, con);
            break;
        case icode::TEMP_PTR:
            validateTempPtrOperand(op, con);
            break;
        case icode::VAR:
        case icode::GBL_VAR:
        case icode::PTR:
            validateVaOrPtrOperand(op, con);
            break;
        case icode::RET_VALUE:
        case icode::CALLEE_RET_VAL:
            validateRetOperand(op, con);
            break;
        case icode::STR_DATA:
            validateStrDataOperand(op, con);
            break;
        case icode::LABEL:
        case icode::MODULE:
            validateNameOperand(op, con);
            break;
        case icode::NONE:
            validateNoneOperand(op, con);
            break;
        default:
            con.internalBugError();
    }
}
