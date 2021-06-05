#include "EntryBuilder.hpp"

EntryBuilder::EntryBuilder(icode::ModuleDescription& rootModule,
                           icode::StringModulesMap& modulesMap,
                           OperandBuilder& opBuilder)
  : rootModule(rootModule)
  , modulesMap(modulesMap)
  , opBuilder(opBuilder)
{
}

void EntryBuilder::setFunctionDescription(icode::FunctionDescription* functionDesc)
{
    functionDescriptionPointer = functionDesc;
}

void EntryBuilder::pushEntry(icode::Entry entry)
{
    (*functionDescriptionPointer).icodeTable.push_back(entry);
}

icode::Operand EntryBuilder::getCreatePointerDestinationOperand(const icode::Operand& op)
{
    /* If not a struct, just copy the operand but change its type to a pointer */

    if (op.dtype != icode::STRUCT)
        return opBuilder.createPointerOperand(op.dtype, op.dtypeName);

    /* If it a struct, create pointer to the first field */

    icode::TypeDescription firstFieldDesc = rootModule.structures[op.dtypeName].structFields.begin()->second;

    return opBuilder.createPointerOperand(firstFieldDesc.dtype, firstFieldDesc.dtypeName);
}

icode::Operand EntryBuilder::createPointer(const icode::Operand& op)
{
    if(op.operandType == icode::TEMP_PTR && op.dtype != icode::STRUCT) return op;
    /* Converts op to TEMP_PTR using the CREATE_PTR instruction */

    /* Converted TEMP_PTR */
    icode::Operand pointerOperand = getCreatePointerDestinationOperand(op);

    /* Construct CREATE_PTR instruction */
    icode::Entry createPointerEntry;

    createPointerEntry.opcode = icode::CREATE_PTR;
    createPointerEntry.op1 = pointerOperand;
    createPointerEntry.op2 = op;

    pushEntry(createPointerEntry);

    return pointerOperand;
}

void EntryBuilder::copy(icode::Operand op1, icode::Operand op2)
{
    /* If op2 is a literal, change generic dtypes like icode::INT and icode::FLOAT
        to correct specific dtype */
    if (op2.operandType == icode::LITERAL)
        op2.dtype = op1.dtype;

    /* Copy one operand value to another, use READ and WRITE instruction
        if pointers are involved */

    if (op1.isPointer() && op2.isPointer())
    {
        icode::Operand temp = opBuilder.createTempOperand(op2.dtype, op2.dtypeName);
        copy(temp, op2);
        copy(op1, temp);
    }
    else
    {
        icode::Entry copy_entry;

        if (op1.isPointer() && !op2.isPointer())
            copy_entry.opcode = icode::WRITE;
        else if (!op1.isPointer() && op2.isPointer())
            copy_entry.opcode = icode::READ;
        else if (!op1.isPointer() && !op2.isPointer())
            copy_entry.opcode = icode::EQUAL;

        copy_entry.op1 = op1;
        copy_entry.op2 = op2;

        pushEntry(copy_entry);
    }
}

icode::Operand EntryBuilder::ensureNotPointer(icode::Operand op)
{
    /* Make sure the operand is not a pointer, if it is a pointer,
        converts it to a temp using the READ instruction */

    if (!op.isPointer())
        return op;

    icode::Operand temp = opBuilder.createTempOperand(op.dtype, op.dtypeName);
    copy(temp, op);
    return temp;
}

icode::Operand EntryBuilder::pushEntryAndEnsureNoPointerWrite(icode::Entry entry)
{
    /* Push an ir entry to the current function's icode table,
        but ensures entry.op1 is not a pointer */

    if (!entry.op1.isPointer())
    {
        pushEntry(entry);
        return entry.op1;
    }

    /* If entry.op1 is a pointer, replace it with a temp and
        write that temp to the pointer */

    /* Create corresponding TEMP to TEMP_PTR  */
    icode::Operand pointerOperand = entry.op1;
    icode::Operand temp = opBuilder.createTempOperand(pointerOperand.dtype, pointerOperand.dtypeName);

    /* Replace TEMP_PTR with TEMP */
    icode::Entry modifiedEntry = entry;
    modifiedEntry.op1 = temp;
    pushEntry(modifiedEntry);

    /* Create WRITE instruction to write the TEMP to TEMP_PTR */
    icode::Entry writeEntry;

    writeEntry.op1 = pointerOperand;
    writeEntry.op2 = temp;
    writeEntry.opcode = icode::WRITE;

    pushEntry(writeEntry);

    return temp;
}

icode::Operand EntryBuilder::binaryOperator(icode::Instruction instruction,
                                            icode::Operand op1,
                                            icode::Operand op2,
                                            icode::Operand op3)
{
    /* Construct icode instruction for binary operator instructions,
        ADD, SUB, MUL, DIV, MOD, RSH, LSH, BWA, BWO, BWX */

    icode::Entry entry;

    entry.opcode = instruction;
    entry.op1 = op1;
    entry.op2 = ensureNotPointer(op2);
    entry.op3 = ensureNotPointer(op3);

    return pushEntryAndEnsureNoPointerWrite(entry);
}

icode::Operand EntryBuilder::unaryOperator(icode::Instruction instruction, icode::Operand op1, icode::Operand op2)
{
    /* Construct icode for unary operator instructions,
        UNARY_MINUS and NOT  */

    icode::Entry entry;

    entry.opcode = instruction;
    entry.op1 = op1;
    entry.op2 = ensureNotPointer(op2);

    return pushEntryAndEnsureNoPointerWrite(entry);
}

icode::Operand EntryBuilder::castOperator(icode::DataType castDataType, icode::Operand op)
{
    /* Construct icode for CAST */

    icode::Entry entry;

    entry.opcode = icode::CAST;
    entry.op1 = opBuilder.createTempOperand(castDataType, icode::dataTypeToString(castDataType));
    entry.op2 = ensureNotPointer(op);

    return pushEntryAndEnsureNoPointerWrite(entry);
}

void EntryBuilder::compareOperator(icode::Instruction instruction, icode::Operand op1, icode::Operand op2)
{
    /* If op2 is a literal, change generic dtypes like icode::INT and icode::FLOAT
        to correct specific dtype */
    if (op2.operandType == icode::LITERAL)
        op2.dtype = op1.dtype;

    /* Construct icode for comparator operator instructions,
        EQ, NEQ, LT, LTE, GT, GTE  */

    icode::Entry entry;

    entry.opcode = instruction;
    entry.op1 = ensureNotPointer(op1);
    entry.op2 = ensureNotPointer(op2);

    pushEntry(entry);
}

icode::Operand EntryBuilder::addressAddOperator(icode::Operand op2, icode::Operand op3)
{
    /* Construct icode for ADDR_ADD */

    icode::Entry entry;

    entry.opcode = icode::ADDR_ADD;
    entry.op1 = opBuilder.createPointerOperand(op2.dtype, op2.dtypeName);
    entry.op2 = op2;
    entry.op3 = op3;

    pushEntry(entry);

    return entry.op1;
}

icode::Operand EntryBuilder::addressMultiplyOperator(icode::Operand op2, icode::Operand op3)
{
    /* Construct icode for ADDR_MUL */

    icode::Entry entry;

    entry.opcode = icode::ADDR_MUL;
    entry.op1 = opBuilder.createPointerOperand(icode::VOID, icode::dataTypeToString(icode::VOID));
    entry.op2 = ensureNotPointer(op2);
    entry.op3 = op3;

    pushEntry(entry);

    return entry.op1;
}

void EntryBuilder::label(icode::Operand op)
{
    /* Construct CREATE_LABEL */

    icode::Entry labelEntry;

    labelEntry.opcode = icode::CREATE_LABEL;
    labelEntry.op1 = op;

    pushEntry(labelEntry);
}

void EntryBuilder::createBranch(icode::Instruction instruction, icode::Operand op)
{
    /* Construct icode for GOTO, IF_TRUE_GOTO, IF_FALSE_GOTO */

    icode::Entry branchEntry;

    branchEntry.opcode = instruction;
    branchEntry.op1 = op;

    pushEntry(branchEntry);
}

void EntryBuilder::printOperator(icode::Instruction printInstruction, icode::Operand op)
{
    /* Construct icode for PRINT, PRINT_STR */

    icode::Entry printEntry;

    printEntry.opcode = printInstruction;

    if (printInstruction == icode::PRINT)
        printEntry.op1 = ensureNotPointer(op);
    else
        printEntry.op1 = op;

    pushEntry(printEntry);
}

void EntryBuilder::inputOperator(icode::Instruction inputInstruction, icode::Operand op, unsigned int size)
{
    /* Construct icode for INPUT, INPUT_STR */

    icode::Entry inputEntry;

    inputEntry.opcode = inputInstruction;
    inputEntry.op1 = op;
    inputEntry.op2 = opBuilder.createIntLiteralOperand(icode::INT, (int)size);

    pushEntry(inputEntry);
}

void EntryBuilder::pass(icode::Instruction passInstruction,
                        icode::Operand op,
                        const std::string& functionName,
                        const icode::FunctionDescription& functionDesc)
{
    /* Construct icode for PASS and PASS_ADDR instructions */

    icode::DataType functionDataType = functionDesc.functionReturnDescription.dtype;
    std::string functionDataTypeName = functionDesc.functionReturnDescription.dtypeName;

    icode::Entry entry;

    entry.opcode = passInstruction;

    if (passInstruction == icode::PASS)
        entry.op1 = ensureNotPointer(op);
    else
        entry.op1 = op;

    entry.op2 = opBuilder.createVarOperand(functionDataType, functionDataTypeName, functionName);
    entry.op3 = opBuilder.createModuleOperand(functionDesc.moduleName);

    pushEntry(entry);
}

icode::Operand EntryBuilder::call(const std::string& functionName, const icode::FunctionDescription& functionDesc)
{
    /* Construct icode for CALL instruction */

    icode::DataType functionDataType = functionDesc.functionReturnDescription.dtype;
    std::string functionDataTypeName = functionDesc.functionReturnDescription.dtypeName;

    icode::Entry callEntry;

    callEntry.opcode = icode::CALL;
    callEntry.op1 = opBuilder.createCalleeRetValOperand(functionDataType, functionDataTypeName);
    callEntry.op2 = opBuilder.createVarOperand(functionDataType, functionDataTypeName, functionName);
    callEntry.op3 = opBuilder.createModuleOperand(functionDesc.moduleName);

    pushEntry(callEntry);

    return callEntry.op1;
}

void EntryBuilder::noArgumentEntry(icode::Instruction instruction)
{
    /* Construct icode for instructions with no arguments,
        RET, SPACE, NEWLN, EXIT */

    icode::Entry entry;

    entry.opcode = instruction;

    pushEntry(entry);
}