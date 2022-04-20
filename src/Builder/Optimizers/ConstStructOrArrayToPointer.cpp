#include "ConstStructOrArrayToPointer.hpp"

void optimize(const std::string& symbolName, OperandBuilder& opBuilder, std::vector<icode::Entry>& icodeTable)
{
    unsigned int lastCreatePointerTempId = 0;

    for (icode::Entry& entry : icodeTable)
    {
        if (entry.opcode == icode::CREATE_PTR && entry.op2.name == symbolName)
        {
            entry.op2.operandType = icode::PTR;
            lastCreatePointerTempId = entry.op1.operandId;
        }

        if (entry.opcode == icode::MEMCPY && entry.op1.operandId == lastCreatePointerTempId)
        {
            entry.opcode = icode::PTR_ASSIGN;
            entry.op3.operandType = icode::NONE;
            entry.op1 = opBuilder.createVarOperand(icode::STRUCT, symbolName, icode::PTR);
        }
    }
}

void constStructsToPointer(OperandBuilder& opBuilder, icode::FunctionDescription& functionDesc)
{
    for (auto& symbol : functionDesc.symbols)
    {
        icode::TypeDescription& type = symbol.second;

        if (type.isMutableOrPointer() || type.isArray() || !type.isStruct())
            continue;

        type.becomePointer();
        optimize(symbol.first, opBuilder, functionDesc.icodeTable);
    }
}