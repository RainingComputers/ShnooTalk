#include "EntryBuilder.hpp"

using namespace icode;

EntryBuilder::EntryBuilder(OperandBuilder& opBuilder)
  : opBuilder(opBuilder)
{
}

void EntryBuilder::setWorkingFunction(FunctionDescription* functionDesc)
{
    workingFunction = functionDesc;
}

void EntryBuilder::pushEntry(Entry entry)
{
    (*workingFunction).icodeTable.push_back(entry);
}

Operand EntryBuilder::getCreatePointerDestinationOperand(const Operand& op,
                                                         const std::string& dtypeName,
                                                         ModuleDescription* workingModule)
{
    /* If not a struct, just copy the operand but change its type to a pointer */

    if (op.dtype != STRUCT)
        return opBuilder.createPointerOperand(op.dtype);

    /* If it a struct, create pointer to the first field */

    TypeDescription firstFieldDesc = workingModule->structures[dtypeName].structFields.begin()->second;

    return opBuilder.createPointerOperand(firstFieldDesc.dtype);
}

Operand EntryBuilder::createPointer(const Operand& op, const std::string& dtypeName, ModuleDescription* workingModule)
{
    if (op.operandType == TEMP_PTR && op.dtype != STRUCT)
        return op;

    /* Converted TEMP_PTR */
    Operand pointerOperand = getCreatePointerDestinationOperand(op, dtypeName, workingModule);

    /* Construct CREATE_PTR instruction */
    Entry createPointerEntry;

    createPointerEntry.opcode = CREATE_PTR;
    createPointerEntry.op1 = pointerOperand;
    createPointerEntry.op2 = op;

    pushEntry(createPointerEntry);

    return pointerOperand;
}

void EntryBuilder::copy(Operand op1, Operand op2)
{
    /* If op2 is a literal, change generic dtypes like INT and FLOAT
        to correct specific dtype */
    if (op2.operandType == LITERAL)
        op2.dtype = op1.dtype;

    /* Copy one operand value to another, use READ and WRITE instruction
        if pointers are involved */

    if (op1.isPointer() && op2.isPointer())
    {
        Operand temp = opBuilder.createTempOperand(op2.dtype);
        copy(temp, op2);
        copy(op1, temp);
    }
    else
    {
        Entry copyEntry;

        if (op1.isPointer() && !op2.isPointer())
            copyEntry.opcode = WRITE;
        else if (!op1.isPointer() && op2.isPointer())
            copyEntry.opcode = READ;
        else if (!op1.isPointer() && !op2.isPointer())
            copyEntry.opcode = EQUAL;

        copyEntry.op1 = op1;
        copyEntry.op2 = op2;

        pushEntry(copyEntry);
    }
}

Operand EntryBuilder::ensureNotPointer(Operand op)
{
    /* Make sure the operand is not a pointer, if it is a pointer,
        converts it to a temp using the READ instruction */

    if (!op.isPointer())
        return op;

    Operand temp = opBuilder.createTempOperand(op.dtype);
    copy(temp, op);
    return temp;
}

Operand EntryBuilder::pushEntryAndEnsureNoPointerWrite(Entry entry)
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
    Operand pointerOperand = entry.op1;
    Operand temp = opBuilder.createTempOperand(pointerOperand.dtype);

    /* Replace TEMP_PTR with TEMP */
    Entry modifiedEntry = entry;
    modifiedEntry.op1 = temp;
    pushEntry(modifiedEntry);

    /* Create WRITE instruction to write the TEMP to TEMP_PTR */
    Entry writeEntry;

    writeEntry.op1 = pointerOperand;
    writeEntry.op2 = temp;
    writeEntry.opcode = WRITE;

    pushEntry(writeEntry);

    return temp;
}

Operand EntryBuilder::binaryOperator(Instruction instruction, Operand op1, Operand op2, Operand op3)
{
    /* Construct icode instruction for binary operator instructions,
        ADD, SUB, MUL, DIV, MOD, RSH, LSH, BWA, BWO, BWX */

    Entry entry;

    entry.opcode = instruction;
    entry.op1 = op1;
    entry.op2 = ensureNotPointer(op2);
    entry.op3 = ensureNotPointer(op3);

    return pushEntryAndEnsureNoPointerWrite(entry);
}

Operand EntryBuilder::unaryOperator(Instruction instruction, Operand op1, Operand op2)
{
    /* Construct icode for unary operator instructions,
        UNARY_MINUS and NOT  */

    Entry entry;

    entry.opcode = instruction;
    entry.op1 = op1;
    entry.op2 = ensureNotPointer(op2);

    return pushEntryAndEnsureNoPointerWrite(entry);
}

Operand EntryBuilder::castOperator(DataType castDataType, Operand op)
{
    /* Construct icode for CAST */

    Entry entry;

    entry.opcode = CAST;
    entry.op1 = opBuilder.createTempOperand(castDataType);
    entry.op2 = ensureNotPointer(op);

    return pushEntryAndEnsureNoPointerWrite(entry);
}

void EntryBuilder::compareOperator(Instruction instruction, Operand op1, Operand op2)
{
    /* If op2 is a literal, change generic dtypes like INT and FLOAT
        to correct specific dtype */
    if (op2.operandType == LITERAL)
        op2.dtype = op1.dtype;

    /* Construct icode for comparator operator instructions,
        EQ, NEQ, LT, LTE, GT, GTE  */

    Entry entry;

    entry.opcode = instruction;
    entry.op1 = ensureNotPointer(op1);
    entry.op2 = ensureNotPointer(op2);

    pushEntry(entry);
}

Operand EntryBuilder::addressAddOperator(Operand op2, Operand op3)
{
    /* Construct icode for ADDR_ADD */

    Entry entry;

    entry.opcode = ADDR_ADD;
    entry.op1 = opBuilder.createPointerOperand(op2.dtype);
    entry.op2 = op2;
    entry.op3 = op3;

    pushEntry(entry);

    return entry.op1;
}

Operand EntryBuilder::addressMultiplyOperator(Operand op2, Operand op3)
{
    /* Construct icode for ADDR_MUL */

    Entry entry;

    entry.opcode = ADDR_MUL;
    entry.op1 = opBuilder.createPointerOperand(VOID);
    entry.op2 = ensureNotPointer(op2);
    entry.op3 = op3;

    pushEntry(entry);

    return entry.op1;
}

void EntryBuilder::label(Operand op)
{
    /* Construct CREATE_LABEL */

    Entry labelEntry;

    labelEntry.opcode = CREATE_LABEL;
    labelEntry.op1 = op;

    pushEntry(labelEntry);
}

void EntryBuilder::createBranch(Instruction instruction, Operand op)
{
    /* Construct icode for GOTO, IF_TRUE_GOTO, IF_FALSE_GOTO */

    Entry branchEntry;

    branchEntry.opcode = instruction;
    branchEntry.op1 = op;

    pushEntry(branchEntry);
}

void EntryBuilder::printOperator(Instruction printInstruction, Operand op)
{
    /* Construct icode for PRINT, PRINT_STR */

    Entry printEntry;

    printEntry.opcode = printInstruction;

    if (printInstruction == PRINT)
        printEntry.op1 = ensureNotPointer(op);
    else
        printEntry.op1 = op;

    pushEntry(printEntry);
}

void EntryBuilder::inputOperator(Instruction inputInstruction, Operand op, unsigned int size)
{
    /* Construct icode for INPUT, INPUT_STR */

    Entry inputEntry;

    inputEntry.opcode = inputInstruction;
    inputEntry.op1 = op;
    inputEntry.op2 = opBuilder.createIntLiteralOperand(INT, (int)size);

    pushEntry(inputEntry);
}

void EntryBuilder::pass(Instruction passInstruction,
                        Operand op,
                        const std::string& functionName,
                        const FunctionDescription& functionDesc)
{
    /* Construct icode for PASS and PASS_ADDR instructions */

    DataType functionDataType = functionDesc.functionReturnType.dtype;

    Entry entry;

    entry.opcode = passInstruction;

    if (passInstruction == PASS)
        entry.op1 = ensureNotPointer(op);
    else
        entry.op1 = op;

    entry.op2 = opBuilder.createVarOperand(functionDataType, functionName);
    entry.op3 = opBuilder.createModuleOperand(functionDesc.moduleName);

    pushEntry(entry);
}

Operand EntryBuilder::call(const std::string& functionName, const FunctionDescription& functionDesc)
{
    /* Construct icode for CALL instruction */

    DataType functionDataType = functionDesc.functionReturnType.dtype;

    Entry callEntry;

    callEntry.opcode = CALL;
    callEntry.op1 = opBuilder.createCalleeRetValOperand(functionDataType);
    callEntry.op2 = opBuilder.createVarOperand(functionDataType, functionName);
    callEntry.op3 = opBuilder.createModuleOperand(functionDesc.moduleName);

    pushEntry(callEntry);

    return callEntry.op1;
}

void EntryBuilder::noArgumentEntry(Instruction instruction)
{
    /* Construct icode for instructions with no arguments,
        RET, SPACE, NEWLN, EXIT */

    Entry entry;

    entry.opcode = instruction;

    pushEntry(entry);
}

bool EntryBuilder::doesFunctionTerminate()
{
    if (workingFunction->icodeTable.size() < 1)
        return false;

    icode::Instruction lastOpcode = workingFunction->icodeTable.back().opcode;
    return lastOpcode == icode::RET;
}

bool EntryBuilder::terminateFunction()
{
    if (doesFunctionTerminate())
        return true;

    if (!workingFunction->isVoid())
        return false;

    noArgumentEntry(icode::RET);
    return true;
}