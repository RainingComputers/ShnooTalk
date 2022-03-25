#include "NameMangle.hpp"
#include "TypeCheck.hpp"
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

Operand FunctionBuilder::autoCast(const Operand& op, DataType destinationDataType)
{
    /* This compiler does not support implicit type casting, this function
        is meant for casting to and from AUTO_INT and AUTO_FLOAT datatypes
        and explicit casting */

    /* Literals are sometimes have AUTO_INT and AUTO_FLOAT data types, and adding
        them will lead to more temp operands having AUTO datatypes */

    /* Pointers will also AUTO data types when assigning conditional expression
        i.e. when ALLOC_PTR instruction is used */

    if (op.dtype == destinationDataType)
        return op;

    Entry entry;

    entry.opcode = CAST;
    entry.op1 = opBuilder.createTempOperand(destinationDataType);
    entry.op2 = op;

    pushEntry(entry);

    return entry.op1;
}

Operand FunctionBuilder::ensureNotPointer(Operand op)
{
    /* Make sure the operand is not a pointer, if it is a pointer,
        converts it to a temp using the READ instruction */

    if (!op.isPointer())
        return op;

    Operand temp = opBuilder.createTempOperand(op.dtype);

    Entry copyEntry;

    copyEntry.op1 = temp;
    copyEntry.op2 = op;
    copyEntry.opcode = READ;
    pushEntry(copyEntry);

    return temp;
}

void FunctionBuilder::operandCopy(Operand dest, Operand src)
{
    Operand castedSrc = autoCast(ensureNotPointer(src), dest.dtype);

    /* Copy one operand value to another, WRITE instruction
        if pointers are involved */

    Entry copyEntry;

    if (dest.isPointer())
        copyEntry.opcode = WRITE;
    else
        copyEntry.opcode = EQUAL;

    copyEntry.op1 = dest;
    copyEntry.op2 = castedSrc;

    pushEntry(copyEntry);
}

void FunctionBuilder::memCopy(Operand op1, Operand op2, unsigned int numBytes)
{
    Entry memCpyEntry;

    memCpyEntry.op1 = op1;
    memCpyEntry.op2 = op2;
    memCpyEntry.op3 = opBuilder.createIntLiteralOperand(AUTO_INT, numBytes);
    memCpyEntry.opcode = MEMCPY;

    pushEntry(memCpyEntry);
}

Operand FunctionBuilder::getCreatePointerDestOperand(const TypeDescription& type)
{
    /* If not a struct, just copy the operand but change its type to a pointer */

    if (!type.isStruct())
        return opBuilder.createTempPtrOperand(type.dtype);

    /* If it a struct, create pointer to the first field */
    ModuleDescription* workingModule = &modulesMap.at(type.moduleName);

    DataType firstFieldDataType = workingModule->structures.at(type.dtypeName).getFirstFieldDataType();

    return opBuilder.createTempPtrOperand(firstFieldDataType);
}

Operand FunctionBuilder::createPointer(const Unit& unit)
{
    if (unit.isPointer() && !unit.isStruct())
        return unit.op();

    /* Converted TEMP_PTR */
    Operand pointerOperand = getCreatePointerDestOperand(unit.type());

    /* Construct CREATE_PTR instruction */
    Entry createPointerEntry;

    createPointerEntry.opcode = CREATE_PTR;
    createPointerEntry.op1 = pointerOperand;
    createPointerEntry.op2 = unit.op();

    pushEntry(createPointerEntry);

    return pointerOperand;
}

Unit FunctionBuilder::createTemp(DataType dtype)
{
    Operand tempPointer = opBuilder.createTempPtrOperand(dtype);

    icode::Entry entry;

    entry.op1 = tempPointer;
    entry.opcode = ALLOC_PTR;

    pushEntry(entry);

    return Unit(typeDescriptionFromDataType(dtype), tempPointer);
}

Unit FunctionBuilder::createTempArray(const TypeDescription& type, unsigned int numElements)
{
    Operand tempPointer = getCreatePointerDestOperand(type);

    icode::Entry entry;

    entry.op1 = tempPointer;
    entry.op2 = opBuilder.createBytesOperand(type.dtypeSize * numElements);
    entry.opcode = ALLOC_ARRAY_PTR;

    pushEntry(entry);

    TypeDescription pointerType = type;
    pointerType.becomeArrayPointer();

    return Unit(pointerType, entry.op1);
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
        unsigned int updateSize = dest.dtypeSize();

        if (unit.isArray())
            updateSize *= dest.dimensions().back();

        Operand update = opBuilder.createBytesOperand(updateSize);
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

void FunctionBuilder::unitPointerAssign(const Unit& to, const Unit& src)
{
    icode::Entry entry;

    entry.op1 = to.op();
    entry.op2 = src.op();
    entry.opcode = PTR_ASSIGN;

    pushEntry(entry);
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

    pushEntry(entry);

    return Unit(LHS.type(), entry.op1).clearProperties();
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

    pushEntry(entry);

    return Unit(unaryOperatorTerm.type(), entry.op1).clearProperties();
}

Unit FunctionBuilder::castOperator(const Unit& unitToCast, DataType destinationDataType)
{
    /* Construct icode for CAST */
    Operand result = autoCast(ensureNotPointer(unitToCast.op()), destinationDataType);

    return Unit(typeDescriptionFromDataType(destinationDataType), result);
}

Unit FunctionBuilder::addrOperator(const Unit& unit)
{
    /* Construct icode for CAST */
    icode::Entry entry;

    entry.opcode = ADDR;
    entry.op1 = opBuilder.createTempOperand(icode::AUTO_INT);
    entry.op2 = unit.op();

    pushEntry(entry);

    return Unit(typeDescriptionFromDataType(icode::AUTO_INT), entry.op1);
}

Unit FunctionBuilder::pointerCastOperator(const Unit& unitToCast, TypeDescription destinationType)
{
    Entry entry;

    entry.opcode = PTR_CAST;
    entry.op1 = opBuilder.createTempPtrOperand(destinationType.dtype);
    entry.op2 = unitToCast.op();

    pushEntry(entry);

    return Unit(destinationType, entry.op1);
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
    entry.op1 = opBuilder.createTempPtrOperand(op2.dtype);
    entry.op2 = op2;
    entry.op3 = op3;

    pushEntry(entry);

    return entry.op1;
}

Operand FunctionBuilder::addressAddOperatorPtrPtr(Operand op2, Operand op3)
{
    /* Construct icode for ADDR_ADD */

    Entry entry;

    entry.opcode = ADDR_ADD;
    entry.op1 = opBuilder.createTempPtrPtrOperand(op2.dtype);
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
    entry.op1 = opBuilder.createTempPtrOperand(VOID);
    entry.op2 = ensureNotPointer(op2);
    entry.op3 = op3;

    pushEntry(entry);

    return entry.op1;
}

Unit FunctionBuilder::getStructFieldFromString(const std::string& fieldName, const Unit& unit)
{
    StructDescription structDescription = modulesMap[unit.moduleName()].structures[unit.dtypeName()];

    TypeDescription fieldType = structDescription.structFields.at(fieldName);

    if (unit.isMutable())
        fieldType.becomeMutable();

    Operand pointerOperand = createPointer(unit);
    Operand offsetOperand = opBuilder.createBytesOperand(fieldType.offset);

    Operand fieldOperand;

    if (fieldType.isPointer())
        fieldOperand = addressAddOperatorPtrPtr(pointerOperand, offsetOperand);
    else
        fieldOperand = addressAddOperator(pointerOperand, offsetOperand);

    fieldOperand.dtype = fieldType.dtype;

    return Unit(fieldType, fieldOperand);
}

Unit FunctionBuilder::getStructField(const Token& fieldNameToken, const Unit& unit)
{
    StructDescription structDescription = modulesMap[unit.moduleName()].structures[unit.dtypeName()];

    const std::string& fieldName = fieldNameToken.toString();

    if (!structDescription.fieldExists(fieldName))
        console.compileErrorOnToken("Undefined STRUCT field", fieldNameToken);

    return getStructFieldFromString(fieldName, unit);
}

Unit FunctionBuilder::getIndexedElement(const Unit& unit, const std::vector<Unit>& indices)
{
    Operand elementOperand = createPointer(unit);

    TypeDescription elementType = unit.type();

    unsigned int dimensionCount = 0;
    unsigned int elementWidth = unit.size() / unit.numElements();

    for (const Unit& indexUnit : indices)
    {
        Operand subscriptOperand = addressMultiplyOperator(indexUnit.op(), opBuilder.createBytesOperand(elementWidth));

        elementType.size = elementWidth;

        if (dimensionCount < elementType.dimensions.size() - 1)
            elementWidth /= elementType.dimensions[dimensionCount + 1];

        elementOperand = addressAddOperator(elementOperand, subscriptOperand);

        dimensionCount++;
    }

    unsigned int remainingDimensionCount = elementType.dimensions.size() - dimensionCount;

    elementType.dimensions.erase(elementType.dimensions.begin(),
                                 elementType.dimensions.end() - remainingDimensionCount);

    return Unit(elementType, elementOperand);
}

std::vector<Unit> FunctionBuilder::destructureArray(const Unit& unit)
{
    std::vector<Unit> destructuredUnits;

    for (unsigned int i = 0; i < unit.numElements(); i += 1)
    {
        std::vector<Unit> indices;
        indices.push_back(unitBuilder.unitFromIntLiteral(i));

        destructuredUnits.push_back(getIndexedElement(unit, indices));
    }

    return destructuredUnits;
}

std::vector<Unit> FunctionBuilder::destructureStruct(const Unit& unit)
{
    std::vector<Unit> destructuredUnits;

    const ModuleDescription& structModule = modulesMap.at(unit.moduleName());
    const StructDescription& structDescription = structModule.structures.at(unit.dtypeName());

    for (const std::string& fieldName : structDescription.fieldNames)
        destructuredUnits.push_back(getStructFieldFromString(fieldName, unit));

    return destructuredUnits;
}

std::vector<Unit> FunctionBuilder::destructureUnit(const Unit& unit)
{
    if (unit.isList())
        return unit.destructureUnitList();

    if (unit.isArray())
        return destructureArray(unit);

    if (unit.isStruct())
        return destructureStruct(unit);

    console.internalBugError();
}

std::map<std::string, Unit> FunctionBuilder::destructureStructMapped(const Unit& unit)
{
    std::map<std::string, Unit> mappedDestructuredUnits;

    const ModuleDescription& structModule = modulesMap.at(unit.moduleName());
    const StructDescription& structDescription = structModule.structures.at(unit.dtypeName());

    for (auto field : structDescription.structFields)
        mappedDestructuredUnits[field.first] = getStructFieldFromString(field.first, unit);

    return mappedDestructuredUnits;
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

    if (unit.isArray())
    {
        printEntry.opcode = PRINT_STR;
        printEntry.op1 = unit.op();
    }
    else
    {
        printEntry.opcode = PRINT;
        printEntry.op1 = ensureNotPointer(unit.op());
    }

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
        size = unit.numElements();
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

std::string FunctionBuilder::getMangledCalleeName(const Token& calleeNameToken, const FunctionDescription& callee)
{
    const std::string& mangeledCalleeName = nameMangle(calleeNameToken, callee.moduleName);

    icode::ModuleDescription& workingModule = modulesMap.at(workingFunction->moduleName);

    if (workingModule.incompleteFunctions.find(calleeNameToken.toString()) != workingModule.incompleteFunctions.end())
        return mangeledCalleeName;

    icode::ModuleDescription& functionModule = modulesMap.at(callee.moduleName);

    if (functionModule.externFunctions.find(calleeNameToken.toString()) != functionModule.externFunctions.end())
        return calleeNameToken.toString();

    return mangeledCalleeName;
}

Operand FunctionBuilder::createPointerForPassAddress(const Unit& actualParam, const Unit& formalParam)
{
    if (!actualParam.isList())
        return createPointer(actualParam);

    /* NOTE: actual param could be AUTO_INT or AUTO_FLOAT and formal
        param could be a different type, in the case the type size would mismatch */

    unsigned int numElements = actualParam.size() / actualParam.dtypeSize();

    Unit tempArray = createTempArray(formalParam.type(), numElements);

    unitListCopy(tempArray, actualParam);

    return tempArray.op();
}

void FunctionBuilder::passParameterPreMangled(const std::string& mangeledCalleeName,
                                              FunctionDescription callee,
                                              const Unit& formalParam,
                                              const Unit& actualParam)
{
    DataType functionDataType = callee.functionReturnType.dtype;

    Entry entry;

    if (formalParam.isMutableAndPointer())
    {
        entry.opcode = PASS_PTR_PTR;
        entry.op1 = actualParam.op();
    }
    else if (formalParam.isMutableOrPointer() || formalParam.isStructOrArray())
    {
        entry.opcode = PASS_PTR;
        entry.op1 = createPointerForPassAddress(actualParam, formalParam);
    }
    else
    {
        entry.opcode = PASS;
        entry.op1 = autoCast(ensureNotPointer(actualParam.op()), formalParam.dtype());
    }

    entry.op2 = opBuilder.createVarOperand(functionDataType, mangeledCalleeName);
    entry.op3 = opBuilder.createModuleOperand(callee.moduleName);

    pushEntry(entry);
}

void FunctionBuilder::passParameter(const Token& calleeNameToken,
                                    FunctionDescription callee,
                                    const Unit& formalParam,
                                    const Unit& actualParam)
{
    /* Construct icode for PASS and PASS_ADDR instructions */
    std::string mangledCalleeName = getMangledCalleeName(calleeNameToken, callee);

    passParameterPreMangled(mangledCalleeName, callee, formalParam, actualParam);
}

Unit FunctionBuilder::callFunctionPreMangled(const std::string& mangeledCalleeName, const FunctionDescription& callee)
{
    /* Construct icode for CALL instruction */

    DataType functionDataType = callee.functionReturnType.dtype;

    Entry callEntry;

    callEntry.opcode = CALL;

    if (callee.functionReturnType.isPointer())
        callEntry.op1 = opBuilder.createCalleeRetPointerOperand(functionDataType);
    else
        callEntry.op1 = opBuilder.createCalleeRetValOperand(functionDataType);

    callEntry.op2 = opBuilder.createVarOperand(functionDataType, mangeledCalleeName);
    callEntry.op3 = opBuilder.createModuleOperand(callee.moduleName);

    pushEntry(callEntry);

    return Unit(callee.functionReturnType, callEntry.op1);
}

Unit FunctionBuilder::callFunction(const Token& calleeNameToken, const FunctionDescription& callee)
{
    /* Construct icode for CALL instruction */

    std::string mangeldCalleeName = getMangledCalleeName(calleeNameToken, callee);

    return callFunctionPreMangled(mangeldCalleeName, callee);
}

void FunctionBuilder::noArgumentEntry(Instruction instruction)
{
    /* Construct icode for instructions with no arguments,
        RET, SPACE, NEWLN, EXIT */

    Entry entry;

    entry.opcode = instruction;

    pushEntry(entry);
}

Unit FunctionBuilder::getReturnValueUnit()
{
    const TypeDescription& returnType = workingFunction->functionReturnType;

    Operand operand;

    if (returnType.isPointer())
        operand = opBuilder.createRetPointerOperand(returnType.dtype);
    else
        operand = opBuilder.createRetValueOperand(returnType.dtype);

    return Unit(returnType, operand);
}

bool FunctionBuilder::doesFunctionTerminate()
{
    if (workingFunction->icodeTable.size() < 1)
        return false;

    Instruction lastOpcode = workingFunction->icodeTable.back().opcode;
    return lastOpcode == RET;
}

bool validMainReturn(const icode::FunctionDescription& functionDescription)
{
    if (functionDescription.functionReturnType.dtype != icode::I32)
        return false;

    if (functionDescription.functionReturnType.isArray())
        return false;

    return true;
}

void FunctionBuilder::terminateFunction(const Token& nameToken)
{
    if (nameToken.toString() == "main" && !validMainReturn(*workingFunction))
        console.compileErrorOnToken("Invalid return type for MAIN", nameToken);

    if (doesFunctionTerminate())
        return;

    if (workingFunction->isVoid())
    {
        noArgumentEntry(RET);
        return;
    }

    console.compileErrorOnToken("Missing RETURN for this FUNCTION", nameToken);
}
