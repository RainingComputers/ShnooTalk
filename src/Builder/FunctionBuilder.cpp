#include "NameMangle.hpp"
#include "TypeDescriptionUtil.hpp"
#include "Validator/EntryValidator.hpp"

#include "FunctionBuilder.hpp"

using namespace icode;

FunctionBuilder::FunctionBuilder(StringModulesMap& modulesMap,
                                 OperandBuilder& opBuilder,
                                 UnitBuilder& unitBuilder,
                                 Console& console)
  : modulesMap(modulesMap)
  , opBuilder(opBuilder)
  , unitBuilder(unitBuilder)
  , console(console)
{
}

void FunctionBuilder::setWorkingFunction(FunctionDescription* functionDesc)
{
    workingFunction = functionDesc;
}

void FunctionBuilder::pushEntry(Entry entry)
{
    validateEntry(entry, console);

    (*workingFunction).icodeTable.push_back(entry);
}

Operand FunctionBuilder::getCreatePointerDestinationOperand(const Unit& unit)
{
    /* If not a struct, just copy the operand but change its type to a pointer */

    if (!unit.isStruct())
        return opBuilder.createPointerOperand(unit.dtype());

    /* If it a struct, create pointer to the first field */
    ModuleDescription* workingModule = &modulesMap.at(unit.moduleName());

    TypeDescription firstFieldDesc = workingModule->structures.at(unit.dtypeName()).structFields.begin()->second;

    return opBuilder.createPointerOperand(firstFieldDesc.dtype);
}

Operand FunctionBuilder::createPointer(const Unit& unit)
{
    if (unit.isPointer() && !unit.isStruct())
        return unit.op();

    /* Converted TEMP_PTR */
    Operand pointerOperand = getCreatePointerDestinationOperand(unit);

    /* Construct CREATE_PTR instruction */
    Entry createPointerEntry;

    createPointerEntry.opcode = CREATE_PTR;
    createPointerEntry.op1 = pointerOperand;
    createPointerEntry.op2 = unit.op();

    pushEntry(createPointerEntry);

    return pointerOperand;
}

Operand FunctionBuilder::autoCast(const Operand& op, DataType destinationDataType)
{
    /* This compiler does not support implicit type casting, this function
        is meant for casting to and from AUTO_INT and AUTO_FLOAT datatypes
        and explicit casting */

    /* Literals are sometimes have AUTO_INT and AUTO_FLOAT data types, and adding
        them will lead to more temp operands having AUTO datatypes */

    /* Pointers will never have the AUTO data types */

    if (op.dtype == destinationDataType)
        return op;

    Entry entry;

    entry.opcode = CAST;
    entry.op1 = opBuilder.createTempOperand(destinationDataType);
    entry.op2 = op;

    pushEntry(entry);

    return entry.op1;
}

void FunctionBuilder::operandCopy(Operand dest, Operand src)
{
    Operand castedSrc = autoCast(src, dest.dtype);

    /* Copy one operand value to another, use READ and WRITE instruction
        if pointers are involved */

    if (dest.isPointer() && castedSrc.isPointer())
    {
        Operand temp = opBuilder.createTempOperand(castedSrc.dtype);
        operandCopy(temp, castedSrc);
        operandCopy(dest, temp);
    }
    else
    {
        Entry copyEntry;

        if (dest.isPointer() && !castedSrc.isPointer())
            copyEntry.opcode = WRITE;
        else if (!dest.isPointer() && castedSrc.isPointer())
            copyEntry.opcode = READ;
        else if (!dest.isPointer() && !castedSrc.isPointer())
            copyEntry.opcode = EQUAL;

        copyEntry.op1 = dest;
        copyEntry.op2 = castedSrc;

        pushEntry(copyEntry);
    }
}

void FunctionBuilder::memCopy(Operand op1, Operand op2, int numBytes)
{
    Entry memCpyEntry;

    memCpyEntry.op1 = op1;
    memCpyEntry.op2 = op2;
    memCpyEntry.op3 = opBuilder.createIntLiteralOperand(I64, numBytes);
    memCpyEntry.opcode = MEMCPY;

    pushEntry(memCpyEntry);
}

void FunctionBuilder::unitListCopy(const Unit& dest, const Unit& src)
{
    Operand destPointer = createPointer(dest);

    std::vector<Unit> unitsToCopy = src.flatten();

    for (size_t i = 0; i < unitsToCopy.size(); i++)
    {
        const Unit& unit = unitsToCopy[i];

        if (unit.isArray() || unit.isStruct())
            memCopy(destPointer, createPointer(unit), unit.size());
        else
            operandCopy(destPointer, unit.op());

        if (i == unitsToCopy.size() - 1)
            break;

        /* Move to next element */
        int updateSize = dest.dtypeSize();

        if (unit.isStringLtrl())
            updateSize *= dest.dimensions().back();

        Operand update = opBuilder.createLiteralAddressOperand(updateSize);
        destPointer = addressAddOperator(destPointer, update);
    }
}

void FunctionBuilder::unitCopy(const Unit& dest, const Unit& src)
{
    if (src.isList())
        unitListCopy(dest, src);
    else if (dest.isArray() || dest.isStruct())
    {
        Operand destPointer = createPointer(dest);
        Operand srcPointer = createPointer(src);

        memCopy(destPointer, srcPointer, src.size());
    }
    else
        operandCopy(dest.op(), src.op());
}

Operand FunctionBuilder::ensureNotPointer(Operand op)
{
    /* Make sure the operand is not a pointer, if it is a pointer,
        converts it to a temp using the READ instruction */

    if (!op.isPointer())
        return op;

    Operand temp = opBuilder.createTempOperand(op.dtype);
    operandCopy(temp, op);
    return temp;
}

Operand FunctionBuilder::pushEntryAndEnsureNoPointerWrite(Entry entry)
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

Unit FunctionBuilder::binaryOperator(Instruction instruction, const Unit& LHS, const Unit& RHS)
{
    /* Construct icode instruction for binary operator instructions,
        ADD, SUB, MUL, DIV, MOD, RSH, LSH, BWA, BWO, BWX */

    DataType dtype = LHS.dtype();

    Entry entry;

    entry.opcode = instruction;
    entry.op1 = opBuilder.createTempOperand(dtype);
    entry.op2 = ensureNotPointer(LHS.op());
    entry.op3 = autoCast(ensureNotPointer(RHS.op()), dtype);

    Operand result = pushEntryAndEnsureNoPointerWrite(entry);

    return Unit(LHS.type(), result);
}

Unit FunctionBuilder::unaryOperator(Instruction instruction, const Unit& unaryOperatorTerm)
{
    /* Construct icode for unary operator instructions,
        UNARY_MINUS and NOT  */

    const DataType dtype = unaryOperatorTerm.dtype();

    Entry entry;

    entry.opcode = instruction;
    entry.op1 = opBuilder.createTempOperand(dtype);
    entry.op2 = ensureNotPointer(unaryOperatorTerm.op());

    Operand result = pushEntryAndEnsureNoPointerWrite(entry);

    return Unit(unaryOperatorTerm.type(), result);
}

Unit FunctionBuilder::castOperator(const Unit& unitToCast, DataType destinationDataType)
{
    /* Construct icode for CAST */
    Operand result = autoCast(ensureNotPointer(unitToCast.op()), destinationDataType);

    return Unit(typeDescriptionFromDataType(destinationDataType), result);
}

void FunctionBuilder::compareOperator(Instruction instruction, const Unit& LHS, const Unit& RHS)
{
    /* Construct icode for comparator operator instructions,
        EQ, NEQ, LT, LTE, GT, GTE  */

    DataType dtype = LHS.dtype();

    Entry entry;

    entry.opcode = instruction;
    entry.op1 = ensureNotPointer(LHS.op());
    entry.op2 = autoCast(ensureNotPointer(RHS.op()), dtype);

    pushEntry(entry);
}

Operand FunctionBuilder::addressAddOperator(Operand op2, Operand op3)
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

Operand FunctionBuilder::addressMultiplyOperator(Operand op2, Operand op3)
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

Unit FunctionBuilder::getStructField(const Token& fieldName, const Unit& unit)
{
    StructDescription structDescription = modulesMap[unit.moduleName()].structures[unit.dtypeName()];

    TypeDescription fieldType;

    if (!structDescription.getField(fieldName.toString(), fieldType))
        console.compileErrorOnToken("Undefined STRUCT field", fieldName);

    if (unit.isMutable())
        fieldType.becomeMutable();

    Operand pointerOperand = createPointer(unit);

    Operand fieldOperand = addressAddOperator(pointerOperand, opBuilder.createLiteralAddressOperand(fieldType.offset));

    fieldOperand.dtype = fieldType.dtype;

    return Unit(fieldType, fieldOperand);
}

Unit FunctionBuilder::getIndexedElement(const Unit& unit, const std::vector<Unit>& indices)
{
    unsigned int dimensionCount = 0;
    unsigned int elementWidth = unit.size() / unit.dimensions()[0];

    Operand elementOperand = createPointer(unit);

    TypeDescription elementType = unit.type();

    for (const Unit& indexUnit : indices)
    {
        Operand subscriptOperand =
          addressMultiplyOperator(indexUnit.op(), opBuilder.createLiteralAddressOperand(elementWidth));

        if (dimensionCount + 1 != elementType.dimensions.size())
            elementWidth /= elementType.dimensions[dimensionCount + 1];

        elementOperand = addressAddOperator(elementOperand, subscriptOperand);

        dimensionCount++;
    }

    unsigned int remainingDimensionCount = elementType.dimensions.size() - dimensionCount;

    elementType.dimensions.erase(elementType.dimensions.begin(),
                                 elementType.dimensions.end() - remainingDimensionCount);

    return Unit(elementType, elementOperand);
}

Operand FunctionBuilder::createLabel(const Token& tok, bool isTrueLabel, std::string prefix)
{
    std::string label_name = tok.getLineColString();

    return opBuilder.createLabelOperand("_" + prefix + "_" + (isTrueLabel ? "true" : "false") + label_name);
}

void FunctionBuilder::insertLabel(Operand op)
{
    /* Construct CREATE_LABEL */

    Entry labelEntry;

    labelEntry.opcode = CREATE_LABEL;
    labelEntry.op1 = op;

    pushEntry(labelEntry);
}

void FunctionBuilder::createBranch(Instruction instruction, Operand op)
{
    /* Construct icode for GOTO, IF_TRUE_GOTO, IF_FALSE_GOTO */

    Entry branchEntry;

    branchEntry.opcode = instruction;
    branchEntry.op1 = op;

    pushEntry(branchEntry);
}

void FunctionBuilder::createPrint(const Unit& unit)
{
    /* Construct icode for PRINT, PRINT_STR */

    Entry printEntry;

    Instruction printInstruction = unit.isArray() ? PRINT_STR : PRINT;

    printEntry.opcode = printInstruction;

    if (printInstruction == PRINT)
        printEntry.op1 = ensureNotPointer(unit.op());
    else
        printEntry.op1 = unit.op();

    pushEntry(printEntry);
}

void FunctionBuilder::createInput(const Unit& unit)
{
    /* Construct icode for INPUT, INPUT_STR */

    Entry inputEntry;

    Instruction inputInstruction = INPUT;
    int size = 0;

    if (unit.isArray())
    {
        inputInstruction = INPUT_STR;
        size = unit.dimensions()[0];
    }

    inputEntry.opcode = inputInstruction;
    inputEntry.op1 = unit.op();
    inputEntry.op2 = opBuilder.createIntLiteralOperand(AUTO_INT, (int)size);

    pushEntry(inputEntry);
}

Unit FunctionBuilder::createLocal(const Token nameToken, TypeDescription& typeDescription)
{
    if (workingFunction->symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already defined", nameToken);

    typeDescription.setProperty(IS_LOCAL);

    workingFunction->symbols[nameToken.toString()] = typeDescription;

    return unitBuilder.unitFromTypeDescription(typeDescription, nameToken.toString());
}

void FunctionBuilder::passParameter(const Token& calleeNameToken,
                                    FunctionDescription callee,
                                    const Unit& formalParam,
                                    const Unit& actualParam)
{
    /* Construct icode for PASS and PASS_ADDR instructions */
    std::string mangledCalleeName = nameMangle(calleeNameToken, callee.moduleName);

    DataType functionDataType = callee.functionReturnType.dtype;

    Entry entry;

    if (formalParam.isMutable() || formalParam.isStruct() || formalParam.isArray())
    {
        entry.opcode = PASS_ADDR;
        entry.op1 = createPointer(actualParam);
    }
    else
    {
        entry.opcode = PASS;
        entry.op1 = autoCast(ensureNotPointer(actualParam.op()), formalParam.dtype());
    }

    entry.op2 = opBuilder.createVarOperand(functionDataType, mangledCalleeName);
    entry.op3 = opBuilder.createModuleOperand(callee.moduleName);

    pushEntry(entry);
}

Unit FunctionBuilder::callFunction(const Token& calleeNameToken, FunctionDescription callee)
{
    /* Construct icode for CALL instruction */

    std::string mangledCalleeName = nameMangle(calleeNameToken, callee.moduleName);

    DataType functionDataType = callee.functionReturnType.dtype;

    Entry callEntry;

    callEntry.opcode = CALL;
    callEntry.op1 = opBuilder.createCalleeRetValOperand(functionDataType);
    callEntry.op2 = opBuilder.createVarOperand(functionDataType, mangledCalleeName);
    callEntry.op3 = opBuilder.createModuleOperand(callee.moduleName);

    pushEntry(callEntry);

    return Unit(callee.functionReturnType, callEntry.op1);
}

void FunctionBuilder::noArgumentEntry(Instruction instruction)
{
    /* Construct icode for instructions with no arguments,
        RET, SPACE, NEWLN, EXIT */

    Entry entry;

    entry.opcode = instruction;

    pushEntry(entry);
}

Unit FunctionBuilder::getReturnPointerUnit()
{
    const TypeDescription& returnType = workingFunction->functionReturnType;

    Operand returnPointerOperand = opBuilder.createRetPointerOperand(returnType.dtype);

    return Unit(returnType, returnPointerOperand);
}

bool FunctionBuilder::doesFunctionTerminate()
{
    if (workingFunction->icodeTable.size() < 1)
        return false;

    Instruction lastOpcode = workingFunction->icodeTable.back().opcode;
    return lastOpcode == RET;
}

void FunctionBuilder::terminateFunction(const Token& nameToken)
{
    if (doesFunctionTerminate())
        return;

    if (!workingFunction->isVoid())
        console.compileErrorOnToken("Missing RETURN for this FUNCTION", nameToken);

    noArgumentEntry(RET);
    return;
}
